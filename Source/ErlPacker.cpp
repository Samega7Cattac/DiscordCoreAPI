/*
	DiscordCoreAPI, A bot library for Discord, written in C++, and featuring explicit multithreading through the usage of custom, asynchronous C++ CoRoutines.

	Copyright 2021, 2022 Chris M. (RealTimeChris)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
	USA
*/
/// ErlPacker.cpp - Source file for the erlpacking class.
/// Nov 8, 2021
/// https://discordcoreapi.com
/// \file ErlPacker.cpp

#include <discordcoreapi/ErlPacker.hpp>

namespace DiscordCoreInternal {

	ErlPackError::ErlPackError(const std::string& message) : std::runtime_error(message.c_str()){};

	std::string_view ErlPacker::parseJsonToEtf(std::string&& dataToParse) {
		this->bufferString.clear();
		this->offSet = 0;
		this->size = 0;
		this->appendVersion();
		dataToParse.reserve(dataToParse.size() + simdjson::SIMDJSON_PADDING);
		simdjson::ondemand::parser theParser{};
		auto theDocument = theParser.iterate(dataToParse.data(), dataToParse.length(), dataToParse.capacity());
		this->singleValueJsonToETF(theDocument);
		return this->bufferString;
	}

	std::string_view ErlPacker::parseEtfToJson(std::string_view dataToParse) {
		if (this->bufferString.size() < dataToParse.size()) {
			this->bufferString.resize(dataToParse.size() * 2);
		}
		this->offSet = 0;
		this->buffer = dataToParse;
		this->size = dataToParse.size();
		if (this->readBits<uint8_t>() != formatVersion) {
			throw ErlPackError{ "ErlPacker::parseEtfToJson() Error: Incorrect format version specified." };
		}
		this->singleValueETFToJson();
		return std::string_view{ this->bufferString.data(), this->stringSize };
	}

	void ErlPacker::singleValueJsonToETF(simdjson::ondemand::value jsonData) {
		switch (jsonData.type()) {
			case simdjson::ondemand::json_type::array: {
				this->writeArray(jsonData);
				break;
			}
			case simdjson::ondemand::json_type::object: {
				this->writeObject(jsonData);
				break;
			}
			case simdjson::ondemand::json_type::number: {
				this->writeNumber(jsonData);
				break;
			}
			case simdjson::ondemand::json_type::string: {
				this->writeString(jsonData);
				break;
			}
			case simdjson::ondemand::json_type::boolean: {
				this->writeBool(jsonData);
				break;
			}
			case simdjson::ondemand::json_type::null: {
				this->appendNil();
				break;
			}
		}
	}

	void ErlPacker::writeToString(const char*theData, size_t length) {
		if (this->bufferString.size() > this->stringSize + length) {
			this->bufferString.resize((this->bufferString.size() + length) * 2);
		}
		memcpy(this->bufferString.data() + this->stringSize, theData, length);
		this->stringSize += length;
	}

	void ErlPacker::writeObject(simdjson::ondemand::value jsonData) {
		bool add_comma{ false };
		this->appendMapHeader(static_cast<uint32_t>(jsonData.count_fields().take_value()));
		for (auto field: jsonData.get_object()) {
			if (add_comma) {
			}
			std::stringstream theStream{};
			theStream << field.key();
			std::string theKey = theStream.str();

			auto theSize = theKey.size();
			this->appendBinaryExt(theKey, static_cast<uint32_t>(theSize));
			this->singleValueJsonToETF(field.value());
			add_comma = true;
		}
	}

	void ErlPacker::writeString(simdjson::ondemand::value jsonData) {
		std::stringstream theStream{};
		theStream << jsonData.get_string().take_value();
		auto theSize = static_cast<uint32_t>(theStream.str().size());
		this->appendBinaryExt(theStream.str(), theSize);
	}

	void ErlPacker::writeNumber(simdjson::ondemand::value jsonData) {
		if (jsonData.get_number_type() == simdjson::ondemand::number_type::floating_point_number) {
			auto theFloat = jsonData.get_double().take_value();
			this->appendNewFloatExt(theFloat);
		} else if (jsonData.get_number_type() == simdjson::ondemand::number_type::unsigned_integer) {
			auto theInt = jsonData.get_uint64().take_value();
			if (theInt <= 255 && theInt >= 0) {
				this->appendSmallIntegerExt(static_cast<uint8_t>(theInt));
			} else if (theInt <= std::numeric_limits<uint32_t>::max() && theInt >= 0) {
				this->appendIntegerExt(static_cast<uint32_t>(theInt));
			} else {
				this->appendUnsignedLongLong(theInt);
			}
		} else if (jsonData.get_number_type() == simdjson::ondemand::number_type::signed_integer) {
			auto theInt = jsonData.get_int64().take_value();
			if (theInt <= 127 && theInt >= -127) {
				this->appendSmallIntegerExt(static_cast<uint8_t>(theInt));
			} else if (theInt <= std::numeric_limits<int32_t>::max() && theInt >= std::numeric_limits<int32_t>::min()) {
				this->appendIntegerExt(static_cast<uint32_t>(theInt));
			} else {
				this->appendUnsignedLongLong(static_cast<uint64_t>(theInt));
			}
		}
	}

