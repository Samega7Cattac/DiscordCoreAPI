﻿/*
	DiscordCoreAPI, A bot library for Discord, written in C++, and featuring explicit multithreading through the usage of custom, asynchronous C++ CoRoutines.

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

namespace DiscordCoreInternal {}

namespace DiscordCoreAPI {

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

	const bool ConfigManager::doWeCacheGuildMembers() {
		return this->theConfig.cacheOptions.cacheGuildMembers;
	}

	const bool ConfigManager::doWeCacheRoles() {
		return this->theConfig.cacheOptions.cacheRoles;
	}

	const std::string ConfigManager::getBotToken() {
		return this->theConfig.botToken;
	}

	const int32_t ConfigManager::getTotalShardCount() {
		return this->theConfig.shardOptions.totalNumberOfShards;
	}

	const int32_t ConfigManager::getStartingShard() {
		return this->theConfig.shardOptions.startingShard;
	}

	const int32_t ConfigManager::getShardCountForThisProcess() {
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

	StringWrapper& StringWrapper::operator=(const std::string& theString) {
		auto theLength = theString.size();
		this->thePtr = std::make_unique<char[]>(theLength + 1);
		for (int32_t x = 0; x < theLength; x++) {
			this->thePtr[x] = theString[x];
		}
		this->thePtr[theLength] = '\0';
		return *this;
	}

	StringWrapper::StringWrapper(const std::string& theString) {
		*this = theString;
	}

	StringWrapper& StringWrapper::operator=(const StringWrapper& other) {
		if (this != &other) {
			std::stringstream theStream{};
			if (other.thePtr) {
				theStream << other.thePtr;
			}
			auto theLength = theStream.str().size();
			this->thePtr = std::make_unique<char[]>(theLength + 1);
			for (uint64_t x = 0; x < theLength; x++) {
				this->thePtr[x] = other.thePtr[x];
			}
			this->thePtr[theLength] = '\0';
		}
		return *this;
	}

	StringWrapper::StringWrapper(const StringWrapper& other) {
		*this = other;
	}

	StringWrapper& StringWrapper::operator=(std::string& theString) {
		auto theLength = theString.size();
		this->thePtr = std::make_unique<char[]>(theLength + 1);
		for (int32_t x = 0; x < theLength; x++) {
			this->thePtr[x] = theString[x];
		}
		this->thePtr[theLength] = '\0';
		return *this;
	}

	StringWrapper::StringWrapper(std::string& theString) {
		*this = theString;
	}

	StringWrapper& StringWrapper::operator=(const char* theString) {
		if (theString) {
			std::stringstream theStream{};
			theStream << theString;
			int64_t theLength = theStream.str().size();
			this->thePtr = std::make_unique<char[]>(theLength + 1);
			for (int64_t x = 0; x < theLength; x++) {
				this->thePtr[x] = theString[x];
			}
			this->thePtr[theLength] = '\0';
		}
		return *this;
	}

	StringWrapper::StringWrapper(const char* theString) {
		*this = theString;
	};

	StringWrapper& StringWrapper::operator=(StringWrapper& other) {
		if (this != &other) {
			std::stringstream theStream{};
			if (other.thePtr) {
				theStream << other.thePtr;
			}
			auto theLength = theStream.str().size();
			this->thePtr = std::make_unique<char[]>(theLength + 1);
			for (uint64_t x = 0; x < theLength; x++) {
				this->thePtr[x] = other.thePtr[x];
			}
			this->thePtr[theLength] = '\0';
		}
		return *this;
	}

	StringWrapper::StringWrapper(StringWrapper& other) {
		*this = other;
	}

	StringWrapper::operator std::basic_string<char, std::char_traits<char>, std::allocator<char>>() {
		std::stringstream theStream{};
		if (this->thePtr) {
			theStream << this->thePtr;
		}
		std::string theString{};
		for (uint32_t x = 0; x < theStream.str().size(); x++) {
			theString.push_back(theStream.str()[x]);
		}
		return theString;
	}

	StringWrapper::operator nlohmann::json() {
		nlohmann::json theValue;
		std::string theString{};
		for (auto& value: static_cast<std::string>(*this)) {
			theString.push_back(value);
		}
		theValue = theString;
		return theValue;
	}

	void StringWrapper::push_back(char theChar) {
		std::stringstream theStream{};
		if (this->thePtr) {
			theStream << this->thePtr;
		}
		auto theLength = theStream.str().size();
		this->thePtr = std::make_unique<char[]>(theLength + 2);
		for (uint64_t x = 0; x < theLength; x++) {
			this->thePtr[x] = theStream.str()[x];
		}
		this->thePtr[theLength] = theChar;
		this->thePtr[theLength + 1] = '\0';
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

	bool operator!=(StringWrapper lhs, const char* rhs) {
		for (uint64_t x = 0; x < static_cast<std::string>(rhs).size(); x++) {
			if (static_cast<std::string>(lhs)[x] != static_cast<std::string>(rhs)[x]) {
				return false;
			}
		}
		return true;
	}

	bool operator==(std::string& lhs, StringWrapper& rhs) {
		for (uint64_t x = 0; x < static_cast<std::string>(rhs).size(); x++) {
			if (lhs[x] != static_cast<std::string>(rhs)[x]) {
				return false;
			}
		}
		return true;
	}

	std::string TimeStamp::getISO8601TimeStamp(const std::string& year, const std::string& month, const std::string& day, const std::string& hour, const std::string& minute,
		const std::string& second) {
		std::string theTimeStamp{};
		theTimeStamp += year + "-";
		if (month.size() < 2) {
			theTimeStamp += "0" + month + "-";
		} else {
			theTimeStamp += month + "-";
		}
		if (day.size() < 2) {
			theTimeStamp += "0" + day;
		} else {
			theTimeStamp += day;
		}
		if (hour.size() < 2) {
			theTimeStamp += "T0" + hour;
		} else {
			theTimeStamp += "T" + hour;
		}
		if (minute.size() < 2) {
			theTimeStamp += ":0" + minute;
		} else {
			theTimeStamp += ":" + minute;
		}
		if (second.size() < 2) {
			theTimeStamp += ":0" + second;
		} else {
			theTimeStamp += ":" + second;
		}
		return theTimeStamp;
	}

	TimeStamp::operator std::string() {
		return this->originalTimeStamp;
	}

	TimeStamp& TimeStamp::operator=(std::string&& originalTimeStampNew) {
		this->originalTimeStamp = std::move(originalTimeStampNew);
		return *this;
	}

	TimeStamp::TimeStamp(std::string&& originalTimeStampNew) {
		*this = std::move(originalTimeStampNew);
	}

	TimeStamp& TimeStamp::operator=(std::string& originalTimeStampNew) {
		this->originalTimeStamp = originalTimeStampNew;
		return *this;
	}

	TimeStamp::TimeStamp(std::string& originalTimeStampNew) {
		*this = originalTimeStampNew;
	}

	TimeStamp& TimeStamp::operator=(const TimeStamp& other) {
		this->originalTimeStamp = other.originalTimeStamp;
		this->timeStampInMs = other.timeStampInMs;
		this->minute = other.minute;
		this->second = other.second;
		this->month = other.month;
		this->hour = other.hour;
		this->year = other.year;
		this->day = other.day;
		return *this;
	}

	TimeStamp::TimeStamp(const TimeStamp& other) {
		*this = other;
	}

	TimeStamp& TimeStamp::operator=(TimeStamp& other) {
		this->originalTimeStamp = other.originalTimeStamp;
		this->timeStampInMs = other.timeStampInMs;
		this->minute = other.minute;
		this->second = other.second;
		this->month = other.month;
		this->hour = other.hour;
		this->year = other.year;
		this->day = other.day;
		return *this;
	}

	TimeStamp::TimeStamp(TimeStamp& other) {
		*this = other;
	}

	TimeStamp::TimeStamp(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second) {
		this->second = second;
		this->minute = minute;
		this->month = month;
		this->year = year;
		this->hour = hour;
		this->day = day;
	};

	TimeStamp::TimeStamp(uint64_t timeInMs) {
		this->timeStampInMs = timeInMs;
	}

	std::string TimeStamp::getFutureISO8601TimeStamp(int32_t minutesToAdd, int32_t hoursToAdd, int32_t daysToAdd, int32_t monthsToAdd, int32_t yearsToAdd) {
		std::time_t result = std::time(nullptr);
		int32_t secondsPerMinute{ 60 };
		int32_t minutesPerHour{ 60 };
		int32_t secondsPerHour{ minutesPerHour * secondsPerMinute };
		int32_t hoursPerDay{ 24 };
		int32_t secondsPerDay{ secondsPerHour * hoursPerDay };
		int32_t daysPerMonth{ 30 };
		int32_t secondsPerMonth{ secondsPerDay * daysPerMonth };
		int32_t daysPerYear{ 365 };
		int32_t secondsPerYear{ secondsPerDay * daysPerYear };
		int32_t secondsToAdd =
			(yearsToAdd * secondsPerYear) + (monthsToAdd * secondsPerMonth) + (daysToAdd * secondsPerDay) + (hoursToAdd * secondsPerHour) + (minutesToAdd * secondsPerMinute);
		result += secondsToAdd;
		auto resultTwo = std::localtime(&result);
		std::string theReturnString{};
		if (resultTwo->tm_isdst) {
			if (resultTwo->tm_hour + 4 >= 24) {
				resultTwo->tm_hour = resultTwo->tm_hour - 24;
				resultTwo->tm_mday++;
			}
			theReturnString = TimeStamp::getISO8601TimeStamp(std::to_string(resultTwo->tm_year + 1900), std::to_string(resultTwo->tm_mon + 1), std::to_string(resultTwo->tm_mday),
				std::to_string(resultTwo->tm_hour + 4), std::to_string(resultTwo->tm_min), std::to_string(resultTwo->tm_sec));
		} else {
			if (resultTwo->tm_hour + 5 >= 24) {
				resultTwo->tm_hour = resultTwo->tm_hour - 24;
				resultTwo->tm_mday++;
			}
			theReturnString = TimeStamp::getISO8601TimeStamp(std::to_string(resultTwo->tm_year + 1900), std::to_string(resultTwo->tm_mon + 1), std::to_string(resultTwo->tm_mday),
				std::to_string(resultTwo->tm_hour + 5), std::to_string(resultTwo->tm_min), std::to_string(resultTwo->tm_sec));
		}
		return theReturnString;
	}

	bool TimeStamp::hasTimeElapsed(uint64_t days, uint64_t hours, uint64_t minutes) {
		uint64_t startTimeRaw = convertTimestampToMsInteger();
		auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		uint64_t secondsPerMinute = 60;
		uint64_t secondsPerHour = secondsPerMinute * 60;
		uint64_t secondsPerDay = secondsPerHour * 24;
		auto targetElapsedTime = ((days * secondsPerDay) + (hours * secondsPerHour) + (minutes * secondsPerMinute)) * 1000;
		auto actualElapsedTime = currentTime - startTimeRaw;
		if (actualElapsedTime >= targetElapsedTime) {
			return true;
		} else {
			return false;
		}
	}

	std::string TimeStamp::convertMsToDurationString(uint64_t durationInMs) {
		std::string newString{};
		uint64_t msPerSecond{ 1000 };
		uint64_t secondsPerMinute{ 60 };
		uint64_t minutesPerHour{ 60 };
		uint64_t msPerMinute{ msPerSecond * secondsPerMinute };
		uint64_t msPerHour{ msPerMinute * minutesPerHour };
		uint64_t hoursLeft = static_cast<uint64_t>(trunc(durationInMs / msPerHour));
		uint64_t minutesLeft = static_cast<uint64_t>(trunc((durationInMs % msPerHour) / msPerMinute));
		uint64_t secondsLeft = static_cast<uint64_t>(trunc(((durationInMs % msPerHour) % msPerMinute) / msPerSecond));
		if (hoursLeft >= 1) {
			newString += std::to_string(hoursLeft) + " Hours, ";
			newString += std::to_string(minutesLeft) + " Minutes, ";
			newString += std::to_string(secondsLeft) + " Seconds.";
		} else if (minutesLeft >= 1) {
			newString += std::to_string(minutesLeft) + " Minutes, ";
			newString += std::to_string(secondsLeft) + " Seconds.";
		} else {
			newString += std::to_string(secondsLeft) + " Seconds.";
		}
		return newString;
	}

	std::string TimeStamp::convertTimeInMsToDateTimeString(TimeFormat timeFormat) {
		int64_t timeValue = this->timeStampInMs / 1000;
		time_t rawTime(timeValue);
		tm timeInfo = *localtime(&rawTime);
		std::string timeStamp{};
		timeStamp.resize(48);
		switch (timeFormat) {
			case TimeFormat::LongDate: {
				size_t sizeResponse = strftime(timeStamp.data(), 48, "%d %B %G", &timeInfo);
				timeStamp.resize(sizeResponse);
				break;
			}
			case TimeFormat::LongDateTime: {
				size_t sizeResponse = strftime(timeStamp.data(), 48, "%a %b %d %Y %X", &timeInfo);
				timeStamp.resize(sizeResponse);
				break;
			}
			case TimeFormat::LongTime: {
				size_t sizeResponse = strftime(timeStamp.data(), 48, "%T", &timeInfo);
				timeStamp.resize(sizeResponse);
				break;
			}
			case TimeFormat::ShortDate: {
				size_t sizeResponse = strftime(timeStamp.data(), 48, "%d/%m/%g", &timeInfo);
				timeStamp.resize(sizeResponse);
				break;
			}
			case TimeFormat::ShortDateTime: {
				size_t sizeResponse = strftime(timeStamp.data(), 48, "%d %B %G %R", &timeInfo);
				timeStamp.resize(sizeResponse);
				break;
			}
			case TimeFormat::ShortTime: {
				size_t sizeResponse = strftime(timeStamp.data(), 48, "%R", &timeInfo);
				timeStamp.resize(sizeResponse);
				break;
			}
			default: {
				break;
			}
		}
		return timeStamp;
	}

	std::string TimeStamp::getDateTimeStamp(TimeFormat timeFormat) {
		this->timeStampInMs = this->convertTimestampToMsInteger();
		return this->convertTimeInMsToDateTimeString(timeFormat);
	}

	std::string TimeStamp::getCurrentISO8601TimeStamp() {
		std::time_t result = std::time(nullptr);
		auto resultTwo = std::localtime(&result);
		std::string theReturnString = getISO8601TimeStamp(std::to_string(resultTwo->tm_year + 1900), std::to_string(resultTwo->tm_mon), std::to_string(resultTwo->tm_mday),
			std::to_string(resultTwo->tm_hour), std::to_string(resultTwo->tm_min), std::to_string(resultTwo->tm_sec));
		return theReturnString;
	}

	uint64_t TimeStamp::convertTimestampToMsInteger() {
		std::string timeStamp = this->originalTimeStamp;
		TimeStamp timeValue = TimeStamp(stoi(timeStamp.substr(0, 4)), stoi(timeStamp.substr(5, 6)), stoi(timeStamp.substr(8, 9)), stoi(timeStamp.substr(11, 12)),
			stoi(timeStamp.substr(14, 15)), stoi(timeStamp.substr(17, 18)));
		return timeValue.getTime() * 1000;
	}

	std::string TimeStamp::getOriginalTimeStamp() {
		return this->originalTimeStamp;
	}

	uint64_t TimeStamp::getTime() {
		uint64_t theValue{};
		for (int32_t x = 1970; x < this->year; x++) {
			theValue += this->secondsInJan;
			theValue += this->secondsInFeb;
			theValue += this->secondsInMar;
			theValue += this->secondsInApr;
			theValue += this->secondsInMay;
			theValue += this->secondsInJun;
			theValue += this->secondsInJul;
			theValue += this->secondsInAug;
			theValue += this->secondsInSep;
			theValue += this->secondsInOct;
			theValue += this->secondsInNov;
			theValue += this->secondsInDec;
			if (x % 4 == 0) {
				theValue += this->secondsPerDay;
			}
		}
		if (this->month > 0) {
			theValue += static_cast<uint64_t>((this->day - 1) * this->secondsPerDay);
			theValue += static_cast<uint64_t>(this->hour * this->secondsPerHour);
			theValue += static_cast<uint64_t>(this->minute * this->secondsPerMinute);
			theValue += this->second;
		}
		if (this->month > 1) {
			theValue += this->secondsInJan;
		}
		if (this->month > 2) {
			theValue += this->secondsInFeb;
		}
		if (this->month > 3) {
			theValue += this->secondsInMar;
		}
		if (this->month > 4) {
			theValue += this->secondsInApr;
		}
		if (this->month > 5) {
			theValue += this->secondsInMay;
		}
		if (this->month > 6) {
			theValue += this->secondsInJun;
		}
		if (this->month > 7) {
			theValue += this->secondsInJul;
		}
		if (this->month > 8) {
			theValue += this->secondsInAug;
		}
		if (this->month > 9) {
			theValue += this->secondsInSep;
		}
		if (this->month > 10) {
			theValue += this->secondsInOct;
		}
		if (this->month > 11) {
			theValue += this->secondsInNov;
		}
		return theValue;
	}

	Permissions& Permissions::operator=(std::string&& other) {
		if (other.size() == 0) {
			this->push_back('0');
		} else {
			for (auto& value: other) {
				this->push_back(value);
			}
		}
		other = "";
		return *this;
	}

	Permissions::Permissions(std::string&& permsNew) {
		*this = std::move(permsNew);
	}

	Permissions& Permissions::operator=(std::string& other) {
		if (other.size() == 0) {
			this->push_back('0');
		} else {
			for (auto& value: other) {
				this->push_back(value);
			}
		}
		return *this;
	}

	Permissions::Permissions(std::string& permsNew) {
		*this = permsNew;
	}

	Permissions::operator const char*() {
		return this->data();
	}

	std::string Permissions::getCurrentChannelPermissions(const GuildMember& guildMember, ChannelData& channel) {
		Permissions permission{};
		std::string permsString = permission.computePermissions(guildMember, channel);
		return permsString;
	}

	bool Permissions::checkForPermission(const GuildMember& guildMember, ChannelData& channel, Permission permission) {
		std::string permissionsString = Permissions::computePermissions(guildMember, channel);
		if ((stoll(permissionsString) & static_cast<int64_t>(permission)) == static_cast<int64_t>(permission)) {
			return true;
		} else {
			return false;
		}
	}

	std::string Permissions::getCurrentGuildPermissions(const GuildMember& guildMember) {
		Permissions permission{};
		std::string permissions = permission.computeBasePermissions(guildMember);
		return permissions;
	}

	void Permissions::removePermissions(const std::vector<Permission>& permissionsToRemove) {
		if (*this == "") {
			this->push_back('0');
		}
		int64_t permissionsInteger = stoll(static_cast<std::string>(static_cast<StringWrapper>(*this)));
		for (auto value: permissionsToRemove) {
			permissionsInteger &= ~static_cast<int64_t>(value);
		}
		std::stringstream sstream{};
		sstream << permissionsInteger;
		*this = sstream.str();
	}

	void Permissions::addPermissions(const std::vector<Permission>& permissionsToAdd) {
		if (*this == "") {
			this->push_back('0');
		}
		int64_t permissionsInteger = stoll(static_cast<std::string>(static_cast<StringWrapper>(*this)));
		for (auto value: permissionsToAdd) {
			permissionsInteger |= static_cast<int64_t>(value);
		}
		std::stringstream sstream{};
		sstream << permissionsInteger;
		*this = sstream.str();
	}

	std::vector<std::string> Permissions::displayPermissions() {
		std::vector<std::string> returnVector{};
		if (*this == "") {
			this->push_back('0');
		}
		int64_t permissionsInteger = stoll(static_cast<std::string>(static_cast<StringWrapper>(*this)));
		if (permissionsInteger & (1ll << 3)) {
			for (int64_t x = 0; x < 41; x++) {
				permissionsInteger |= 1ll << x;
			}
		}
		if (permissionsInteger & (1ll << 0)) {
			returnVector.push_back("Create Instant Invite");
		}
		if (permissionsInteger & (1ll << 1)) {
			returnVector.push_back("Kick Members");
		}
		if (permissionsInteger & (1ll << 2)) {
			returnVector.push_back("Ban Members");
		}
		if (permissionsInteger & (1ll << 3)) {
			returnVector.push_back("Administrator");
		}
		if (permissionsInteger & (1ll << 4)) {
			returnVector.push_back("Manage Channels");
		}
		if (permissionsInteger & (1ll << 5)) {
			returnVector.push_back("Manage Guild");
		}
		if (permissionsInteger & (1ll << 6)) {
			returnVector.push_back("Add Reactions");
		}
		if (permissionsInteger & (1ll << 7)) {
			returnVector.push_back("View Audit Log");
		}
		if (permissionsInteger & (1ll << 8)) {
			returnVector.push_back("Priority Speaker");
		}
		if (permissionsInteger & (1ll << 9)) {
			returnVector.push_back("Stream");
		}
		if (permissionsInteger & (1ll << 10)) {
			returnVector.push_back("View Channel");
		}
		if (permissionsInteger & (1ll << 11)) {
			returnVector.push_back("Send Messages");
		}
		if (permissionsInteger & (1ll << 12)) {
			returnVector.push_back("Send TTS Messages");
		}
		if (permissionsInteger & (1ll << 13)) {
			returnVector.push_back("Manage Messages");
		}
		if (permissionsInteger & (1ll << 14)) {
			returnVector.push_back("Embed Links");
		}
		if (permissionsInteger & (1ll << 15)) {
			returnVector.push_back("Attach Files");
		}
		if (permissionsInteger & (1ll << 16)) {
			returnVector.push_back("Read Message History");
		}
		if (permissionsInteger & (1ll << 17)) {
			returnVector.push_back("Mention Everyone");
		}
		if (permissionsInteger & (1ll << 18)) {
			returnVector.push_back("Use External Emoji");
		}
		if (permissionsInteger & (1ll << 19)) {
			returnVector.push_back("View Guild Insights");
		}
		if (permissionsInteger & (1ll << 20)) {
			returnVector.push_back("Connect");
		}
		if (permissionsInteger & (1ll << 21)) {
			returnVector.push_back("Speak");
		}
		if (permissionsInteger & (1ll << 22)) {
			returnVector.push_back("Mute Members");
		}
		if (permissionsInteger & (1ll << 23)) {
			returnVector.push_back("Deafen Members");
		}
		if (permissionsInteger & (1ll << 24)) {
			returnVector.push_back("Move Members");
		}
		if (permissionsInteger & (1ll << 25)) {
			returnVector.push_back("Use VAD");
		}
		if (permissionsInteger & (1ll << 26)) {
			returnVector.push_back("Change Nickname");
		}
		if (permissionsInteger & (1ll << 27)) {
			returnVector.push_back("Manage Nicknames");
		}
		if (permissionsInteger & (1ll << 28)) {
			returnVector.push_back("Manage Roles");
		}
		if (permissionsInteger & (1ll << 29)) {
			returnVector.push_back("Manage Webhooks");
		}
		if (permissionsInteger & (1ll << 30)) {
			returnVector.push_back("Manage Emojis And Stickers");
		}
		if (permissionsInteger & (1ll << 31)) {
			returnVector.push_back("Use Application Commands");
		}
		if (permissionsInteger & (1ll << 32)) {
			returnVector.push_back("Request To Speak");
		}
		if (permissionsInteger & (1ll << 33)) {
			returnVector.push_back("Manage Events");
		}
		if (permissionsInteger & (1ll << 34)) {
			returnVector.push_back("Manage Threads");
		}
		if (permissionsInteger & (1ll << 35)) {
			returnVector.push_back("Create Public Threads");
		}
		if (permissionsInteger & (1ll << 36)) {
			returnVector.push_back("Create Private Threads");
		}
		if (permissionsInteger & (1ll << 37)) {
			returnVector.push_back("Use External Stickers");
		}
		if (permissionsInteger & (1ll << 38)) {
			returnVector.push_back("Send Messages In Threads");
		}
		if (permissionsInteger & (1ll << 39)) {
			returnVector.push_back("Start Embedded Activities");
		}
		if (permissionsInteger & (1ll << 40)) {
			returnVector.push_back("Moderate Members");
		}
		return returnVector;
	}

	std::string Permissions::getCurrentPermissionString() {
		std::string theReturnString = *this;
		return theReturnString;
	}

	std::string Permissions::getAllPermissions() {
		int64_t allPerms{ 0 };
		for (int64_t x = 0; x < 41; x++) {
			allPerms |= 1ll << x;
		}
		std::stringstream stream{};
		stream << allPerms;
		return stream.str();
	}

	std::string Permissions::computeOverwrites(const std::string& basePermissions, const GuildMember& guildMember, ChannelData& channel) {
		if ((stoll(basePermissions) & static_cast<int64_t>(Permission::Administrator)) == static_cast<int64_t>(Permission::Administrator)) {
			return Permissions::getAllPermissions();
		}

		int64_t permissions = stoll(basePermissions);
		if (channel.permissionOverwrites.contains(guildMember.guildId)) {
			OverWriteData overWritesEveryone = channel.permissionOverwrites[guildMember.guildId];
			permissions &= ~stoll(static_cast<std::string>(static_cast<StringWrapper>(overWritesEveryone.deny)));
			permissions |= stoll(static_cast<std::string>(static_cast<StringWrapper>(overWritesEveryone.allow)));
		}

		std::vector<RoleData> guildMemberRoles{};
		for (auto& value: guildMember.roles) {
			guildMemberRoles.push_back(Roles::getCachedRoleAsync({ .guildId = guildMember.guildId, .roleId = value }).get());
		}
		int64_t allow{ 0 };
		int64_t deny{ 0 };
		for (auto& value: guildMemberRoles) {
			if (channel.permissionOverwrites.contains(value.id)) {
				OverWriteData currentChannelOverwrites = channel.permissionOverwrites[value.id];
				allow |= stoll(static_cast<std::string>(static_cast<StringWrapper>(currentChannelOverwrites.allow)));
				deny |= stoll(static_cast<std::string>(static_cast<StringWrapper>(currentChannelOverwrites.deny)));
			}
		}
		permissions &= ~deny;
		permissions |= allow;
		if (channel.permissionOverwrites.contains(guildMember.id)) {
			OverWriteData currentOverWrites = channel.permissionOverwrites[guildMember.id];
			permissions &= ~stoll(static_cast<std::string>(static_cast<StringWrapper>(currentOverWrites.deny)));
			permissions |= stoll(static_cast<std::string>(static_cast<StringWrapper>(currentOverWrites.allow)));
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
			guildRoles.push_back(Roles::getCachedRoleAsync({ .guildId = guild.id, .roleId = value }).get());
		}
		RoleData roleEveryone{};
		for (auto& value: guildRoles) {
			if (value.id == guild.id) {
				roleEveryone = value;
			}
		}
		int64_t permissions{};
		if (roleEveryone.permissions != "") {
			permissions = stoll(static_cast<std::string>(static_cast<StringWrapper>(roleEveryone.permissions)));
		}
		GetGuildMemberRolesData getRolesData{};
		getRolesData.guildMember = guildMember;
		getRolesData.guildId = guildMember.guildId;
		std::vector<RoleData> guildMemberRoles{};
		for (auto& value: guildMember.roles) {
			guildMemberRoles.push_back(Roles::getCachedRoleAsync({ .guildId = guild.id, .roleId = value }).get());
		}
		for (auto& value: guildMemberRoles) {
			permissions |= stoll(static_cast<std::string>(static_cast<StringWrapper>(value.permissions)));
		}

		if ((permissions & static_cast<int64_t>(Permission::Administrator)) == static_cast<int64_t>(Permission::Administrator)) {
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
					  << e.what() << reset();
			auto theReturnString = theStream.str();
			std::cout << theReturnString;
		}
	}

	std::string constructMultiPartData(nlohmann::json theData, const std::vector<File>& files) {
		const std::string boundary("boundary25");
		const std::string partStart("--" + boundary + "\r\nContent-Type: application/octet-stream\r\nContent-Disposition: form-data; ");

		std::string content("--" + boundary);

		content += "\r\nContent-Type: application/json\r\nContent-Disposition: form-data; "
				   "name=\"payload_json\"\r\n\r\n";
		content += theData.dump() + "\r\n";
		if (files.size() == 1) {
			content += partStart + "name=\"file\"; filename=\"" + files[0].fileName + "\"" + "\r\n\r\n";
			content += files[0].data;
		} else {
			for (uint8_t x = 0; x < files.size(); x++) {
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
			if (isupper(static_cast<unsigned char>(value))) {
				newString += static_cast<char>(tolower(static_cast<unsigned char>(value)));
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

		unsigned char trailing_char = url ? '.' : '=';

		const char* base64_chars_ = base64_chars[url];

		std::string theReturnString{};
		theReturnString.reserve(len_encoded);

		uint64_t pos = 0;

		while (pos < theString.size()) {
			theReturnString.push_back(base64_chars_[(theString[static_cast<int64_t>(pos + 0)] & 0xfc) >> 2]);

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
			if (value >= 128) {
				theReturnString.push_back(value - 128);
			} else if (value < 0) {
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

			if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
				escaped << c;
				continue;
			}

			escaped << std::uppercase;
			escaped << '%' << std::setw(2) << int32_t(static_cast<unsigned char>(c));
			escaped << std::nouppercase;
		}
		return escaped.str();
	}

	void spinLock(int64_t timeInNsToSpinLockFor) {
		int64_t startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		int64_t timePassed{ 0 };
		while (timePassed < timeInNsToSpinLockFor) {
			timePassed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - startTime;
		}
	}

	std::string generateBase64EncodedKey() {
		std::string theReturnString{};
		theReturnString.resize(16);
		std::mt19937_64 randomEngine{ static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()) };
		for (uint32_t x = 0; x < 16; x++) {
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

};
