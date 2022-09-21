﻿/*
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
/// Utilities.cpp - Source file for the Utilities.
/// Jun 28, 2022
/// https://discordcoreapi.com
/// \file Utilities.cpp

#include <discordcoreapi/FoundationEntities.hpp>
#include <discordcoreapi/GuildEntities.hpp>
#include <discordcoreapi/GuildMemberEntities.hpp>
#include <discordcoreapi/InteractionEntities.hpp>
#include <discordcoreapi/GuildScheduledEventEntities.hpp>
#include <discordcoreapi/RoleEntities.hpp>
#include <discordcoreapi/UserEntities.hpp>
#include <discordcoreapi/ChannelEntities.hpp>
#include <discordcoreapi/CoRoutine.hpp>
#include <discordcoreapi/InputEvents.hpp>
#include <discordcoreapi/Utilities.hpp>
#include <string>

namespace DiscordCoreInternal {

	WebSocketClose& WebSocketClose::operator=(uint16_t theValueNew) {
		this->theValue = this->mappingValues[theValueNew];
		return *this;
	};

	WebSocketClose::WebSocketClose(uint16_t theValueNew) {
		*this = theValueNew;
	};

	WebSocketClose::operator uint16_t() {
		for (auto& [key, value]: this->mappingValues) {
			if (theValue == value) {
				return key;
			}
		}
		return uint16_t{};
	}

	WebSocketClose::operator bool() {
		return utCast(this->theValue) & utCast(WebSocketCloseCode::We_Do_Reconnect);
	}
}

namespace DiscordCoreAPI {

	JsonRecord& JsonRecord::operator=(bool theData) noexcept {
		std::string theString{};
		if (theData) {
			theString = "true";
		} else {
			theString = "false";
		}
		this->theValue = theString;
		this->theEvent = JsonParseEvent::Boolean;
		return *this;
	}

	JsonRecord& JsonRecord::operator=(std::nullptr_t) noexcept {
		this->theEvent = JsonParseEvent::Null_Value;
		return *this;
	}

	JsonRecord& JsonRecord::operator=(const char* theData) noexcept {
		this->theEvent = JsonParseEvent::String;
		this->theValue = theData;
		return *this;
	}

	JsonRecord& JsonRecord::operator=(std::string& theData) noexcept {
		this->theEvent = JsonParseEvent::String;
		this->theValue = theData;
		return *this;
	}

	JsonRecord& JsonRecord::operator=(float theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Float;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(double theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Double;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(int64_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer_Large;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(int32_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(int16_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(int8_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer_Small;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(uint64_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer_Large;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(uint32_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(uint16_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord& JsonRecord::operator=(uint8_t theData) noexcept {
		this->theEvent = JsonParseEvent::Number_Integer_Small;
		this->theValue = std::to_string(theData);
		return *this;
	}

	JsonRecord::JsonRecord(bool theData) noexcept {
		this->theEvent = JsonParseEvent::Boolean;
		std::string theString{};
		if (theData) {
			theString = "true";
		} else {
			theString = "false";
		}
		this->theValue = theString;
	}

	JsonRecord::JsonRecord(std::nullptr_t) noexcept {
		this->theEvent = JsonParseEvent::Null_Value;
	}

	JsonRecord::JsonRecord(const char* theData) noexcept {
		this->theEvent = JsonParseEvent::String;
		this->theValue = theData;
	}

	JsonRecord::JsonRecord(std::string& theData) noexcept {
		this->theEvent = JsonParseEvent::String;
		this->theValue = theData;
	}

	JsonRecord::JsonRecord(EnumConverter theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(float theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(double theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(int64_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(int32_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(int16_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(int8_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(uint64_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(uint32_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(uint16_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::JsonRecord(uint8_t theData) noexcept {
		*this = theData;
	}

	JsonRecord::operator std::string() noexcept {
		std::string theString{};
		if (this->theKey != "") {
			theString += "\"" + this->theKey + "\":";
		}
		switch (this->theEvent) {
			case JsonParseEvent::Object_Start: {
				this->theState = JsonParserState::Starting_Object;
				this->currentObjectOrArrayStartIndex++;
				theString += "{";
				break;
			}
			case JsonParseEvent::Object_End: {
				theString += "}";
				this->currentObjectOrArrayStartIndex--;
				if (theString[theString.size() - 2] == ',') {
					theString.erase(theString.begin() + theString.size() - 2);
				}
				break;
			}
			case JsonParseEvent::Array_Start: {
				this->theState = JsonParserState::Starting_Array;
				theString += "[";
				break;
			}
			case JsonParseEvent::Array_End: {
				this->theState = JsonParserState::Adding_Object_Elements;
				theString += "]";
				if (theString[theString.size() - 2] == ',') {
					theString.erase(theString.begin() + theString.size() - 2);
				}
				break;
			}
			case JsonParseEvent::Boolean: {
				theString += this->theValue;
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
			case JsonParseEvent::Null_Value: {
				theString += "null";
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
			case JsonParseEvent::Number_Double: {
				theString += this->theValue;
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
			case JsonParseEvent::Number_Float: {
				theString += this->theValue;
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
			case JsonParseEvent::Number_Integer: {
				theString += this->theValue;
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
			case JsonParseEvent::Number_Integer_Large: {
				theString += this->theValue;
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
			case JsonParseEvent::Number_Integer_Small: {
				theString += this->theValue;
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
			case JsonParseEvent::String: {
				theString += "\"";
				theString += this->theValue;
				theString += "\"";
				if (this->theState == JsonParserState::Starting_Object) {
					this->theState = JsonParserState::Adding_Object_Elements;
				}
				if (this->theState == JsonParserState::Starting_Array) {
					this->theState = JsonParserState::Adding_Array_Elements;
				}
				break;
			}
		}
		for (uint32_t x = 0; x < this->currentObjectOrArrayStartIndex; ++x) {
			theString += "}";
		}
		return theString;
	}

	void JsonSerializer::addNewStructure(const char* keyName) {
		JsonRecord theRecord{};
		theRecord.theKey = keyName;
		theRecord.theEvent = JsonParseEvent::Object_Start;
		this->theJsonData.push_back(theRecord);
	}

	void JsonSerializer::appendStructElement(const char* keyName, JsonRecord&& theRecord) {
		theRecord.theKey = keyName;
		if (theRecord.theValue != "") {
			this->theJsonData.push_back(std::move(theRecord));
		}
	}

	void JsonSerializer::appendStructElement(const char* keyName, EnumConverter& theRecord) {
		this->theJsonData.push_back(theRecord);
	}

	void JsonSerializer::appendStructElement(const char* keyName, EnumConverter&& theRecord) {
		this->theJsonData.push_back(theRecord);
	}

	void JsonSerializer::appendStructElement(const char* keyName, JsonRecord& theRecord) {
		theRecord.theKey = keyName;
		if (theRecord.theValue != "") {
			this->theJsonData.push_back(theRecord);
		}
	}

	void JsonSerializer::endStructure() {
		JsonRecord theRecord{};
		theRecord.theEvent = JsonParseEvent::Object_End;
		this->theJsonData.push_back(theRecord);
	}

	void JsonSerializer::addNewArray(const char* keyName) {
		JsonRecord theRecord{};
		theRecord.theKey = keyName;
		theRecord.theEvent = JsonParseEvent::Array_Start;
		this->theJsonData.push_back(theRecord);
	}

	void JsonSerializer::appendArrayElement(JsonRecord&& theRecord) {
		this->theJsonData.push_back(std::move(theRecord));
	}

	void JsonSerializer::appendArrayElement(JsonRecord& theRecord) {
		this->theJsonData.push_back(theRecord);
	}

	void JsonSerializer::appendStructElement(const char* theKeyName, JsonSerializer&& theRecord) {
		this->addNewStructure(theKeyName);
		for (auto& value: theRecord.theJsonData) {
			this->theJsonData.push_back(std::move(value));
		}
		this->endStructure();
	}

	void JsonSerializer::appendStructElement(const char* theKeyName,JsonSerializer& theRecord) {
		this->addNewStructure(theKeyName);
		for (auto& value: theRecord.theJsonData) {
			this->theJsonData.push_back(value);
		}
		this->endStructure();
	}

	void JsonSerializer::appendArrayElement(JsonSerializer&& theRecord) {
		this->addNewStructure("");
		for (auto& value: theRecord.theJsonData) {
			this->theJsonData.push_back(std::move(value));
		}
		this->endStructure();
	}

	void JsonSerializer::appendArrayElement(JsonSerializer& theRecord) {
		this->addNewStructure("");
		for (auto& value: theRecord.theJsonData) {
			this->theJsonData.push_back(value);
		}
		this->endStructure();
	}

	void JsonSerializer::endArray() {
		JsonRecord theRecord{};
		theRecord.theEvent = JsonParseEvent::Array_End;
		this->theJsonData.push_back(theRecord);
	}

	std::string JsonSerializer::getString() {
		std::string theString{ "{" };
		this->theState = JsonParserState::Starting_Object;
		for (auto iterator = this->theJsonData.begin(); iterator != this->theJsonData.end(); ++iterator) {
			std::cout << "WERE HERE THIS IS ITS!" << std::endl;
			if (this->theState == JsonParserState::Adding_Object_Elements|| this->theState == JsonParserState::Adding_Array_Elements) {
				theString += ",";
			}
			if ((*iterator).theKey != "") {
				theString += "\"" + (*iterator).theKey + "\":";
			}
			switch ((*iterator).theEvent) {
				case JsonParseEvent::Object_Start: {
					this->theState = JsonParserState::Starting_Object;
					this->currentObjectOrArrayStartIndex++;
					theString += "{";
					break;
				}
				case JsonParseEvent::Object_End: {
					this->theState = JsonParserState::Adding_Object_Elements;
					theString += "}";
					this->currentObjectOrArrayStartIndex--;
					if (theString[theString.size() - 2] == ',') {
						theString.erase(theString.begin() + theString.size() - 2);
					}
					break;
				}
				case JsonParseEvent::Array_Start: {
					this->theState = JsonParserState::Starting_Array;
					theString += "[";
					break;
				}
				case JsonParseEvent::Array_End: {
					this->theState = JsonParserState::Adding_Object_Elements;
					theString += "]";
					if (theString[theString.size() - 2] == ',') {
						theString.erase(theString.begin() + theString.size() - 2);
					}
					break;
				}
				case JsonParseEvent::Boolean: {
					theString += (*iterator).theValue;
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
				case JsonParseEvent::Null_Value: {
					theString += "null";
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
				case JsonParseEvent::Number_Double: {
					theString += (*iterator).theValue;
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
				case JsonParseEvent::Number_Float: {
					theString += (*iterator).theValue;
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
				case JsonParseEvent::Number_Integer: {
					theString += (*iterator).theValue;
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
				case JsonParseEvent::Number_Integer_Large: {
					theString += (*iterator).theValue;
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
				case JsonParseEvent::Number_Integer_Small: {
					theString += (*iterator).theValue;
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
				case JsonParseEvent::String: {
					theString += "\"";
					theString += (*iterator).theValue;
					theString += "\"";
					if (this->theState == JsonParserState::Starting_Object) {
						this->theState = JsonParserState::Adding_Object_Elements;
					}
					if (this->theState == JsonParserState::Starting_Array) {
						this->theState = JsonParserState::Adding_Array_Elements;
					}
					break;
				}
			}
			std::cout << "THE STRING FINAL 0202: " << theString << std::endl;
		}
		theString += "}";
		std::cout << "THE STRING FINAL 0101: " << theString << std::endl;
		return theString;
	}

	JsonRecord& JsonRecord::operator=(EnumConverter other) noexcept {
		JsonRecord theRecord{};
		if (other.vectorType) {
			theRecord.theEvent = JsonParseEvent::Array_Start;
			for (auto& value: static_cast<std::vector<uint64_t>>(other)) {
				theRecord.theEvent = JsonParseEvent::Number_Integer_Large;
				theRecord.theValue = std::to_string(static_cast<uint64_t>(value));
			}
			theRecord.theEvent = JsonParseEvent::Array_End;
		}
		return *this;
	}

	JsonRecord& JsonRecord::operator[](const char* keyName) noexcept {
		this->theKey = keyName;
		return *this;
	}

	std::basic_ostream<char>& operator<<(std::basic_ostream<char>& outputSttream, const std::string& (*theFunction)( void )) {
		outputSttream << theFunction();
		return outputSttream;
	}

	ConfigManager::ConfigManager(const DiscordCoreClientConfig& theConfigNew) {
		this->theConfig = theConfigNew;
	}

	const bool ConfigManager::doWePrintWebSocketSuccessMessages() {
		return this->theConfig.logOptions.logWebSocketSuccessMessages;
	}

	const bool ConfigManager::doWePrintWebSocketErrorMessages() {
		return this->theConfig.logOptions.logWebSocketErrorMessages;
	}

	const bool ConfigManager::doWePrintHttpsSuccessMessages() {
		return this->theConfig.logOptions.logHttpsSuccessMessages;
	}

	const bool ConfigManager::doWePrintHttpsErrorMessages() {
		return this->theConfig.logOptions.logHttpsErrorMessages;
	}

	const bool ConfigManager::doWePrintFFMPEGSuccessMessages() {
		return this->theConfig.logOptions.logFFMPEGSuccessMessages;
	}

	const bool ConfigManager::doWePrintFFMPEGErrorMessages() {
		return this->theConfig.logOptions.logFFMPEGErrorMessages;
	}

	const bool ConfigManager::doWePrintGeneralSuccessMessages() {
		return this->theConfig.logOptions.logGeneralSuccessMessages;
	}

	const bool ConfigManager::doWePrintGeneralErrorMessages() {
		return this->theConfig.logOptions.logGeneralErrorMessages;
	}

	const bool ConfigManager::doWeCacheChannels() {
		return this->theConfig.cacheOptions.cacheChannels;
	}

	const bool ConfigManager::doWeCacheUsers() {
		return this->theConfig.cacheOptions.cacheUsers;
	}

	const bool ConfigManager::doWeCacheGuilds() {
		return this->theConfig.cacheOptions.cacheGuilds;
	}

	const bool ConfigManager::doWeCacheRoles() {
		return this->theConfig.cacheOptions.cacheRoles;
	}

	const DiscordCoreInternal::UpdatePresenceData ConfigManager::getPresenceData() {
		return this->theConfig.presenceData;
	}

	const std::string ConfigManager::getBotToken() {
		return this->theConfig.botToken;
	}

	const uint32_t ConfigManager::getTotalShardCount() {
		return this->theConfig.shardOptions.totalNumberOfShards;
	}

	const uint32_t ConfigManager::getStartingShard() {
		return this->theConfig.shardOptions.startingShard;
	}

	const uint32_t ConfigManager::getShardCountForThisProcess() {
		return this->theConfig.shardOptions.numberOfShardsForThisProcess;
	}

	const std::string ConfigManager::getConnectionAddress() {
		return this->theConfig.connectionAddress;
	}

	void ConfigManager::setConnectionAddress(const std::string& connectionAddressNew) {
		this->theConfig.connectionAddress = connectionAddressNew;
	}

	const std::string ConfigManager::getConnectionPort() {
		return this->theConfig.connectionPort;
	}

	void ConfigManager::setConnectionPort(const std::string& connectionPortNew) {
		this->theConfig.connectionPort = connectionPortNew;
	}

	const std::vector<RepeatedFunctionData> ConfigManager::getFunctionsToExecute() {
		return this->theConfig.functionsToExecute;
	}

	const TextFormat ConfigManager::getTextFormat() {
		return this->theConfig.textFormat;
	}

	const GatewayIntents ConfigManager::getGatewayIntents() {
		return this->theConfig.theIntents;
	}

	StringWrapper& StringWrapper::operator=(StringWrapper&& other) noexcept {
		if (this != &other) {
			this->thePtr.reset(nullptr);
			this->thePtr = std::move(other.thePtr);
			other.thePtr.reset(nullptr);
			other.thePtr = nullptr;
		}
		return *this;
	}

	StringWrapper::StringWrapper(StringWrapper&& other) noexcept {
		*this = std::move(other);
	}

	StringWrapper& StringWrapper::operator=(const StringWrapper& other) {
		if (this != &other) {
			this->thePtr.reset(nullptr);
			std::stringstream theStream{};
			if (other.thePtr) {
				theStream << other.thePtr;
			}
			auto theLength = theStream.str().size();
			this->thePtr = std::make_unique<char[]>(theLength + 1);
			for (uint64_t x = 0; x < theLength; ++x) {
				this->thePtr[x] = other.thePtr[x];
			}
		}
		return *this;
	}

	StringWrapper::StringWrapper(const StringWrapper& other) {
		*this = other;
	}

	StringWrapper& StringWrapper::operator=(const std::string& theString) {
		auto theLength = theString.size();
		this->thePtr.reset(nullptr);
		this->thePtr = std::make_unique<char[]>(theLength + 1);
		for (int32_t x = 0; x < theLength; ++x) {
			this->thePtr[x] = theString[x];
		}
		return *this;
	}

	StringWrapper::StringWrapper(const std::string& theString) {
		*this = theString;
	}

	StringWrapper& StringWrapper::operator=(const char* theString) {
		if (theString) {
			this->thePtr.reset(nullptr);
			std::stringstream theStream{};
			theStream << theString;
			int64_t theLength = theStream.str().size();
			this->thePtr = std::make_unique<char[]>(theLength + 1);
			for (int64_t x = 0; x < theLength; ++x) {
				this->thePtr[x] = theString[x];
			}
		}
		return *this;
	}

	StringWrapper::StringWrapper(const char* theString) {
		*this = theString;
	}

	StringWrapper::operator std::basic_string<char, std::char_traits<char>, std::allocator<char>>() {
		std::stringstream theStream{};
		if (this->thePtr) {
			theStream << this->thePtr;
		}
		std::string theString{};
		for (uint32_t x = 0; x < theStream.str().size(); ++x) {
			theString.push_back(theStream.str()[x]);
		}
		return theString;
	}

	void StringWrapper::emplace_back(char theChar) {
		std::stringstream theStream{};
		if (this->thePtr) {
			theStream << this->thePtr;
		}
		auto theLength = theStream.str().size();
		this->thePtr = std::make_unique<char[]>(theLength + 2);
		for (uint64_t x = 0; x < theLength; ++x) {
			this->thePtr[x] = theStream.str()[x];
		}
		this->thePtr[theLength] = theChar;
	}

	size_t StringWrapper::size() {
		std::stringstream theStream{};
		if (this->thePtr) {
			theStream << this->thePtr;
		}
		auto theLength = theStream.str().size();
		return theLength;
	}

	const char* StringWrapper::data() {
		return this->thePtr.get();
	}

	ColorValue::ColorValue(uint32_t theColorValue) {
		this->theColor = theColorValue;
	}

	ColorValue::ColorValue(std::string theHexColorValue) {
		if (theHexColorValue == "") {
			theHexColorValue = "fefefe";
		}
		this->theColor = stoull(theHexColorValue, nullptr, 16);
	}

	RGBColorValue ColorValue::getRgbColorValue() {
		uint8_t red = static_cast<uint8_t>(this->theColor >> 16);
		uint8_t green = static_cast<uint8_t>(this->theColor >> 8);
		uint8_t blue = static_cast<uint8_t>(this->theColor);
		RGBColorValue theColor{};
		theColor.blue = blue;
		theColor.green = green;
		theColor.red = red;
		return theColor;
	}

	HexColorValue ColorValue::getHexColorValue() {
		std::stringstream theStream{};
		theStream << std::hex << this->theColor;
		return theStream.str();
	}

	uint32_t ColorValue::getIntColorValue() {
		return this->theColor;
	}

	IconHash& IconHash::operator=(const std::string theString) {
		std::string newHash{ theString };
		if (newHash.empty() ||newHash == "0") {
			this->highBits = 0;
			this->lowBits = 0;
			return *this;
		}
		if (newHash.length() == 34 && newHash.substr(0, 2) == "a_") {
			newHash = newHash.substr(2);
		}
		if (newHash.length() != 32 && newHash.length() != 33) {
			throw std::length_error("IconHash must be exactly 32 characters in length, passed value is: '" + std::to_string(newHash.size()) + "', in length.");
		}
		this->lowBits = fromString<uint64_t>(newHash.substr(0, 16), std::hex);
		this->highBits = fromString<uint64_t>(newHash.substr(16, 16), std::hex);
		return *this;
	}

	IconHash::IconHash(const std::string theString) noexcept {
		*this = theString;
	}

	std::string IconHash::getIconHash() noexcept {
		if (this->highBits == 0 ||this->lowBits == 0) {
			return {};
		} else {
			return std::string{ toHex(this->lowBits) + toHex(this->highBits) };
		}
	}

	bool IconHash::operator==(const IconHash& other) {
		return this->lowBits == other.lowBits && this->highBits == other.highBits;
	}

	uint64_t strtoull(const std::string& theString) {
		for (auto& value: theString) {
			if (!isdigit(value)) {
				return 0;
			}
		}
		if (theString != "") {
			return stoull(theString);
		} else {
			return 0;
		}
	}

	Permissions& Permissions::operator=(Permission&& other) {
		this->thePermissions = static_cast<uint64_t>(other);
		return *this;
	}

	Permissions::Permissions(Permission&& permsNew) {
		*this = std::move(permsNew);
	}

	Permissions& Permissions::operator=(const Permission& other) {
		this->thePermissions = static_cast<uint64_t>(other);
		return *this;
	}

	Permissions::Permissions(const Permission& permsNew) {
		*this = permsNew;
	}

	Permissions& Permissions::operator=(std::string&& other) {
		if (other.size() == 0 ||other == "") {
			this->thePermissions = 0;
		} else {
			for (auto& value: other) {
				this->thePermissions = stoull(other);
			}
		}
		other = "";
		return *this;
	}

	Permissions::Permissions(std::string&& permsNew) {
		*this = std::move(permsNew);
	}

	Permissions& Permissions::operator=(const std::string& other) {
		if (other.size() == 0 ||other == "") {
			this->thePermissions = 0;
		} else {
			this->thePermissions = stoull(other);
		}
		return *this;
	}

	Permissions::Permissions(const std::string& permsNew) {
		*this = permsNew;
	}


	Permissions& Permissions::operator=(uint64_t other) {
		this->thePermissions = other;
		return *this;
	}

	Permissions::Permissions(uint64_t permsNew) {
		*this = permsNew;
	}

	Permissions::operator uint64_t() {
		return this->thePermissions;
	}

	Permissions::operator std::string() {
		return std::string{ std::to_string(this->thePermissions) };
	}

	std::string Permissions::getCurrentChannelPermissions(const GuildMember& guildMember, ChannelData& channel) {
		std::string permsString = Permissions::computePermissions(guildMember, channel);
		return permsString;
	}

	bool Permissions::checkForPermission(const GuildMember& guildMember, ChannelData& channel, Permission permission) {
		std::string permissionsString = Permissions::computePermissions(guildMember, channel);
		if ((stoull(permissionsString) & static_cast<uint64_t>(permission)) == static_cast<uint64_t>(permission)) {
			return true;
		} else {
			return false;
		}
	}

	std::string Permissions::getCurrentGuildPermissions(const GuildMember& guildMember) {
		std::string permissions = Permissions::computeBasePermissions(guildMember);
		return permissions;
	}

	void Permissions::removePermissions(const std::vector<Permission>& permissionsToRemove) {
		uint64_t permissionsInteger = this->thePermissions;
		for (auto value: permissionsToRemove) {
			permissionsInteger &= ~static_cast<uint64_t>(value);
		}
		std::stringstream sstream{};
		sstream << permissionsInteger;
		*this = sstream.str();
	}

	void Permissions::addPermissions(const std::vector<Permission>& permissionsToAdd) {
		uint64_t permissionsInteger = this->thePermissions;
		for (auto value: permissionsToAdd) {
			permissionsInteger |= static_cast<uint64_t>(value);
		}
		std::stringstream sstream{};
		sstream << permissionsInteger;
		*this = sstream.str();
	}

	std::vector<std::string> Permissions::displayPermissions() {
		std::vector<std::string> returnVector{};
		uint64_t permissionsInteger = this->thePermissions;
		if (permissionsInteger & (1ll << 3)) {
			for (uint64_t x = 0; x < 41; ++x) {
				permissionsInteger |= 1ll << x;
			}
		}
		if (permissionsInteger & (1ll << 0)) {
			returnVector.emplace_back("Create Instant Invite");
		}
		if (permissionsInteger & (1ll << 1)) {
			returnVector.emplace_back("Kick Members");
		}
		if (permissionsInteger & (1ll << 2)) {
			returnVector.emplace_back("Ban Members");
		}
		if (permissionsInteger & (1ll << 3)) {
			returnVector.emplace_back("Administrator");
		}
		if (permissionsInteger & (1ll << 4)) {
			returnVector.emplace_back("Manage Channels");
		}
		if (permissionsInteger & (1ll << 5)) {
			returnVector.emplace_back("Manage Guild");
		}
		if (permissionsInteger & (1ll << 6)) {
			returnVector.emplace_back("Add Reactions");
		}
		if (permissionsInteger & (1ll << 7)) {
			returnVector.emplace_back("View Audit Log");
		}
		if (permissionsInteger & (1ll << 8)) {
			returnVector.emplace_back("Priority Speaker");
		}
		if (permissionsInteger & (1ll << 9)) {
			returnVector.emplace_back("Stream");
		}
		if (permissionsInteger & (1ll << 10)) {
			returnVector.emplace_back("View Channel");
		}
		if (permissionsInteger & (1ll << 11)) {
			returnVector.emplace_back("Send Messages");
		}
		if (permissionsInteger & (1ll << 12)) {
			returnVector.emplace_back("Send TTS Messages");
		}
		if (permissionsInteger & (1ll << 13)) {
			returnVector.emplace_back("Manage Messages");
		}
		if (permissionsInteger & (1ll << 14)) {
			returnVector.emplace_back("Embed Links");
		}
		if (permissionsInteger & (1ll << 15)) {
			returnVector.emplace_back("Attach Files");
		}
		if (permissionsInteger & (1ll << 16)) {
			returnVector.emplace_back("Read Message History");
		}
		if (permissionsInteger & (1ll << 17)) {
			returnVector.emplace_back("Mention Everyone");
		}
		if (permissionsInteger & (1ll << 18)) {
			returnVector.emplace_back("Use External Emoji");
		}
		if (permissionsInteger & (1ll << 19)) {
			returnVector.emplace_back("View Guild Insights");
		}
		if (permissionsInteger & (1ll << 20)) {
			returnVector.emplace_back("Connect");
		}
		if (permissionsInteger & (1ll << 21)) {
			returnVector.emplace_back("Speak");
		}
		if (permissionsInteger & (1ll << 22)) {
			returnVector.emplace_back("Mute Members");
		}
		if (permissionsInteger & (1ll << 23)) {
			returnVector.emplace_back("Deafen Members");
		}
		if (permissionsInteger & (1ll << 24)) {
			returnVector.emplace_back("Move Members");
		}
		if (permissionsInteger & (1ll << 25)) {
			returnVector.emplace_back("Use VAD");
		}
		if (permissionsInteger & (1ll << 26)) {
			returnVector.emplace_back("Change Nickname");
		}
		if (permissionsInteger & (1ll << 27)) {
			returnVector.emplace_back("Manage Nicknames");
		}
		if (permissionsInteger & (1ll << 28)) {
			returnVector.emplace_back("Manage Roles");
		}
		if (permissionsInteger & (1ll << 29)) {
			returnVector.emplace_back("Manage Webhooks");
		}
		if (permissionsInteger & (1ll << 30)) {
			returnVector.emplace_back("Manage Emojis And Stickers");
		}
		if (permissionsInteger & (1ll << 31)) {
			returnVector.emplace_back("Use Application Commands");
		}
		if (permissionsInteger & (1ll << 32)) {
			returnVector.emplace_back("Request To Speak");
		}
		if (permissionsInteger & (1ll << 33)) {
			returnVector.emplace_back("Manage Events");
		}
		if (permissionsInteger & (1ll << 34)) {
			returnVector.emplace_back("Manage Threads");
		}
		if (permissionsInteger & (1ll << 35)) {
			returnVector.emplace_back("Create Public Threads");
		}
		if (permissionsInteger & (1ll << 36)) {
			returnVector.emplace_back("Create Private Threads");
		}
		if (permissionsInteger & (1ll << 37)) {
			returnVector.emplace_back("Use External Stickers");
		}
		if (permissionsInteger & (1ll << 38)) {
			returnVector.emplace_back("Send Messages In Threads");
		}
		if (permissionsInteger & (1ll << 39)) {
			returnVector.emplace_back("Start Embedded Activities");
		}
		if (permissionsInteger & (1ll << 40)) {
			returnVector.emplace_back("Moderate Members");
		}
		return returnVector;
	}

	std::string Permissions::getCurrentPermissionString() {
		std::string theReturnString = std::to_string(this->thePermissions);
		return theReturnString;
	}

	std::string Permissions::getAllPermissions() {
		uint64_t allPerms{ 0 };
		for (uint64_t x = 0; x < 41; ++x) {
			allPerms |= 1ll << x;
		}
		std::stringstream stream{};
		stream << allPerms;
		return stream.str();
	}

	std::string Permissions::computeOverwrites(const std::string& basePermissions, const GuildMember& guildMember, ChannelData& channel) {
		if ((stoull(basePermissions) & static_cast<uint64_t>(Permission::Administrator)) == static_cast<uint64_t>(Permission::Administrator)) {
			return Permissions::getAllPermissions();
		}

		uint64_t permissions = stoull(basePermissions);
		for (int32_t x = 0; x < channel.permissionOverwrites.size(); ++x) {
			if (channel.permissionOverwrites[x].id == guildMember.guildId) {
				permissions &= ~channel.permissionOverwrites[x].deny;
				permissions |= channel.permissionOverwrites[x].allow;
				break;
			}
		}
		std::vector<RoleData> guildMemberRoles{};
		for (auto& value: guildMember.roles) {
			guildMemberRoles.emplace_back(Roles::getCachedRoleAsync({ .guildId = guildMember.guildId, .roleId = value }).get());
		}
		uint64_t allow{ 0 };
		uint64_t deny{ 0 };
		for (auto& value: guildMemberRoles) {
			for (int32_t x = 0; x < channel.permissionOverwrites.size(); ++x) {
				if (channel.permissionOverwrites[x].id == value.id) {
					allow |= channel.permissionOverwrites[x].allow;
					deny |= channel.permissionOverwrites[x].deny;
				}
			}
		}
		permissions &= ~deny;
		permissions |= allow;
		for (int32_t x = 0; x < channel.permissionOverwrites.size(); ++x) {
			if (channel.permissionOverwrites[x].id == guildMember.id) {
				permissions &= ~channel.permissionOverwrites[x].deny;
				permissions |= channel.permissionOverwrites[x].allow;
				break;
			}
		}
		return std::to_string(permissions);
	}

	std::string Permissions::computePermissions(const GuildMember& guildMember, ChannelData& channel) {
		std::string permissions = Permissions::computeBasePermissions(guildMember);
		permissions = Permissions::computeOverwrites(permissions, guildMember, channel);
		return permissions;
	}

	std::string Permissions::computeBasePermissions(const GuildMember& guildMember) {
		const GuildData guild = Guilds::getCachedGuildAsync({ .guildId = guildMember.guildId }).get();
		if (guild.ownerId == guildMember.id) {
			return Permissions::getAllPermissions();
		}
		std::vector<RoleData> guildRoles{};
		for (auto& value: guild.roles) {
			guildRoles.emplace_back(Roles::getCachedRoleAsync({ .guildId = guild.id, .roleId = value }).get());
		}
		RoleData roleEveryone{};
		for (auto& value: guildRoles) {
			if (value.id == guild.id) {
				roleEveryone = value;
			}
		}
		uint64_t permissions{};
		if (roleEveryone.permissions != 0) {
			permissions = roleEveryone.permissions;
		}
		GetGuildMemberRolesData getRolesData{};
		getRolesData.guildMember = guildMember;
		getRolesData.guildId = guildMember.guildId;
		std::vector<RoleData> guildMemberRoles{};
		for (auto& value: guildMember.roles) {
			guildMemberRoles.emplace_back(Roles::getCachedRoleAsync({ .guildId = guild.id, .roleId = value }).get());
		}
		for (auto& value: guildMemberRoles) {
			permissions |= value.permissions;
		}

		if ((permissions & static_cast<uint64_t>(Permission::Administrator)) == static_cast<uint64_t>(Permission::Administrator)) {
			return Permissions::getAllPermissions();
		}

		return std::to_string(permissions);
	}

	void reportException(const std::string& currentFunctionName, std::source_location theLocation) {
		try {
			auto currentException = std::current_exception();
			if (currentException) {
				std::rethrow_exception(currentException);
			}
		} catch (const std::exception& e) {
			std::stringstream theStream{};
			theStream << shiftToBrightRed() << "Error Report: \n"
					  << "Caught At: " << currentFunctionName << ", in File: " << theLocation.file_name() << " (" << std::to_string(theLocation.line()) << ":"
					  << std::to_string(theLocation.column()) << ")"
					  << "\nThe Error: \n"
					  << e.what() << reset() << std::endl
					  << std::endl;
			auto theReturnString = theStream.str();
			cout << theReturnString;
		}
	}

	void rethrowException(const std::string& currentFunctionName, std::source_location theLocation) {
		try {
			auto currentException = std::current_exception();
			if (currentException) {
				std::rethrow_exception(currentException);
			}
		} catch (const std::exception& e) {
			std::stringstream theStream{};
			theStream << shiftToBrightRed() << "Caught At: " << currentFunctionName << ", in File: " << theLocation.file_name() << " (" << std::to_string(theLocation.line()) << ":"
					  << std::to_string(theLocation.column()) << ")"
					  << "\nThe Error: \n"
					  << e.what() << reset();
			auto theReturnString = theStream.str();
			cout << theReturnString;
			if (std::current_exception()) {
				std::rethrow_exception(std::current_exception());
			}
		}
	}

	std::string constructMultiPartData(std::string theData, const std::vector<File>& files) {
		const std::string boundary("boundary25");
		const std::string partStart("--" + boundary + "\r\nContent-Type: application/octet-stream\r\nContent-Disposition: form-data; ");

		std::string content("--" + boundary);

		content += "\r\nContent-Type: application/json\r\nContent-Disposition: form-data; "
				   "name=\"payload_json\"\r\n\r\n";
		content += theData + "\r\n";
		if (files.size() == 1) {
			content += partStart + "name=\"file\"; filename=\"" + files[0].fileName + "\"" + "\r\n\r\n";
			content += files[0].data;
		} else {
			for (uint8_t x = 0; x < files.size(); ++x) {
				content += partStart + "name=\"files[" + std::to_string(x) + "]\"; filename=\"" + files[x].fileName + "\"\r\n\r\n";
				content += files[x].data;
				content += "\r\n";
			}
		}
		content += "\r\n--" + boundary + "--";
		return content;
	}

	std::string convertToLowerCase(const std::string& stringToConvert) {
		std::string newString;
		for (auto& value: stringToConvert) {
			if (isupper(static_cast<uint8_t>(value))) {
				newString += static_cast<char>(tolower(static_cast<uint8_t>(value)));
			} else {
				newString += value;
			}
		}
		return newString;
	}

	std::string base64Encode(const std::string& theString, bool url) {
		const char* base64_chars[2] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										"abcdefghijklmnopqrstuvwxyz"
										"0123456789"
										"+/",

			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789"
			"-_" };

		size_t len_encoded = (theString.size() + 2) / 3 * 4;

		uint8_t trailing_char = url ? '.' : '=';

		const char* base64_chars_ = base64_chars[url];

		std::string theReturnString{};
		theReturnString.reserve(len_encoded);
		DiscordCoreAPI::StopWatch theStopWatch{ 1500ms };
		uint64_t pos = 0;

		while (pos < theString.size()) {
			if (theStopWatch.hasTimePassed()) {
				break;
			}
			theReturnString.push_back(base64_chars_[(theString[static_cast<uint64_t>(pos + 0)] & 0xfc) >> 2]);

			if (static_cast<uint64_t>(pos + 1) < theString.size()) {
				theReturnString.push_back(base64_chars_[((theString[static_cast<int64_t>(pos + 0)] & 0x03) << 4) + ((theString[static_cast<int64_t>(pos + 1)] & 0xf0) >> 4)]);

				if (static_cast<uint64_t>(pos + 2) < theString.size()) {
					theReturnString.push_back(base64_chars_[((theString[static_cast<int64_t>(pos + 1)] & 0x0f) << 2) + ((theString[static_cast<int64_t>(pos + 2)] & 0xc0) >> 6)]);
					theReturnString.push_back(base64_chars_[theString[static_cast<int64_t>(pos + 2)] & 0x3f]);
				} else {
					theReturnString.push_back(base64_chars_[(theString[static_cast<int64_t>(pos + 1)] & 0x0f) << 2]);
					theReturnString.push_back(trailing_char);
				}
			} else {
				theReturnString.push_back(base64_chars_[(theString[static_cast<int64_t>(pos + 0)] & 0x03) << 4]);
				theReturnString.push_back(trailing_char);
				theReturnString.push_back(trailing_char);
			}

			pos += 3;
		}

		return theReturnString;
	}

	std::string loadFileContents(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::in | std::ios::binary);
		std::ostringstream stream{};
		stream << file.rdbuf();
		return stream.str();
	}

	std::string utf8MakeValid(const std::string& inputString) {
		std::string theReturnString{};
		for (auto& value: inputString) {
			if (value >= 128 ||value < 0) {
				int32_t theDifference = 0 - value;
				theReturnString.push_back(value + theDifference);
			} else {
				theReturnString.push_back(value);
			}
		}
		return theReturnString;
	}

	std::string urlEncode(const std::string& inputString) {
		std::ostringstream escaped{};
		escaped.fill('0');
		escaped << std::hex;

		for (std::string::const_iterator i = inputString.begin(), n = inputString.end(); i != n; ++i) {
			std::string::value_type c = (*i);

			if (isalnum(c) ||c == '-' ||c == '_' ||c == '.' ||c == '~') {
				escaped << c;
				continue;
			}

			escaped << std::uppercase;
			escaped << '%' << std::setw(2) << int32_t(static_cast<uint8_t>(c));
			escaped << std::nouppercase;
		}
		return escaped.str();
	}

	void spinLock(uint64_t timeInNsToSpinLockFor) {
		uint64_t startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		uint64_t timePassed{ 0 };
		while (timePassed < timeInNsToSpinLockFor) {
			timePassed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - startTime;
		}
	}

	std::string generateBase64EncodedKey() {
		std::string theReturnString{};
		theReturnString.resize(16);
		std::mt19937_64 randomEngine{ static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()) };
		for (uint32_t x = 0; x < 16; ++x) {
			theReturnString[x] = static_cast<uint8_t>((static_cast<float>(randomEngine()) / static_cast<float>(randomEngine.max())) * 255.0f);
		}
		theReturnString = base64Encode(theReturnString, false);
		return theReturnString;
	}

	std::string shiftToBrightGreen() {
		return std::string("\033[1;40;92m");
	}

	std::string shiftToBrightBlue() {
		return std::string("\033[1;40;96m");
	}

	std::string shiftToBrightRed() {
		return std::string("\033[1;40;91m");
	}

	bool nanoSleep(int64_t ns) {
#ifdef _WIN32
		HANDLE timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
		LARGE_INTEGER largeInt{ .QuadPart = -ns / 100 };
		if (!timer) {
			return false;
		}

		if (!SetWaitableTimerEx(timer, &largeInt, 0, NULL, NULL, NULL, 0)) {
			CloseHandle(timer);
			return false;
		}
		WaitForSingleObjectEx(timer, INFINITE, false);
		CloseHandle(timer);
#else
		std::this_thread::sleep_for(std::chrono::nanoseconds{ ns });
#endif
		return true;
	}

	std::string reset() {
		return std::string("\033[0m");
	}

	std::string getTimeAndDate() {
		const time_t now = std::time(nullptr);
		tm time = *std::localtime(&now);
		std::string timeStamp{};
		timeStamp.resize(48);
		if (time.tm_isdst) {
			if (time.tm_hour + 4 >= 24) {
				time.tm_hour = 0 + time.tm_hour + 4 - 24;

			} else {
				time.tm_hour = time.tm_hour + 4;
			}
		} else {
			if (time.tm_hour + 5 >= 24) {
				time.tm_hour = 0 + time.tm_hour + 5 - 24;
			} else {
				time.tm_hour = time.tm_hour + 5;
			}
		}
		size_t size = strftime(timeStamp.data(), 48, "%F %R", &time);
		timeStamp.resize(size);
		return timeStamp;
	}
	
	std::string escapeCharacters(std::string_view theString) {
		std::string theStringNew{};
		auto theSize = theString.size();
		for (int32_t x = 0; x < theSize; x++) {
			switch (static_cast<char>(theString[x])) {
				
				case 0x08: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), 'b');
					break;
				}
				case 0x09: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), 't');
					break;
				}
				case 0x0A: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), 'n');
					break;
				}
				case 0x0B: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), 'v');
					break;
				}
				case 0x0C: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), 'f');
					break;
				}
				case 0x0D: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), 'r');
					break;
				}
				case 0x22: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), '"');
					break;
				}
				case 0x00: {
					break;
				}
				case 0x5C: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), '\\');
					break;
				}
				case 0x27: {
					theStringNew.insert(theStringNew.end(), '\\');
					theStringNew.insert(theStringNew.end(), '\'');
					break;
				}
				default: {
					theStringNew.insert(theStringNew.end(), theString[x]);
				}
			}
		}
		return theStringNew;
	}

};
