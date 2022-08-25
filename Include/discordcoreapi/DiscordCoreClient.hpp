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
/// DiscordCoreClient01.hpp - Header file for the main/exposed DiscordCoreClient class.
/// May 12, 2021 Chris M.
/// https://discordcoreapi.com
/// \file DiscordCoreClient.hpp

#pragma once

#include <discordcoreapi/FoundationEntities.hpp>
#include <discordcoreapi/ApplicationCommandEntities.hpp>
#include <discordcoreapi/AutoModerationEntities.hpp>
#include <discordcoreapi/ChannelEntities.hpp>
#include <discordcoreapi/CoRoutine.hpp>
#include <discordcoreapi/CommandController.hpp>
#include <discordcoreapi/EventEntities.hpp>
#include <discordcoreapi/EventManager.hpp>
#include <discordcoreapi/GuildEntities.hpp>
#include <discordcoreapi/GuildMemberEntities.hpp>
#include <discordcoreapi/GuildScheduledEventEntities.hpp>
#include <discordcoreapi/Https.hpp>
#include <discordcoreapi/InputEvents.hpp>
#include <discordcoreapi/InteractionEntities.hpp>
#include <discordcoreapi/MessageEntities.hpp>
#include <discordcoreapi/ReactionEntities.hpp>
#include <discordcoreapi/RoleEntities.hpp>
#include <discordcoreapi/SSLClients.hpp>
#include <discordcoreapi/SongAPI.hpp>
#include <discordcoreapi/SoundCloudAPI.hpp>
#include <discordcoreapi/StageInstanceEntities.hpp>
#include <discordcoreapi/StickerEntities.hpp>
#include <discordcoreapi/ThreadEntities.hpp>
#include <discordcoreapi/ThreadPool.hpp>
#include <discordcoreapi/UserEntities.hpp>
#include <discordcoreapi/VoiceConnection.hpp>
#include <discordcoreapi/WebHookEntities.hpp>
#include <discordcoreapi/WebSocketEntities.hpp>
#include <discordcoreapi/YouTubeAPI.hpp>

namespace DiscordCoreAPI {

	class DiscordCoreAPI_Dll SIGTERMError : public std::runtime_error {
	  public:
		SIGTERMError(std::string theString);
	};

	class DiscordCoreAPI_Dll SIGSEGVError : public std::runtime_error {
	  public:
		SIGSEGVError(std::string theString);
	};

	class DiscordCoreAPI_Dll SIGINTError : public std::runtime_error {
	  public:
		SIGINTError(std::string theString);
	};

	class DiscordCoreAPI_Dll SIGILLError : public std::runtime_error {
	  public:
		SIGILLError(std::string theString);
	};

	class DiscordCoreAPI_Dll SIGABRTError : public std::runtime_error {
	  public:
		SIGABRTError(std::string theString);
	};

	class DiscordCoreAPI_Dll SIGFPEError : public std::runtime_error {
	  public:
		SIGFPEError(std::string theString);
	};

	using SoundCloudAPIMap = std::map<uint64_t, std::unique_ptr<DiscordCoreInternal::SoundCloudAPI>>;

	using YouTubeAPIMap = std::map<uint64_t, std::unique_ptr<DiscordCoreInternal::YouTubeAPI>>;

	using VoiceConnectionMap = std::map<uint64_t, std::unique_ptr<VoiceConnection>>;

	using SongAPIMap = std::map<uint64_t, std::unique_ptr<SongAPI>>;

	DiscordCoreAPI_Dll VoiceConnectionMap& getVoiceConnectionMap();

	DiscordCoreAPI_Dll SoundCloudAPIMap& getSoundCloudAPIMap();

	DiscordCoreAPI_Dll YouTubeAPIMap& getYouTubeAPIMap();

	DiscordCoreAPI_Dll SongAPIMap& getSongAPIMap();

	/**
	 * \addtogroup main_endpoints
	 * @{
	 */
	/// DiscordCoreClient - The main class for this library. \brief DiscordCoreClient - The main class for this library.
	class DiscordCoreAPI_Dll DiscordCoreClient {
	  public:
		friend class DiscordCoreInternal::WebSocketMessageHandler;
		friend class DiscordCoreInternal::WebSocketSSLShard;
		friend class DiscordCoreInternal::BaseSocketAgent;
		friend class VoiceConnection;
		friend class GuildData;
		friend class BotUser;
		friend class Guilds;

		/// DiscordCoreClient constructor. \brief DiscordCoreClient constructor.
		/// \param configData A DiscordCoreClientConfig structure to select various library options.
		DiscordCoreClient(DiscordCoreClientConfig configData);

		/// For registering a function with the CommandController. \brief For registering a function with the CommandController.
		/// \param functionNames A vector containing the possible names for activating this command/function.
		/// \param baseFunction A unique_ptr to the command to be registered.
		void registerFunction(const std::vector<std::string>& functionNames, std::unique_ptr<BaseFunction> baseFunction, CreateApplicationCommandData commandData);

		/// For collecting a reference to the CommandController. \brief For collecting a reference to the CommandController.
		/// \returns CommandController& A reference to the CommandController.
		CommandController& getCommandController();

		/// For collecting a reference to the EventManager. \brief For collecting a reference to the EventManager.
		/// \returns EventManager& A reference to the EventManager.
		EventManager& getEventManager();

		/// For collecting a copy of the current bot's User. \brief For collecting a copy of the current bot's User.
		/// \returns BotUser An instance of BotUser.
		BotUser getBotUser();

		/// Executes the library, and waits for completion. \brief Executes the library, and waits for completion.
		void runBot();

		~DiscordCoreClient() noexcept;

	  protected:
		std::map<uint32_t, std::unique_ptr<DiscordCoreInternal::BaseSocketAgent>> baseSocketAgentMap{};
		std::unique_ptr<DiscordCoreInternal::HttpsClient> httpsClient{ nullptr };
		StopWatch<std::chrono::milliseconds> theConnectionStopWatch{ 5100ms };
#ifdef _WIN32
		DiscordCoreInternal::WSADataWrapper theWSAData{};
#endif
		std::deque<ConnectionPackage> theConnections{};
		CommandController commandController{ this };
		ConfigManager configManager{};
		std::mutex connectionMutex{};
		EventManager eventManager{};
		BotUser currentUser{};

		std::vector<uint32_t> collectWorkerDimensions(uint32_t shardCount, uint32_t threadCount);

		GatewayBotData getGateWayBot();

		bool instantiateWebSockets();
	};
	/**@}*/
}// namespace DiscordCoreAPI
