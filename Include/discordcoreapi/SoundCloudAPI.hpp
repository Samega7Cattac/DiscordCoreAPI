/*
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
/// SoundCloudAPI.hpp - Header for the SoundCloud api related stuff.
/// Aug 25, 2021
/// https://discordcoreapi.com
/// \file SoundCloudAPI.hpp

#pragma once

#include <discordcoreapi/AudioDecoder.hpp>
#include <discordcoreapi/CoRoutine.hpp>
#include <discordcoreapi/Https.hpp>

namespace DiscordCoreInternal {

	class DiscordCoreAPI_Dll SoundCloudRequestBuilder {
	  public:
		SoundCloudRequestBuilder() noexcept = default;

	  protected:
		static std::string clientId;

		const std::string baseUrl02{ "https://api-v2.soundcloud.com" };
		DiscordCoreAPI::ConfigManager* configManager{ nullptr };
		const std::string baseUrl{ "https://soundcloud.com" };
		const std::string appVersion{ "1654762087" };
		HttpsClient* httpsClient{ nullptr };

		std::vector<DiscordCoreAPI::Song> collectSearchResults(const std::string& theString);

		DiscordCoreAPI::Song constructDownloadInfo(DiscordCoreAPI::Song& newSong);

		DiscordCoreAPI::Song collectFinalSong(DiscordCoreAPI::Song& newSong);

		std::string collectClientId();
	};

	class DiscordCoreAPI_Dll SoundCloudAPI : public SoundCloudRequestBuilder {
	  public:
		SoundCloudAPI(DiscordCoreAPI::ConfigManager* configManagerNew, HttpsClient* httpsClient, const Snowflake guildId);

		void weFailedToDownloadOrDecode(const DiscordCoreAPI::Song& newSong, std::stop_token stopToken, int32_t currentReconnectTries);

		void downloadAndStreamAudio(const DiscordCoreAPI::Song& newSong, std::stop_token stopToken, int32_t currentReconnectTries);

		std::vector<DiscordCoreAPI::Song> searchForSong(const std::string& searchQuery);

		DiscordCoreAPI::Song collectFinalSong(DiscordCoreAPI::Song& newSong);

	  protected:
		DiscordCoreAPI::Snowflake guildId{};
		const int32_t maxBufferSize{ 8192 };
		DiscordCoreAPI::Song theSong{};
	};

};// namespace DiscordCoreAPI