	void ErlPacker::writeArray(simdjson::ondemand::value jsonData) {
		bool add_comma{ false };
		this->appendListHeader(static_cast<uint32_t>(jsonData.count_elements().take_value()));
		for (auto element: jsonData.get_array()) {
			this->singleValueJsonToETF(element.value());
			add_comma = true;
		}
		this->appendNilExt();
	}

	void ErlPacker::writeBool(simdjson::ondemand::value jsonData) {
		auto theBool = jsonData.get_bool().take_value();
		if (theBool) {
			this->appendTrue();
		} else {
			this->appendFalse();
		}
	}

	void ErlPacker::writeToBuffer(const std::string& bytes) {
		this->bufferString.insert(this->bufferString.end(), bytes.begin(), bytes.end());
		this->offSet += bytes.size();
	}

	void ErlPacker::appendBinaryExt(const std::string& bytes, uint32_t sizeNew) {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Binary_Ext) };
		DiscordCoreAPI::storeBits(bufferNew, sizeNew);
		this->writeToBuffer(bufferNew);
		this->writeToBuffer(bytes);
	}

	void ErlPacker::appendUnsignedLongLong(uint64_t value) {
		std::string bufferNew{};
		bufferNew.resize(static_cast<uint64_t>(1) + 2 + sizeof(uint64_t));
		bufferNew[0] = static_cast<uint8_t>(ETFTokenType::Small_Big_Ext);
		DiscordCoreAPI::StopWatch theStopWatch{ 1500ms };
		uint8_t bytesToEncode = 0;
		while (value > 0) {
			if (theStopWatch.hasTimePassed()) {
				break;
			}
			bufferNew[static_cast<size_t>(3) + bytesToEncode] = value & 0xF;
			value >>= 8;
			bytesToEncode++;
		}
		bufferNew[1] = bytesToEncode;
		bufferNew[2] = 0;
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendSmallIntegerExt(uint8_t value) {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Small_Integer_Ext), static_cast<char>(value) };
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendIntegerExt(uint32_t value) {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Integer_Ext) };
		DiscordCoreAPI::storeBits(bufferNew, value);
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendListHeader(uint32_t sizeNew) {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::List_Ext) };
		DiscordCoreAPI::storeBits(bufferNew, sizeNew);
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendMapHeader(uint32_t sizeNew) {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Map_Ext) };
		DiscordCoreAPI::storeBits(bufferNew, sizeNew);
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendNewFloatExt(double floatValue) {
		std::string bufferNew{};
		bufferNew.push_back(static_cast<unsigned char>(ETFTokenType::New_Float_Ext));

		void* punner{ &floatValue };
		DiscordCoreAPI::storeBits(bufferNew, *static_cast<uint64_t*>(punner));
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendVersion() {
		std::string bufferNew{};
		bufferNew.push_back(static_cast<char>(formatVersion));
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendNilExt() {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Nil_Ext) };
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendFalse() {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Small_Atom_Ext), 5, static_cast<uint8_t>('f'), static_cast<uint8_t>('a'), static_cast<uint8_t>('l'),
			static_cast<uint8_t>('s'), static_cast<uint8_t>('e') };
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendTrue() {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Small_Atom_Ext), 4, static_cast<uint8_t>('t'), static_cast<uint8_t>('r'), static_cast<uint8_t>('u'),
			static_cast<uint8_t>('e') };
		this->writeToBuffer(bufferNew);
	}

	void ErlPacker::appendNil() {
		std::string bufferNew{ static_cast<uint8_t>(ETFTokenType::Small_Atom_Ext), 3, static_cast<uint8_t>('n'), static_cast<uint8_t>('i'), static_cast<uint8_t>('l') };
		this->writeToBuffer(bufferNew);
	}

	size_t ErlPacker::readString(uint32_t length) {
		if (this->offSet + static_cast<uint64_t>(length) > this->size) {
			throw ErlPackError{ "this->readString() Error: readString() past end of buffer.\n\n" };
		}
		if (this->bufferString.size() <= length * 2) {
			this->bufferString.resize(length * 2);
		}
		size_t theFinalSize{};
		char* theStringNew = ( char* )this->buffer.data() + this->offSet;
		for (uint32_t x = 0; x < length; ++x) {
			switch (static_cast<char>(theStringNew[x])) {
				case 0x00: {
					break;
				}
				case 0x27: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('\'');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x22: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('"');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x5c: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('\\');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x07: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('a');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x08: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('b');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x0C: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('f');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x0A: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('n');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x0D: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('r');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x0B: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('v');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				case 0x09: {
					this->bufferString[this->stringSize] = static_cast<char>('\\');
					this->bufferString[this->stringSize + 1] = static_cast<char>('t');
					theFinalSize += 2;
					this->stringSize += 2;
					break;
				}
				default: {
					this->bufferString[this->stringSize] = theStringNew[x];
					theFinalSize++;
					this->stringSize++;
					break;
				}
			}
		}
		this->offSet += length;
		return theFinalSize;
	}

	void ErlPacker::processAtom(const char* atom, uint32_t length) {
		if (atom == nullptr) {
			return;
		}
		if (length >= 3 && length <= 5) {
			if (length == 3 && strncmp(atom, "nil", 3) == 0) {
				this->writeToString("null", 4);
				return;
			} else if (length == 4 && strncmp(atom, "null", 4) == 0) {
				this->writeToString("null", 4);
				return;
			} else if (length == 4 && strncmp(atom, "true", 4) == 0) {
				this->writeToString("true", 4);
				return;
			} else if (length == 5 && strncmp(atom, "false", 5) == 0) {
				this->writeToString("false", 5);
				return;
			}
		}
		this->writeToString("\"", 2);
		this->writeToString(atom, length);
		this->writeToString("\"", 2);
		return;
	}

	void ErlPacker::singleValueETFToJson() {
		if (this->offSet >= this->size) {
			throw ErlPackError{ "ErlPacker::singleValueETFToJson() Error: Read past end of ETF buffer.\n\n" };
		}
		uint8_t type = this->readBits<uint8_t>();
		switch (static_cast<ETFTokenType>(type)) {
			case ETFTokenType::Small_Integer_Ext: {
				return this->parseSmallIntegerExt();
			}
			case ETFTokenType::Integer_Ext: {
				return this->parseIntegerExt();
			}
			case ETFTokenType::New_Float_Ext: {
				return this->parseNewFloatExt();
			}
			case ETFTokenType::Float_Ext: {
				return this->parseFloatExt();
			}
			case ETFTokenType::Atom_Ext: {
				return this->parseAtomUtf8Ext();
			}
			case ETFTokenType::Small_Tuple_Ext: {
				return this->parseSmallTupleExt();
			}
			case ETFTokenType::Large_Tuple_Ext: {
				return this->parseLargeTupleExt();
			}
			case ETFTokenType::Nil_Ext: {
				return this->parseNilExt();
			}
			case ETFTokenType::String_Ext: {
				return this->parseStringAsList();
			}
			case ETFTokenType::List_Ext: {
				return this->parseListExt();
			}
			case ETFTokenType::Binary_Ext: {
				return this->parseBinaryExt();
			}
			case ETFTokenType::Small_Big_Ext: {
				return this->parseSmallBigExt();
			}
			case ETFTokenType::Large_Big_Ext: {
				return this->parseLargeBigExt();
			}
			case ETFTokenType::Small_Atom_Ext: {
				return this->parseSmallAtomExt();
			}
			case ETFTokenType::Map_Ext: {
				return this->parseMapExt();
			}
			case ETFTokenType::Atom_Utf8_Ext: {
				return this->parseAtomUtf8Ext();
			}
			default: {
				throw ErlPackError{ "ErlPacker::singleValueETFToJson() Error: Unknown data type in ETF.\n\n" };
			}
		}
	}

	void ErlPacker::parseSmallIntegerExt() {
		this->writeToString(( const char* )this->readBits<uint8_t>(), 1);
		return;
	}

	void ErlPacker::parseBigint(uint32_t digits) {
		uint8_t sign = this->readBits<uint8_t>();
		if (digits > 8) {
			throw ErlPackError{ "ErlPacker::parseBigint() Error: Integers larger than 8 bytes are not supported.\n\n" };
		}
		uint64_t value = 0;
		uint64_t b = 1;
		for (uint32_t x = 0; x < digits; ++x) {
			uint8_t digit = this->readBits<uint8_t>();
			uint64_t digitNew = digit;
			value += digitNew * b;
			b <<= 8;
		}
		if (digits <= 4) {
			if (sign == 0) {
				this->writeToString(reinterpret_cast<const char*>(&value), sizeof(value));
				return;
			}
			const bool isSignBitAvailable = (value & 1ull << 31ull) == 0;
			if (isSignBitAvailable) {
				value = -value;
				this->writeToString(reinterpret_cast<const char*>(&value), sizeof(value));
				return;
			}
		}
		char outBuffer[32] = { 0 };
		const char* formatString = sign == 0 ? "%llu" : "-%ll";
		auto theValue = sign == 0 ? static_cast<uint64_t>(value) : static_cast<int64_t>(value);
		const int32_t res = sprintf(outBuffer, formatString, theValue);
		if (res < 0) {
			throw ErlPackError{ "ErlPacker::parseBigint() Error: Parse big integer failed.\n\n" };
		}
		const uint8_t length = static_cast<uint8_t>(res);
		std::string theReturnValue = std::string{ outBuffer, length };
		this->writeToString(reinterpret_cast<const char*>(&theReturnValue), sizeof(theReturnValue));
		return;
	}

	void ErlPacker::parseArray(const uint32_t length) {
		for (uint32_t x = 0; x < length; x++) {
			this->singleValueETFToJson();
			if (x < length - 1) {
				this->writeToString(",", 1);
			}
		}
		return;
	}

	void ErlPacker::parseTuple(const uint32_t length) {
		return this->parseArray(length);
	}

	void ErlPacker::parseSmallTupleExt() {
		return this->parseTuple(this->readBits<uint8_t>());
	}

	void ErlPacker::parseLargeTupleExt() {
		return this->parseTuple(this->readBits<uint32_t>());
	}

	void ErlPacker::parseSmallAtomExt() {
		uint8_t length = this->readBits<uint8_t>();
		auto lengthNew = this->readString(length);
		return this->processAtom(this->bufferString.data(), length);
	};

	void ErlPacker::parseStringAsList() {
		this->writeToString("\"", 2);
		uint16_t length = this->readBits<uint16_t>();
		if (static_cast<uint64_t>(this->offSet) + length > this->size) {
			throw ErlPackError{ "ErlPacker::parseStringAsList() Error: String list past end of buffer.\n\n" };
		}
		for (uint16_t x = 0; x < length; ++x) {
			this->parseSmallIntegerExt();
		}
		this->writeToString("\"", 2);

		return;
	}

	void ErlPacker::parseNewFloatExt() {
		uint64_t theValue = readBits<uint64_t>();
		void* thePtr{ &theValue };
		this->writeToString(reinterpret_cast<const char*>(thePtr), sizeof(double));
		return ;
	}

	void ErlPacker::parseSmallBigExt() {
		this->writeToString("\"", 2);
		this->parseBigint(this->readBits<uint8_t>());
		this->writeToString("\"", 2);
		return;
	}

	void ErlPacker::parseLargeBigExt() {
		return this->parseBigint(this->readBits<uint32_t>());
	}

	void ErlPacker::parseAtomUtf8Ext() {
		uint32_t length = this->readBits<uint16_t>();
		auto lengthNew = static_cast<uint32_t>(this->readString(length));
		return this->processAtom(this->bufferString.data(), lengthNew);
	}

	void ErlPacker::parseIntegerExt() {
		auto theInteger = this->readBits<uint32_t>();
		this->writeToString(reinterpret_cast<const char*>(&theInteger), 4);
		return;
	}

	void ErlPacker::parseBinaryExt() {
		this->writeToString("\"", 2);
		uint32_t length = this->readBits<uint32_t>();
		this->readString(length);
		this->writeToString("\"", 2);
		return;
	}

	void ErlPacker::parseFloatExt() {
		const uint8_t floatLength = 31;
		size_t floatString = readString(floatLength);

		if (floatString == NULL) {
			return;
		}

		double number{};
		std::string nullTerminated{};
		nullTerminated.insert(nullTerminated.begin(), this->bufferString.data(), this->bufferString.data() + floatLength);

		auto count = sscanf(nullTerminated.data(), "%lf", &number);

		if (count != 1) {
			return;
		}
		this->writeToString(reinterpret_cast<const char*>(&number), sizeof(number));
		return;
	}

	void ErlPacker::parseListExt() {
		uint32_t length = this->readBits<uint32_t>();
		std::string theArray{};
		this->writeToString("[", 1);
		this->parseArray(length);
		this->writeToString("]", 1);
		uint8_t theValue = this->readBits<uint8_t>();
		if (static_cast<ETFTokenType>(theValue) != ETFTokenType::Nil_Ext) {
			return;
		}
		return;
	}

	void ErlPacker::parseNilExt() {
		this->writeToString("[]", 2);
		return;
	}

	void ErlPacker::parseMapExt() {
		uint32_t length = readBits<uint32_t>();
		std::string map{};
		this->writeToString("{", 1);
		for (uint32_t i = 0; i < length; ++i) {
			this->singleValueETFToJson();
			this->writeToString(":", 1);
			this->singleValueETFToJson();
			if (i < length - 1) {
				this->writeToString(",", 1);
			}
		}
		this->writeToString("}", 1);
		return;
	}
}
