/*
*
	Copyright 2021, 2022 Chris M.

	This file is part of DiscordCoreAPI.
	DiscordCoreAPI is free software: you can redistribute it and/or modify it under the terms of the GNU
	General Public License as published by the Free Software Foundation, either version 3 of the License,
	or (at your option) any later version.
	DiscordCoreAPI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with DiscordCoreAPI.
	If not, see <https://www.gnu.org/licenses/>.

*/
/// DiscordCoreClient01.cpp - Source file for the main/exposed DiscordCoreClient class.
/// May 12, 2021
/// Chris M.
/// https://discordcoreapi.com
/// \file DiscordCoreClient.cpp

#include <discordcoreapi/DiscordCoreClient.hpp>
#include <csignal>
#include <atomic>

namespace DiscordCoreAPI {

	namespace Globals {
		VoiceConnectionMap voiceConnectionMap{};
		SoundCloudAPIMap soundCloudAPIMap{};
		YouTubeAPIMap youtubeAPIMap{};
		SongAPIMap songAPIMap{};
		std::atomic_bool doWeQuit{ false };
		std::atomic_flag weveQuit{};
	}

	VoiceConnectionMap& getVoiceConnectionMap() {
		return Globals::voiceConnectionMap;
	}

	SoundCloudAPIMap& getSoundCloudAPIMap() {
		return Globals::soundCloudAPIMap;
	}

	YouTubeAPIMap& getYouTubeAPIMap() {
		return Globals::youtubeAPIMap;
	}

	SongAPIMap& getSongAPIMap() {
		return Globals::songAPIMap;
	}

	DiscordCoreClient::DiscordCoreClient(const std::string& botTokenNew, std::vector<RepeatedFunctionData> functionsToExecuteNew, LoggingOptions loggingOptionsNew,
		CacheOptions cacheOptionsNew, ShardingOptions shardOptionsNew) {
		curl_global_init(CURL_GLOBAL_NOTHING);
		std::signal(SIGTERM, &signalHandler);
		std::signal(SIGSEGV, &signalHandler);
		std::signal(SIGINT, &signalHandler);
		std::signal(SIGILL, &signalHandler);
		std::signal(SIGABRT, &signalHandler);
		std::signal(SIGFPE, &signalHandler);
		this->functionsToExecute = functionsToExecuteNew;
		DiscordCoreInternal::HttpSSLClient::initialize();
		this->loggingOptions = loggingOptionsNew;
		if (this->loggingOptions.logFFMPEGSuccessMessages) {
			av_log_set_level(AV_LOG_VERBOSE);
		} else if (this->loggingOptions.logFFMPEGErrorMessages) {
			av_log_set_level(AV_LOG_ERROR);
		} else {
			av_log_set_level(AV_LOG_QUIET);
		}
		if (sodium_init() == -1) {
			if (this->loggingOptions.logGeneralErrorMessages){
				std::cout << DiscordCoreAPI::shiftToBrightRed() << "LibSodium failed to initialize!" << std::endl << std::endl << reset();
			}			
		}
		this->shardingOptions = shardOptionsNew;
		this->cacheOptions = cacheOptionsNew;
		this->eventManager.onChannelCreation(&EventHandler::onChannelCreation);
		this->eventManager.onChannelUpdate(&EventHandler::onChannelUpdate);
		this->eventManager.onChannelDeletion(&EventHandler::onChannelDeletion);
		this->eventManager.onGuildCreation(&EventHandler::onGuildCreation);
		this->eventManager.onGuildUpdate(&EventHandler::onGuildUpdate);
		this->eventManager.onGuildDeletion(&EventHandler::onGuildDeletion);
		this->eventManager.onGuildMemberAdd(&EventHandler::onGuildMemberAdd);
		this->eventManager.onGuildMemberRemove(&EventHandler::onGuildMemberRemove);
		this->eventManager.onGuildMemberUpdate(&EventHandler::onGuildMemberUpdate);
		this->eventManager.onRoleCreation(&EventHandler::onRoleCreation);
		this->eventManager.onRoleUpdate(&EventHandler::onRoleUpdate);
		this->eventManager.onRoleDeletion(&EventHandler::onRoleDeletion);
		this->eventManager.onUserUpdate(&EventHandler::onUserUpdate);
		this->eventManager.onVoiceStateUpdate(&EventHandler::onVoiceStateUpdate);
		EventHandler::initialize(this->cacheOptions);
		this->httpClient = std::make_unique<DiscordCoreInternal::HttpClient>(botTokenNew, this->loggingOptions.logHttpSuccessMessages, this->loggingOptions.logHttpErrorMessages,
			this->loggingOptions.logFFMPEGSuccessMessages, this->loggingOptions.logFFMPEGErrorMessages);
		ApplicationCommands::initialize(this->httpClient.get());
		Channels::initialize(this->httpClient.get(), this->cacheOptions.cacheChannels);
		Guilds::initialize(this->httpClient.get(), this, this->cacheOptions.cacheGuilds);
		GuildMembers::initialize(this->httpClient.get(), this->cacheOptions.cacheGuildMembers);
		GuildScheduledEvents::initialize(this->httpClient.get());
		Interactions::initialize(this->httpClient.get());
		Messages::initialize(this->httpClient.get());
		Reactions::initialize(this->httpClient.get());
		Roles::initialize(this->httpClient.get(), this->cacheOptions.cacheRoles);
		Stickers::initialize(this->httpClient.get());
		StageInstances::initialize(this->httpClient.get());
		Threads::initialize(this->httpClient.get());
		Users::initialize(this->httpClient.get(), this->cacheOptions.cacheUsers);
		WebHooks::initialize(this->httpClient.get());
		this->botToken = botTokenNew;
	}

