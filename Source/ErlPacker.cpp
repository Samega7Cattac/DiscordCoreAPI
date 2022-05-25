/*
*
	Copyright 2021, 2022 Chris M. (RealTimeChris)

	This file is part of DiscordCoreAPI.
	DiscordCoreAPI is free software: you can redistribute it and/or modify it under the terms of the GNU
	General Public License as published by the Free Software Foundation, either version 3 of the License,
	or (at your option) any later version.
	DiscordCoreAPI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with DiscordCoreAPI.
	If not, see <https://www.gnu.org/licenses/>.

*/
/// ErlPacker.cpp - Source file for the erlpacking class.
/// Nov 8, 2021
/// Chris M.
/// https://discordcoreapi.com
/// \file ErlPacker.cpp

#include <discordcoreapi/ErlPacker.hpp>

namespace DiscordCoreInternal {

	ErlPackBuffer::ErlPackBuffer(const std::string* theBuffer) {
		this->buffer.insert(this->buffer.begin(), theBuffer->begin(), theBuffer->end());
	};

	ErlPackBuffer& ErlPackBuffer::operator=(ErlPackBuffer&& other) noexcept {
		if (this != &other) {
			this->buffer = std::move(other.buffer);
			other.buffer = std::string{};
			this->offSet = other.offSet;
			other.offSet = 0;
		}
		return *this;
	}

	ErlPackBuffer::ErlPackBuffer(ErlPackBuffer&& other) noexcept {
		*this = std::move(other);
	}

	std::string ErlPacker::parseJsonToEtf(const nlohmann::json& dataToParse) {
		ErlPackBuffer buffer{};
		ErlPacker::appendVersion(buffer);
		ErlPacker::singleValueJsonToETF(buffer, dataToParse);
		return buffer.buffer;
	}

	nlohmann::json ErlPacker::parseEtfToJson(const std::string* dataToParse) {
		ErlPackBuffer buffer{ dataToParse };
		uint8_t version{};
		ErlPacker::readBits(buffer, version);
		return ErlPacker::singleValueETFToJson(buffer);
	}

	template<typename ReturnType> void ErlPacker::etfByteOrder(ReturnType x, ReturnType& theValue) {
		const uint8_t byteSize{ 8 };
		for (uint32_t y = 0; y < sizeof(ReturnType); y += 1) {
			theValue |= static_cast<ReturnType>(static_cast<uint8_t>(x >> (byteSize * y))) << byteSize * (sizeof(ReturnType) - y - 1);
		}
	}

	template<typename ReturnType> void ErlPacker::storeBits(std::vector<uint8_t>& to, ReturnType& num, uint32_t& offSet) {
		const uint8_t byteSize{ 8 };
		ReturnType newVal{};
		ErlPacker::etfByteOrder(num, newVal);
		for (uint32_t x = 0; x < sizeof(ReturnType); x += 1) {
			to[offSet + static_cast<size_t>(x)] = static_cast<uint8_t>(newVal >> (byteSize * x));
		}
	}

	void ErlPacker::singleValueJsonToETF(ErlPackBuffer& buffer, const nlohmann::json& jsonData) {
		if (jsonData.is_array()) {
			uint32_t length = static_cast<uint32_t>(jsonData.size());
			if (length == 0) {
				ErlPacker::appendNilExt(buffer);
			} else {
				if (length > std::numeric_limits<uint32_t>::max() - 1) {
					throw ErlPackError("ETF Parse Error: List too large for ETF");
				}
			}
			ErlPacker::appendListHeader(buffer, length);
			for (uint64_t index = 0; index < length; ++index) {
				ErlPacker::singleValueJsonToETF(buffer, jsonData[index]);
			}
			ErlPacker::appendNilExt(buffer);
		} else if (jsonData.is_object()) {
			uint32_t length = static_cast<uint32_t>(jsonData.size());
			if (length > std::numeric_limits<uint32_t>::max() - 1) {
				throw ErlPackError("ETF Parse Error: Map too large for ETF");
			}
			ErlPacker::appendMapHeader(buffer, length);
			for (auto n = jsonData.begin(); n != jsonData.end(); ++n) {
				nlohmann::json jstr = n.key();
				ErlPacker::singleValueJsonToETF(buffer, jstr);
				ErlPacker::singleValueJsonToETF(buffer, n.value());
			}
		} else if (jsonData.is_number_integer()) {
			uint8_t numberOld = jsonData.get<uint8_t>();
			if (numberOld >= 0 && numberOld <= 127) {
				ErlPacker::appendSmallIntegerExt(buffer, numberOld);
			} else if (jsonData.is_number_unsigned() && numberOld >= std::numeric_limits<uint32_t>::max() - 1) {
				uint64_t number = jsonData.get<uint64_t>();
				ErlPacker::appendUnsignedLongLong(buffer, number);
			} else {
				int32_t number = jsonData.get<int32_t>();
				ErlPacker::appendIntegerExt(buffer, number);
			}
		} else if (jsonData.is_boolean()) {
			if (jsonData.get<bool>()) {
				ErlPacker::appendTrue(buffer);
			} else {
				ErlPacker::appendFalse(buffer);
			}
		} else if (jsonData.is_string()) {
			std::string newString = jsonData.get<std::string>();
			newString.shrink_to_fit();
			std::vector<uint8_t> newVector{};
			newVector.insert(newVector.begin(), newString.begin(), newString.end());
			uint32_t newValue = static_cast<uint32_t>(newVector.size());
			ErlPacker::appendBinary(buffer, newVector, newValue);
		} else if (jsonData.is_number_float()) {
			double newValue = jsonData.get<double>();
			ErlPacker::appendNewFloatExt(buffer, newValue);
		} else if (jsonData.is_null()) {
			ErlPacker::appendNilExt(buffer);
		}
	}

	void ErlPacker::writeToBuffer(ErlPackBuffer& buffer, const std::vector<uint8_t>& bytes) {
		buffer.buffer.reserve(buffer.offSet + bytes.size());
		buffer.buffer.insert(buffer.buffer.end(), bytes.begin(), bytes.end());
		buffer.offSet += static_cast<uint32_t>(bytes.size());
	}