	void DiscordCoreClient::registerFunction(const std::vector<std::string>& functionNames, std::unique_ptr<BaseFunction> baseFunction) {
		this->commandController.registerFunction(functionNames, std::move(baseFunction));
	}

	BotUser DiscordCoreClient::getBotUser() {
		return this->currentUser;
	}

	void DiscordCoreClient::runBot() {
		this->instantiateWebSockets();
		if (!this->didWeStartFine) {
			return;
		}
		this->webSocketMap[std::to_string(this->shardingOptions.startingShard)]->getTheTask()->join();
		DiscordCoreAPI::Globals::weveQuit.test_and_set();
	}

	void DiscordCoreClient::instantiateWebSockets() {
		GatewayBotData gatewayData = this->getGateWayBot();
		if (gatewayData.url == "") {
			throw std::runtime_error("Sorry, but failed to collect the connection URL - did you remember to properly set your bot token?");
		}
		if (gatewayData.url == "") {
			if (this->loggingOptions.logGeneralErrorMessages) {
				std::cout << shiftToBrightRed << "Failed to collect the connection URL! Closing! Did you remember to properly set your bot token?" << reset << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::seconds{ 5 });
			return;
		}
		if (this->shardingOptions.startingShard + this->shardingOptions.numberOfShardsForThisProcess > this->shardingOptions.totalNumberOfShards) {
			if (this->loggingOptions.logGeneralErrorMessages) {
				std::cout << shiftToBrightRed() << "Your sharding options are incorrect! Please fix it!" << reset() << std::endl;
			}
			return;
		}
		int32_t shardGroupCount{ static_cast<int32_t>(
			ceil(static_cast<float>(this->shardingOptions.numberOfShardsForThisProcess) / static_cast<float>(gatewayData.sessionStartLimit.maxConcurrency))) };
		int32_t shardsPerGroup{ static_cast<int32_t>(ceil(static_cast<float>(this->shardingOptions.numberOfShardsForThisProcess) / static_cast<float>(shardGroupCount))) };
		for (int32_t x = 0; x < shardGroupCount; x += 1) {
			for (int32_t y = 0; y < shardsPerGroup; y += 1) {
				if (this->loggingOptions.logGeneralSuccessMessages) {
					std::cout << shiftToBrightBlue << "Connecting Shard " + std::to_string(x * shardsPerGroup + y + 1) << " of "
							  << this->shardingOptions.numberOfShardsForThisProcess
							  << std::string(" Shards for this process. (") + std::to_string(x * shardsPerGroup + y + 1 + this->shardingOptions.startingShard) + " of " +
							std::to_string(this->shardingOptions.totalNumberOfShards) + std::string(" Shards total across all processes.)")
							  << std::endl;
				}
				auto thePtr = std::unique_ptr<DiscordCoreInternal::BaseSocketAgent, WebSocketDeleter>(
					new DiscordCoreInternal::BaseSocketAgent{ this->botToken, gatewayData.url.substr(gatewayData.url.find("wss://") + std::string("wss://").size()),
						&this->eventManager, this, &this->commandController, &Globals::doWeQuit, this->loggingOptions.logWebSocketSuccessMessages,
						this->loggingOptions.logWebSocketErrorMessages, x * shardsPerGroup + y + this->shardingOptions.startingShard, this->shardingOptions.totalNumberOfShards },
					WebSocketDeleter{});
				this->webSocketMap.insert_or_assign(std::to_string(x * shardsPerGroup + y + this->shardingOptions.startingShard), std::move(thePtr));
			}
			if (shardGroupCount > 1 && x < shardGroupCount - 1) {
				if (this->loggingOptions.logGeneralSuccessMessages) {
					std::cout << "Waiting to connect the subsequent group of shards..." << std::endl << std::endl;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds{ 20000 });
			}
		}
		this->currentUser = BotUser{ Users::getCurrentUserAsync().get(), this->webSocketMap[std::to_string(this->shardingOptions.startingShard)].get() };
		if (this->loggingOptions.logGeneralSuccessMessages) {
			std::cout << shiftToBrightGreen() << "All of the shards are connected for the current process!" << reset() << std::endl << std::endl;
		}
		for (auto& value: this->functionsToExecute) {
			if (value.repeated) {
				TimeElapsedHandler onSend = [=, this](void) -> void {
					value.function(this);
				};
				this->threadIds.push_back(this->threadPool.storeThread(onSend, value.intervalInMs));
			} else {
				ThreadPool::executeFunctionAfterTimePeriod(value.function, value.intervalInMs, false, this);
			}
		}
		this->didWeStartFine = true;
	}

	GatewayBotData DiscordCoreClient::getGateWayBot() {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Get_Gateway_Bot;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Get;
		workload.relativePath = "/gateway/bot";
		workload.callStack = "DiscordCoreClient::getGateWayBot";
		return this->httpClient->submitWorkloadAndGetResult<GatewayBotData>(workload);
	}

	DiscordCoreClient::~DiscordCoreClient() {
		if (this) {
			curl_global_cleanup();
		}
	}

	void signalHandler(int32_t) {
		std::atomic_signal_fence(std::memory_order_seq_cst);
		DiscordCoreAPI::Globals::doWeQuit.store(true);
		while (!DiscordCoreAPI::Globals::weveQuit.test()) {
			std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		}
	}
}