	void ErlPacker::appendVersion(ErlPackBuffer& buffer) {
		std::vector<uint8_t> bufferNew{ formatVersion };
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendFalse(ErlPackBuffer& buffer) {
		std::vector<uint8_t> bufferNew{ static_cast<uint8_t>(ETFTokenType::Small_Atom), 5, static_cast<uint8_t>('f'), static_cast<uint8_t>('a'), static_cast<uint8_t>('l'),
			static_cast<uint8_t>('s'), static_cast<uint8_t>('e') };
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendTrue(ErlPackBuffer& buffer) {
		std::vector<uint8_t> bufferNew{ static_cast<uint8_t>(ETFTokenType::Small_Atom), 4, static_cast<uint8_t>('t'), static_cast<uint8_t>('r'), static_cast<uint8_t>('u'),
			static_cast<uint8_t>('e') };
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendSmallIntegerExt(ErlPackBuffer& buffer, uint8_t& value) {
		std::vector<uint8_t> bufferNew{ static_cast<uint8_t>(ETFTokenType::Small_Integer_Ext), value };
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendIntegerExt(ErlPackBuffer& buffer, int32_t& value) {
		std::vector<uint8_t> bufferNew{ static_cast<uint8_t>(ETFTokenType::Integer_Ext), 0, 0, 0, 0 };
		uint32_t newValue{ 1 };
		ErlPacker::storeBits(bufferNew, value, newValue);
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendUnsignedLongLong(ErlPackBuffer& buffer, uint64_t& value) {
		std::vector<uint8_t> bufferNew{};
		bufferNew.resize(static_cast<uint64_t>(1) + 2 + sizeof(uint64_t));
		bufferNew[0] = static_cast<uint8_t>(ETFTokenType::Small_BigInt);
		uint8_t bytesToEncode = 0;
		while (value > 0) {
			bufferNew[static_cast<size_t>(3) + bytesToEncode] = value & 0xF;
			value >>= 8;
			bytesToEncode++;
		}
		bufferNew[1] = bytesToEncode;
		bufferNew[2] = 0;
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendNewFloatExt(ErlPackBuffer& buffer, double& value) {
		std::vector<uint8_t> bufferNew{};
		bufferNew.resize(static_cast<uint64_t>(1 + 8));
		bufferNew[0] = static_cast<uint8_t>(ETFTokenType::New_Float_Ext);
		void* doubleValue{ &value };
		uint32_t newValue{ 1 };
		ErlPacker::storeBits(bufferNew, *static_cast<uint64_t*>(doubleValue), newValue);
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendBinary(ErlPackBuffer& buffer, const std::vector<uint8_t>& bytes, uint32_t& size) {
		std::vector<uint8_t> bufferNew{};
		bufferNew.resize(5);
		bufferNew[0] = static_cast<uint8_t>(ETFTokenType::Binary);
		uint32_t newValue{ 1 };
		ErlPacker::storeBits(bufferNew, size, newValue);
		ErlPacker::writeToBuffer(buffer, bufferNew);
		ErlPacker::writeToBuffer(buffer, bytes);
	}

	void ErlPacker::appendNilExt(ErlPackBuffer& buffer) {
		std::vector<uint8_t> bufferNew{};
		bufferNew.resize(1);
		bufferNew[0] = static_cast<uint8_t>(ETFTokenType::Nil);
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendListHeader(ErlPackBuffer& buffer, uint32_t& size) {
		std::vector<uint8_t> bufferNew{};
		bufferNew.resize(5);
		bufferNew[0] = static_cast<uint8_t>(ETFTokenType::List);
		uint32_t newValue{ 1 };
		ErlPacker::storeBits(bufferNew, size, newValue);
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	void ErlPacker::appendMapHeader(ErlPackBuffer& buffer, uint32_t& size) {
		std::vector<uint8_t> bufferNew{};
		bufferNew.resize(5);
		bufferNew[0] = static_cast<uint8_t>(ETFTokenType::Map);
		uint32_t newValue{ 1 };
		ErlPacker::storeBits(bufferNew, size, newValue);
		ErlPacker::writeToBuffer(buffer, bufferNew);
	}

	template<typename ReturnType> void ErlPacker::readBits(const ErlPackBuffer& buffer, ReturnType& theValue) {
		const uint8_t byteSize{ 8 };
		if (buffer.offSet + sizeof(ReturnType) > buffer.buffer.size()) {
			throw ErlPackError("ETF Parse Error: readBits() past end of buffer");
		}
		ReturnType newValue{ 0 };
		for (uint64_t x = 0; x < sizeof(ReturnType); x += 1) {
			newValue |= static_cast<ReturnType>(static_cast<uint64_t>(buffer.buffer.data()[buffer.offSet + x]) << (x * static_cast<uint64_t>(byteSize)));
		}
		buffer.offSet += sizeof(ReturnType);
		ErlPacker::etfByteOrder(newValue, theValue);
	}

	void ErlPacker::readString(const ErlPackBuffer& buffer, uint32_t& length, std::vector<char>& theString) {
		if (buffer.offSet + static_cast<uint64_t>(length) > buffer.buffer.size()) {
			throw ErlPackError("ETF Parse Error: readString() past end of buffer");
		}
		std::vector<char> str{};
		for (uint64_t x = 0; x < length; x += 1) {
			str.push_back(buffer.buffer[static_cast<uint64_t>(buffer.offSet + x)]);
		}
		buffer.offSet += length;
		theString = std::move(str);
	}

	nlohmann::json ErlPacker::singleValueETFToJson(const ErlPackBuffer& buffer) {
		if (buffer.offSet > buffer.buffer.size()) {
			throw ErlPackError("ETF Parse Error: Read past end of ETF buffer");
		}
		uint8_t type{};
		ErlPacker::readBits(buffer, type);
		switch (static_cast<ETFTokenType>(type)) {
			case ETFTokenType::Small_Integer_Ext: {
				return ErlPacker::parseSmallIntegerExt(buffer);
			}
			case ETFTokenType::Integer_Ext: {
				return ErlPacker::parseIntegerExt(buffer);
			}
			case ETFTokenType::New_Float_Ext: {
				return ErlPacker::parseNewFloatExt(buffer);
			}
			case ETFTokenType::Atom: {
				return ErlPacker::parseAtom(buffer);
			}
			case ETFTokenType::Small_Atom: {
				return ErlPacker::parseSmallAtom(buffer);
			}
			case ETFTokenType::Small_Tuple: {
				return ErlPacker::parseSmallTuple(buffer);
			}
			case ETFTokenType::Large_Tuple: {
				return ErlPacker::parseLargeTuple(buffer);
			}
			case ETFTokenType::Nil: {
				return ErlPacker::parseNil();
			}
			case ETFTokenType::String: {
				return ErlPacker::parseStringAsList(buffer);
			}
			case ETFTokenType::List: {
				return ErlPacker::parseList(buffer);
			}
			case ETFTokenType::Map: {
				return ErlPacker::parseMap(buffer);
			}
			case ETFTokenType::Binary: {
				return ErlPacker::parseBinary(buffer);
			}
			case ETFTokenType::Small_BigInt: {
				return ErlPacker::parseSmallBigInt(buffer);
			}
			case ETFTokenType::Large_BigInt: {
				return ErlPacker::parseLargeBigInt(buffer);
			}
			default: {
				throw ErlPackError("ETF Parse Error: Unknown data type in ETF");
			}
		}
	}

	nlohmann::json ErlPacker::processAtom(const std::vector<char>& atom, uint32_t& length) {
		nlohmann::json jsonData{};
		if (atom.size() == 0) {
			return jsonData;
		}
		if (length >= 3 && length <= 5) {
			if (length == 3 && strncmp(atom.data(), "nil", 3) == 0) {
				return jsonData;
			} else if (length == 4 && strncmp(atom.data(), "null", 4) == 0) {
				return jsonData;
			} else if (length == 4 && strncmp(atom.data(), "true", 4) == 0) {
				jsonData = true;
				return jsonData;
			} else if (length == 5 && strncmp(atom.data(), "false", 5) == 0) {
				jsonData = false;
				return jsonData;
			}
		}
		jsonData = std::string(atom.data(), length);
		return jsonData;
	}

	nlohmann::json ErlPacker::parseAtom(const ErlPackBuffer& buffer) {
		uint16_t length{};
		readBits(buffer, length);
		uint32_t lengthNew = length;
		std::vector<char> atom{};
		readString(buffer, lengthNew, atom);
		return processAtom(atom, lengthNew);
	}

	nlohmann::json ErlPacker::parseSmallAtom(const ErlPackBuffer& buffer) {
		uint8_t length{};
		readBits(buffer, length);
		uint32_t lengthNew = length;
		std::vector<char> atom{};
		readString(buffer, lengthNew, atom);
		return processAtom(atom, lengthNew);
	}

	nlohmann::json ErlPacker::parseSmallIntegerExt(const ErlPackBuffer& buffer) {
		uint8_t newValue{};
		readBits(buffer, newValue);
		nlohmann::json jsonData = newValue;
		return jsonData;
	}

	nlohmann::json ErlPacker::parseIntegerExt(const ErlPackBuffer& buffer) {
		int32_t newValue{};
		readBits(buffer, newValue);
		nlohmann::json jsonData = newValue;
		return jsonData;
	}

	nlohmann::json ErlPacker::parseArray(const ErlPackBuffer& buffer, uint32_t& length) {
		nlohmann::json array = nlohmann::json::array();
		for (uint32_t x = 0; x < length; x += 1) {
			array.emplace_back(singleValueETFToJson(buffer));
		}
		return array;
	}

	nlohmann::json ErlPacker::parseList(const ErlPackBuffer& buffer) {
		uint32_t length{};
		readBits(buffer, length);
		std::cout << "THE LENGTH: " << length << std::endl;
		nlohmann::json::array_t theArray = parseArray(buffer, length);
		uint8_t theValue{};
		readBits(buffer, theValue);
		const ETFTokenType tailMarker = static_cast<ETFTokenType>(theValue);
		if (tailMarker == ETFTokenType::Nil) {
			return nlohmann::json::array_t();
		}
		return theArray;
	}

	nlohmann::json ErlPacker::parseTuple(const ErlPackBuffer& buffer, uint32_t& length) {
		return parseArray(buffer, length);
	}

	nlohmann::json ErlPacker::parseNil() {
		return nlohmann::json();
	}

	nlohmann::json ErlPacker::parseMap(const ErlPackBuffer& buffer) {
		uint32_t length{};
		readBits(buffer, length);
		auto theMap = nlohmann::json::object();
		for (uint32_t x = 0; x < length; ++x) {
			const nlohmann::json key = singleValueETFToJson(buffer);
			const nlohmann::json value = singleValueETFToJson(buffer);
			if (key.is_number()) {
				theMap[std::to_string(key.get<uint64_t>())] = value;
			} else {
				theMap[key.get<std::string>()] = value;
			}
		}
		return theMap;
	}

	nlohmann::json ErlPacker::parseNewFloatExt(const ErlPackBuffer& buffer) {
		uint64_t theValue{};
		readBits(buffer, theValue);
		const void* doubleValue{ &theValue };
		nlohmann::json jsonData = *static_cast<const double*>(doubleValue);
		return jsonData;
	}

	nlohmann::json ErlPacker::parseBigInt(const ErlPackBuffer& buffer, uint32_t& digits) {
		uint8_t sign{};
		readBits(buffer, sign);
		if (digits > 8) {
			throw ErlPackError("ETF Parse Error: Integers larger than 8 bytes are not supported.");
		}
		uint64_t value = 0;
		uint64_t b = 1;
		for (uint32_t x = 0; x < digits; ++x) {
			uint8_t digit{};
			readBits(buffer, digit);
			uint64_t digitNew = digit;
			value += digitNew * b;
			b <<= 8;
		}
		if (digits <= 4) {
			if (sign == 0) {
				nlohmann::json jsonData = std::to_string(value);
				return jsonData;
			}
			const bool isSignBitAvailable = (value & 1ull << 31ull) == 0;
			if (isSignBitAvailable) {
				nlohmann::json jsonData = std::to_string(-static_cast<uint32_t>(value));
				return jsonData;
			}
		}
		char outBuffer[32] = { 0 };
		const char* formatString = sign == 0 ? "%llu" : "-%ll";
		const uint32_t res = sprintf(outBuffer, formatString, value);
		if (res < 0) {
			throw ErlPackError("ETF Parse Error: parse big Integer failed.");
		}
		const uint8_t length = static_cast<uint8_t>(res);
		nlohmann::json jsonData = std::string(outBuffer, length);
		return jsonData;
	}

	nlohmann::json ErlPacker::parseSmallBigInt(const ErlPackBuffer& buffer) {
		uint8_t theValue{};
		readBits(buffer, theValue);
		uint32_t theValueNew = theValue;
		return parseBigInt(buffer, theValueNew);
	}

	nlohmann::json ErlPacker::parseLargeBigInt(const ErlPackBuffer& buffer) {
		uint32_t theValue{};
		readBits(buffer, theValue);
		return parseBigInt(buffer, theValue);
	}

	nlohmann::json ErlPacker::parseBinary(const ErlPackBuffer& buffer) {
		uint32_t length{};
		readBits(buffer, length);
		if (length == 0) {
			nlohmann::json jsonData{};
			return jsonData;
		}
		std::vector<char> stringNew{};
		readString(buffer, length, stringNew);
		nlohmann::json jsonData = std::string(stringNew.data(), length);
		return jsonData;
	}

	nlohmann::json ErlPacker::parseString(const ErlPackBuffer& buffer) {
		uint16_t length{};
		readBits(buffer, length);
		uint32_t lengthNew = length;
		std::vector<char> stringNew{};
		readString(buffer, lengthNew, stringNew);
		if (stringNew.size() == 0) {
			nlohmann::json jsonData{};
			return jsonData;
		}
		nlohmann::json jsonData = std::string(stringNew.data(), length);
		return jsonData;
	}

	nlohmann::json ErlPacker::parseStringAsList(const ErlPackBuffer& buffer) {
		uint16_t length{};
		readBits(buffer, length);
		nlohmann::json array = nlohmann::json::array();
		if (static_cast<uint64_t>(buffer.offSet) + length > buffer.buffer.size()) {
			throw ErlPackError("ETF Parse Error: String list past end of buffer");
		}
		for (uint16_t x = 0; x < length; ++x) {
			array.push_back(parseSmallIntegerExt(buffer));
		}
		return array;
	}

	nlohmann::json ErlPacker::parseSmallTuple(const ErlPackBuffer& buffer) {
		uint8_t theValue{};
		readBits(buffer, theValue);
		uint32_t theValueNew = theValue;
		return parseTuple(buffer, theValueNew);
	}

	nlohmann::json ErlPacker::parseLargeTuple(const ErlPackBuffer& buffer) {
		uint32_t theValue{};
		readBits(buffer, theValue);
		return parseTuple(buffer, theValue);
	}
}
