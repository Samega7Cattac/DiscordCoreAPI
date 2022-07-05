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
/// WebSocketEntities.cpp - Source file for the webSocket related classes and structs.
/// May 13, 2021
/// https://discordcoreapi.com
/// \file WebSocketEntities.cpp

#include <discordcoreapi/WebSocketEntities.hpp>
#include <discordcoreapi/EventManager.hpp>
#include <discordcoreapi/DiscordCoreClient.hpp>
#include <concepts>

namespace DiscordCoreInternal {

	BaseSocketAgent::BaseSocketAgent(DiscordCoreAPI::DiscordCoreClient* discordCoreClientNew, std::atomic_bool* doWeQuitNew, int32_t currentBaseSocketAgentNew) noexcept {
		this->configManager = &discordCoreClientNew->configManager;
		this->currentBaseSocketAgent = currentBaseSocketAgentNew;
		this->discordCoreClient = discordCoreClientNew;
		this->doWeQuit = doWeQuitNew;
		this->taskThread = std::make_unique<std::jthread>([this](std::stop_token stopToken) {
			this->run(stopToken);
		});
	}

	void BaseSocketAgent::sendMessage(std::string& dataToSend, WebSocketSSLShard* theShard, bool priority) noexcept {
		if (theShard->areWeConnected01.load()) {
			try {
				if (this->configManager->doWePrintWebSocketSuccessMessages()) {
					std::cout << DiscordCoreAPI::shiftToBrightBlue() << "Sending WebSocket " + theShard->shard.dump() + std::string("'s Message: ") << std::endl
							  << dataToSend << DiscordCoreAPI::reset();
				}
				bool didWeWrite{ false };
				DiscordCoreAPI::StopWatch theStopWatch{ 5000ms };
				do {
					if (theStopWatch.hasTimePassed()) {
						break;
					}
					didWeWrite = theShard->writeData(dataToSend, priority);
				} while (!didWeWrite);
				if (!didWeWrite) {
					theShard->disconnect();
				}
			} catch (...) {
				if (this->configManager->doWePrintWebSocketErrorMessages()) {
					DiscordCoreAPI::reportException("BaseSocketAgent::sendMessage()");
				}
				this->onClosed(theShard);
			}
		}
	}

	void BaseSocketAgent::connectVoiceChannel(VoiceConnectInitData theData) noexcept {
		std::lock_guard theLock{ this->theMutex };
		this->voiceConnections.push(theData);
	}

	void BaseSocketAgent::connect(DiscordCoreAPI::ConnectionPackage thePackage) noexcept {
		while (!this->discordCoreClient->theStopWatch.hasTimePassed()) {
			std::this_thread::sleep_for(1ms);
		}
		this->discordCoreClient->theStopWatch.resetTimer();
		this->connections.push(thePackage);
		while (!this->sslShards.contains(thePackage.currentShard)) {
			std::this_thread::sleep_for(1ms);
		}
		while (!this->sslShards[thePackage.currentShard]->areWeConnected02.load()) {
			std::this_thread::sleep_for(1ms);
		}
	}

	void BaseSocketAgent::onClosed(WebSocketSSLShard* theShard) noexcept {
		if (this->maxReconnectTries > theShard->currentReconnectTries) {
			theShard->disconnect();
		} else if (this->maxReconnectTries<= theShard->currentReconnectTries) {
			this->doWeQuit->store(true);
			this->taskThread->request_stop();
		}
	}

	std::jthread* BaseSocketAgent::getTheTask() noexcept {
		return this->taskThread.get();
	}

	void BaseSocketAgent::getVoiceConnectionData(const VoiceConnectInitData& doWeCollect, WebSocketSSLShard* theShard) noexcept {
		if (theShard->areWeConnected02.load()) {
			try {
				DiscordCoreAPI::StopWatch<std::chrono::milliseconds> theStopWatch{ 5500ms };
				int32_t theCurrentIndex = theShard->shard[0];
				DiscordCoreAPI::UpdateVoiceStateData dataPackage{};
				dataPackage.channelId = 0;
				dataPackage.guildId = doWeCollect.guildId;
				dataPackage.selfDeaf = doWeCollect.selfDeaf;
				dataPackage.selfMute = doWeCollect.selfMute;
				theShard->userId = doWeCollect.userId;
				nlohmann::json newData = dataPackage;
				std::string theString{};
				this->stringifyJsonData(newData, theString, theShard->dataOpCode);
				bool didWeWrite{ false };
				theStopWatch.resetTimer();
				do {
					if (theStopWatch.hasTimePassed()) {
						break;
					}
					didWeWrite = theShard->writeData(theString, true);
				} while (!didWeWrite);
				if (!didWeWrite) {
					theShard->disconnect();
				}

				if (doWeCollect.channelId == 0) {
					return;
				}
				dataPackage.channelId = doWeCollect.channelId;
				nlohmann::json newData02 = dataPackage;
				std::string theString02{};
				this->stringifyJsonData(newData02, theString02, theShard->dataOpCode);
				theStopWatch.resetTimer();
				theShard->areWeCollectingData = true;
				do {
					if (theStopWatch.hasTimePassed()) {
						break;
					}
					didWeWrite = theShard->writeData(theString02, true);
				} while (!didWeWrite);
				if (!didWeWrite) {
					theShard->disconnect();
				}
				theStopWatch.resetTimer();
				while (theShard->areWeCollectingData) {
					if (theStopWatch.hasTimePassed()) {
						break;
					}
					std::this_thread::sleep_for(1ms);
				}
			} catch (...) {
				if (this->configManager->doWePrintWebSocketErrorMessages()) {
					DiscordCoreAPI::reportException("BaseSocketAgent::getVoiceConnectionData()");
				}
				this->onClosed(theShard);
			}
		}
	}

	void BaseSocketAgent::createHeader(std::string& outBuffer, uint64_t sendLength, WebSocketOpCode opCode) noexcept {
		try {
			outBuffer.push_back(static_cast<uint8_t>(opCode) | webSocketFinishBit);

			int32_t indexCount{ 0 };
			if (sendLength <= webSocketMaxPayloadLengthSmall) {
				outBuffer.push_back(static_cast<uint8_t>(sendLength));
				indexCount = 0;
			} else if (sendLength <= webSocketMaxPayloadLengthLarge) {
				outBuffer.push_back(webSocketPayloadLengthMagicLarge);
				indexCount = 2;
			} else {
				outBuffer.push_back(webSocketPayloadLengthMagicHuge);
				indexCount = 8;
			}
			for (int32_t x = indexCount - 1; x >= 0; x--) {
				outBuffer.push_back(static_cast<uint8_t>(sendLength >> x * 8));
			}

			outBuffer[1] |= webSocketMaskBit;
			outBuffer.push_back(0);
			outBuffer.push_back(0);
			outBuffer.push_back(0);
			outBuffer.push_back(0);
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("BaseSocketAgent::createHeader()");
			}
		}
	}

	void BaseSocketAgent::stringifyJsonData(const nlohmann::json& dataToSend, std::string& theString, WebSocketOpCode theOpCode) noexcept {
		std::string theVector{};
		std::string header{};
		if (this->configManager->getTextFormat() == DiscordCoreAPI::TextFormat::Etf) {
			theVector = this->erlPacker.parseJsonToEtf(dataToSend);
		} else {
			theVector = dataToSend.dump();
		}
		this->createHeader(header, theVector.size(), theOpCode);
		std::string theVectorNew{};
		theVectorNew.insert(theVectorNew.begin(), header.begin(), header.end());
		theVectorNew.insert(theVectorNew.begin() + header.size(), theVector.begin(), theVector.end());
		theString = theVectorNew;
	}

	void BaseSocketAgent::parseHeadersAndMessage(WebSocketSSLShard* theShard) noexcept {
		if (theShard->areWeConnected01.load()) {
			if (!theShard->areWeConnected02.load()){
				std::string newVector = theShard->getInputBuffer();
				if (newVector.find("\r\n\r\n") != std::string::npos) {
					newVector.erase(0, newVector.find("\r\n\r\n") + 4);
					theShard->inputBuffer.clear();
					theShard->inputBuffer.insert(theShard->inputBuffer.end(), newVector.begin(), newVector.end());
					theShard->areWeConnected02.store(true);
				}
			}
			if (theShard->inputBuffer.size() < 4) {
				return;
			}
			theShard->dataOpCode = static_cast<WebSocketOpCode>(theShard->inputBuffer[0] & ~webSocketFinishBit);
			switch (theShard->dataOpCode) {
				case WebSocketOpCode::Op_Continuation:
					[[fallthrough]];
				case WebSocketOpCode::Op_Text:
					[[fallthrough]];
				case WebSocketOpCode::Op_Binary:
					[[fallthrough]];
				case WebSocketOpCode::Op_Ping:
					[[fallthrough]];
				case WebSocketOpCode::Op_Pong: {
					uint8_t length01 = theShard->inputBuffer[1];
					theShard->messageOffset = 2;
					if (length01 & webSocketMaskBit) {
						return;
					}
					theShard->messageLength = length01;
					if (length01 == webSocketPayloadLengthMagicLarge) {
						if (theShard->inputBuffer.size() < 8) {
							return;
						}
						uint8_t length03 = theShard->inputBuffer[2];
						uint8_t length04 = theShard->inputBuffer[3];
						theShard->messageLength = static_cast<uint64_t>((length03 << 8) | length04);
						theShard->messageOffset += 2;
					} else if (length01 == webSocketPayloadLengthMagicHuge) {
						if (theShard->inputBuffer.size() < 10) {
							return;
						}
						theShard->messageLength = 0;
						for (int64_t x = 2, shift = 56; x < 10; ++x, shift -= 8) {
							uint8_t lengthNew = static_cast<uint8_t>(theShard->inputBuffer[x]);
							theShard->messageLength |= static_cast<uint64_t>((lengthNew & static_cast<uint64_t>(0xff)) << static_cast<uint64_t>(shift));
						}
						theShard->messageOffset += 8;
					}
					if (theShard->inputBuffer.size() < static_cast<uint64_t>(theShard->messageOffset) + static_cast<uint64_t>(theShard->messageLength)) {
						return;
					} else {
						std::string finalMessage{};
						finalMessage.insert(finalMessage.begin(), theShard->inputBuffer.begin() + theShard->messageOffset,
							theShard->inputBuffer.begin() + theShard->messageOffset + theShard->messageLength);
						theShard->processedMessages.push(finalMessage);
						theShard->inputBuffer.erase(theShard->inputBuffer.begin(), theShard->inputBuffer.begin() + theShard->messageOffset + theShard->messageLength);
						return;
					}
				}
				case WebSocketOpCode::Op_Close: {
					uint16_t close = theShard->inputBuffer[2] & 0xff;
					close <<= 8;
					close |= theShard->inputBuffer[3] & 0xff;
					theShard->closeCode = static_cast<WebSocketCloseCode>(close);
					theShard->inputBuffer.erase(theShard->inputBuffer.begin(), theShard->inputBuffer.begin() + 4);
					if (this->configManager->doWePrintWebSocketErrorMessages()) {
						std::cout << DiscordCoreAPI::shiftToBrightRed()
								  << "WebSocket " + this->sslShards[0]->shard.dump() + " Closed; Code: " << +static_cast<uint16_t>(this->sslShards[0]->closeCode)
								  << DiscordCoreAPI::reset() << std::endl
								  << std::endl;
					}
					this->onClosed(theShard);
					return;
				}
				default: {
					return;
				}
			}
		}
	}

	void BaseSocketAgent::checkForAndSendHeartBeat(WebSocketSSLShard* theShard, bool isImmediate) noexcept {
		if (theShard->areWeConnected02.load()) {
			try {
				if ((theShard->heartBeatStopWatch.hasTimePassed() && theShard->haveWeReceivedHeartbeatAck) || isImmediate) {
					nlohmann::json heartbeat{};
					heartbeat["d"] = theShard->lastNumberReceived;
					heartbeat["op"] = 1;
					std::string theString{};
					if (theShard->dataOpCode == WebSocketOpCode::Op_Binary) {
						this->stringifyJsonData(heartbeat, theString, WebSocketOpCode::Op_Binary);
					} else {
						this->stringifyJsonData(heartbeat, theString, WebSocketOpCode::Op_Text);
					}
					this->sendMessage(theString, theShard, true);
					theShard->haveWeReceivedHeartbeatAck = false;
					theShard->heartBeatStopWatch.resetTimer();
				}
			} catch (...) {
				if (this->configManager->doWePrintWebSocketErrorMessages()) {
					DiscordCoreAPI::reportException("BaseSocketAgent::checkForAndSendHeartBeat()");
				}
				this->onClosed(theShard);
			}
		}
	}

	void BaseSocketAgent::onMessageReceived(WebSocketSSLShard* theShard) noexcept {
		if (theShard->areWeConnected01.load()) {
			try {
				std::string messageNew{};
				if (theShard->processedMessages.size() > 0) {
					messageNew = theShard->processedMessages.front();
					theShard->processedMessages.pop();
				} else {
					return;
				}
				if (messageNew.size() == 0) {
					return;
				}
				nlohmann::json payload{};

				if (this->configManager->getTextFormat() == DiscordCoreAPI::TextFormat::Etf) {
					try {
						payload = this->erlPacker.parseEtfToJson(&messageNew);
					} catch (...) {
						if (this->configManager->doWePrintGeneralErrorMessages()) {
							DiscordCoreAPI::reportException("ErlPacker::parseEtfToJson()");
						}
						return;
					}
				} else {
					payload = nlohmann::json::parse(messageNew);
				}

				if (payload.contains("t") && !payload["t"].is_null()) {
					if (payload["t"] == "RESUMED") {
						theShard->areWeConnected03.store(true);
						theShard->currentReconnectTries = 0;
					}

					if (payload["t"] == "READY") {
						theShard->areWeConnected03.store(true);
						theShard->sessionId = payload["d"]["session_id"].get<std::string>();
						DiscordCoreAPI::UserData theUser{ payload["d"]["user"] };
						this->discordCoreClient->currentUser = DiscordCoreAPI::BotUser{ theUser, this };
						DiscordCoreAPI::Users::insertUser(theUser);
						theShard->currentReconnectTries = 0;
					}
				}

				if (payload.contains("s") && !payload["s"].is_null()) {
					theShard->lastNumberReceived = payload["s"].get<int32_t>();
				}

				if (payload.contains("op") && !payload["op"].is_null()) {
					switch (payload["op"].get<int32_t>()) {
						case 0: {
							if (payload.contains("d") && !payload["d"].is_null() && payload.contains("t") && !payload["t"].is_null()) {
								if (payload["t"] == "APPLICATION_COMMAND_PERMISSIONS_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnApplicationCommandPermissionsUpdateData> dataPackage{
										std::make_unique<DiscordCoreAPI::OnApplicationCommandPermissionsUpdateData>(payload["d"])
									};
									dataPackage->permissionData = payload["d"];
									this->discordCoreClient->eventManager.onApplicationCommandPermissionsUpdateEvent(*dataPackage);
								} else if (payload["t"] == "AUTO_MODERATION_RULE_CREATE") {
									std::unique_ptr<DiscordCoreAPI::OnAutoModerationRuleCreationData> dataPackage{
										std::make_unique<DiscordCoreAPI::OnAutoModerationRuleCreationData>()
									};
									dataPackage->theRule = payload["d"];
									this->discordCoreClient->eventManager.onAutoModerationRuleCreationEvent(*dataPackage);
								} else if (payload["t"] == "AUTO_MODERATION_RULE_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnAutoModerationRuleUpdateData> dataPackage{
										std::make_unique<DiscordCoreAPI::OnAutoModerationRuleUpdateData>()
									};
									dataPackage->theRule = payload["d"];
									this->discordCoreClient->eventManager.onAutoModerationRuleUpdateEvent(*dataPackage);
								} else if (payload["t"] == "AUTO_MODERATION_RULE_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnAutoModerationRuleDeletionData> dataPackage{
										std::make_unique<DiscordCoreAPI::OnAutoModerationRuleDeletionData>()
									};
									dataPackage->theRule = payload["d"];
									this->discordCoreClient->eventManager.onAutoModerationRuleDeletionEvent(*dataPackage);
								} else if (payload["t"] == "AUTO_MODERATION_ACTION_EXECUTION") {
									std::unique_ptr<DiscordCoreAPI::OnAutoModerationActionExecutionData> dataPackage{
										std::make_unique<DiscordCoreAPI::OnAutoModerationActionExecutionData>()
									};
									dataPackage->theData = payload["d"];
									this->discordCoreClient->eventManager.onAutoModerationActionExecutionEvent(*dataPackage);
								} else if (payload["t"] == "CHANNEL_CREATE") {
									std::unique_ptr<DiscordCoreAPI::OnChannelCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnChannelCreationData>() };
									dataPackage->channel = payload["d"];
									this->discordCoreClient->eventManager.onChannelCreationEvent(*dataPackage);
								} else if (payload["t"] == "CHANNEL_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnChannelUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnChannelUpdateData>() };
									if (payload["d"].contains("id") && !payload["d"]["id"].is_null()) {
										dataPackage->channelOld =
											DiscordCoreAPI::Channels::getCachedChannelAsync({ .channelId = stoull(payload["d"]["id"].get<std::string>()) }).get();
										dataPackage->channelNew = dataPackage->channelOld;
									}
									dataPackage->channelNew = payload["d"];
									this->discordCoreClient->eventManager.onChannelUpdateEvent(*dataPackage);
								} else if (payload["t"] == "CHANNEL_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnChannelDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnChannelDeletionData>() };
									dataPackage->channel = payload["d"];
									this->discordCoreClient->eventManager.onChannelDeletionEvent(*dataPackage);
								} else if (payload["t"] == "CHANNEL_PINS_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnChannelPinsUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnChannelPinsUpdateData>() };
									dataPackage->dataPackage = payload["d"];
									this->discordCoreClient->eventManager.onChannelPinsUpdateEvent(*dataPackage);
								} else if (payload["t"] == "THREAD_CREATE") {
									std::unique_ptr<DiscordCoreAPI::OnThreadCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnThreadCreationData>() };
									dataPackage->thread = payload["d"];
									this->discordCoreClient->eventManager.onThreadCreationEvent(*dataPackage);
								} else if (payload["t"] == "THREAD_UPATE") {
									std::unique_ptr<DiscordCoreAPI::OnThreadUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnThreadUpdateData>() };
									dataPackage->thread = payload["d"];
									this->discordCoreClient->eventManager.onThreadUpdateEvent(*dataPackage);
								} else if (payload["t"] == "THREAD_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnThreadDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnThreadDeletionData>() };
									dataPackage->thread = payload["d"];
									this->discordCoreClient->eventManager.onThreadDeletionEvent(*dataPackage);
								} else if (payload["t"] == "THREAD_LIST_SYNC") {
									std::unique_ptr<DiscordCoreAPI::OnThreadListSyncData> dataPackage{ std::make_unique<DiscordCoreAPI::OnThreadListSyncData>() };
									dataPackage->threadListSyncData = payload["d"];
									this->discordCoreClient->eventManager.onThreadListSyncEvent(*dataPackage);
								} else if (payload["t"] == "THREAD_MEMBER_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnThreadMemberUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnThreadMemberUpdateData>() };
									dataPackage->threadMember = payload["d"];
									this->discordCoreClient->eventManager.onThreadMemberUpdateEvent(*dataPackage);
								} else if (payload["t"] == "THREAD_MEMBERS_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnThreadMembersUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnThreadMembersUpdateData>() };
									dataPackage->threadMembersUpdateData = payload["d"];
									this->discordCoreClient->eventManager.onThreadMembersUpdateEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_CREATE") {
									DiscordCoreAPI::GuildData guildNew{};
									std::unique_ptr<DiscordCoreAPI::OnGuildCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildCreationData>() };
									guildNew = payload["d"];
									guildNew.discordCoreClient = this->discordCoreClient;
									dataPackage->guild = guildNew;
									this->discordCoreClient->eventManager.onGuildCreationEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildUpdateData>() };
									if (payload["d"].contains("id") && !payload["d"]["id"].is_null()) {
										dataPackage->guildOld = DiscordCoreAPI::Guilds::getCachedGuildAsync({ .guildId = stoull(payload["d"]["id"].get<std::string>()) }).get();
										dataPackage->guildNew = dataPackage->guildOld;
									}
									dataPackage->guildNew = payload["d"];
									this->discordCoreClient->eventManager.onGuildUpdateEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildDeletionData>() };
									dataPackage->guild = payload["d"];
									this->discordCoreClient->eventManager.onGuildDeletionEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_BAN_ADD") {
									std::unique_ptr<DiscordCoreAPI::OnGuildBanAddData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildBanAddData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("user") && !payload["d"]["user"].is_null()) {
										DiscordCoreAPI::User newData{};
										newData = payload["d"]["user"];
										dataPackage->user = newData;
									}
									this->discordCoreClient->eventManager.onGuildBanAddEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_BAN_REMOVE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildBanRemoveData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildBanRemoveData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("user") && !payload["d"]["user"].is_null()) {
										DiscordCoreAPI::User newData{};
										newData = payload["d"]["user"];
										dataPackage->user = newData;
									}
									this->discordCoreClient->eventManager.onGuildBanRemoveEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_EMOJIS_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildEmojisUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildEmojisUpdateData>() };
									dataPackage->updateData = payload["d"];
									this->discordCoreClient->eventManager.onGuildEmojisUpdateEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_STICKERS_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildStickersUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildStickersUpdateData>() };
									dataPackage->updateData = payload["d"];
									this->discordCoreClient->eventManager.onGuildStickersUpdateEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_INTEGRATIONS_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildIntegrationsUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildIntegrationsUpdateData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									this->discordCoreClient->eventManager.onGuildIntegrationsUpdateEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_MEMBER_ADD") {
									std::unique_ptr<DiscordCoreAPI::OnGuildMemberAddData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildMemberAddData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildMember.guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									dataPackage->guildMember = payload["d"];
									dataPackage->discordCoreClient = this->discordCoreClient;
									this->discordCoreClient->eventManager.onGuildMemberAddEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_MEMBER_REMOVE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildMemberRemoveData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildMemberRemoveData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("user") && !payload["d"]["user"].is_null()) {
										DiscordCoreAPI::User newData{};
										newData = payload["d"]["user"];
										dataPackage->user = newData;
									}
									this->discordCoreClient->eventManager.onGuildMemberRemoveEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_MEMBER_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnGuildMemberUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildMemberUpdateData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildMemberNew.guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("user") && payload["d"]["user"].contains("id") && !payload["d"]["user"]["id"].is_null()) {
										dataPackage->guildMemberOld =
											DiscordCoreAPI::GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = stoull(payload["d"]["user"]["id"].get<std::string>()),
																										.guildId = stoull(payload["d"]["guild_id"].get<std::string>()) })
												.get();
										dataPackage->guildMemberNew = dataPackage->guildMemberOld;
									}
									dataPackage->guildMemberNew = payload["d"];
									this->discordCoreClient->eventManager.onGuildMemberUpdateEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_MEMBERS_CHUNK") {
									std::unique_ptr<DiscordCoreAPI::OnGuildMembersChunkData> dataPackage{ std::make_unique<DiscordCoreAPI::OnGuildMembersChunkData>() };
									dataPackage->chunkEventData = payload["d"];
									this->discordCoreClient->eventManager.onGuildMembersChunkEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_ROLE_CREATE") {
									std::unique_ptr<DiscordCoreAPI::OnRoleCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnRoleCreationData>() };
									if (payload["d"].contains("role") && !payload["d"]["role"].is_null()) {
										dataPackage->role = payload["d"]["role"];
									}
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									this->discordCoreClient->eventManager.onRoleCreationEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_ROLE_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnRoleUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnRoleUpdateData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									dataPackage->roleNew = payload["d"]["role"];
									if (payload["d"].contains("role") && !payload["d"]["role"].is_null()) {
										dataPackage->roleOld = DiscordCoreAPI::Roles::getCachedRoleAsync(
											{ .guildId = dataPackage->guildId, .roleId = stoull(payload["d"]["role"]["id"].get<std::string>()) })
																   .get();
										dataPackage->roleNew = dataPackage->roleOld;
									}
									this->discordCoreClient->eventManager.onRoleUpdateEvent(*dataPackage);
								} else if (payload["t"] == "GUILD_ROLE_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnRoleDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnRoleDeletionData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("role_id") && !payload["d"]["role_id"].is_null()) {
										dataPackage->roleOld = DiscordCoreAPI::Roles::getCachedRoleAsync(
											{ .guildId = dataPackage->guildId, .roleId = stoull(payload["d"]["role_id"].get<std::string>()) })
																   .get();
									}
									this->discordCoreClient->eventManager.onRoleDeletionEvent(*dataPackage);
								} else if (payload["t"] == "INTEGRATION_CREATE") {
									std::unique_ptr<DiscordCoreAPI::OnIntegrationCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnIntegrationCreationData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("integration") && !payload["d"]["integration"].is_null()) {
										dataPackage->integrationData = payload["d"]["integration"];
									}
									this->discordCoreClient->eventManager.onIntegrationCreationEvent(*dataPackage);
								} else if (payload["t"] == "INTEGRATION_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnIntegrationUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnIntegrationUpdateData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("integration") && !payload["d"]["integration"].is_null()) {
										dataPackage->integrationData = payload["d"]["integration"];
									}
									this->discordCoreClient->eventManager.onIntegrationUpdateEvent(*dataPackage);
								} else if (payload["t"] == "INTEGRATION_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnIntegrationDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnIntegrationDeletionData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("application_id") && !payload["d"]["application_id"].is_null()) {
										dataPackage->applicationId = stoull(payload["d"]["application_id"].get<std::string>());
									}
									if (payload["d"].contains("id") && !payload["d"]["id"].is_null()) {
										dataPackage->id = stoull(payload["d"]["id"].get<std::string>());
									}
									this->discordCoreClient->eventManager.onIntegrationDeletionEvent(*dataPackage);
								} else if (payload["t"] == "INTERACTION_CREATE") {
									std::unique_ptr<DiscordCoreAPI::InteractionData> interactionData{ std::make_unique<DiscordCoreAPI::InteractionData>() };
									*interactionData = payload["d"];
									std::unique_ptr<DiscordCoreAPI::InputEventData> eventData{ std::make_unique<DiscordCoreAPI::InputEventData>(*interactionData) };
									if (interactionData->type == DiscordCoreAPI::InteractionType::Application_Command) {
										eventData->responseType = DiscordCoreAPI::InputEventResponseType::Unset;
										*eventData->interactionData = *interactionData;
										std::unique_ptr<DiscordCoreAPI::OnInteractionCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnInteractionCreationData>() };
										dataPackage->interactionData = *interactionData;
										std::unique_ptr<DiscordCoreAPI::CommandData> commandData{ std::make_unique<DiscordCoreAPI::CommandData>(*eventData) };
										this->discordCoreClient->commandController.checkForAndRunCommand(*commandData);
										this->discordCoreClient->eventManager.onInteractionCreationEvent(*dataPackage);
										std::unique_ptr<DiscordCoreAPI::OnInputEventCreationData> eventCreationData{ std::make_unique<DiscordCoreAPI::OnInputEventCreationData>() };
										eventCreationData->inputEventData = *eventData;
										this->discordCoreClient->eventManager.onInputEventCreationEvent(*eventCreationData);
									} else if (interactionData->type == DiscordCoreAPI::InteractionType::Message_Component) {
										if (interactionData->data.componentData.componentType == DiscordCoreAPI::ComponentType::Button) {
											eventData->responseType = DiscordCoreAPI::InputEventResponseType::Unset;
											*eventData->interactionData = *interactionData;
											std::unique_ptr<DiscordCoreAPI::OnInteractionCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnInteractionCreationData>() };
											dataPackage->interactionData = *interactionData;
											if (DiscordCoreAPI::ButtonCollector::buttonInteractionBufferMap.contains(
													std::to_string(eventData->getChannelId()) + std::to_string(eventData->getMessageId()))) {
												DiscordCoreAPI::ButtonCollector::buttonInteractionBufferMap[std::to_string(eventData->getChannelId()) +
													std::to_string(eventData->getMessageId())]
													->send(*interactionData);
											}
											this->discordCoreClient->eventManager.onInteractionCreationEvent(*dataPackage);
										} else if (interactionData->data.componentData.componentType == DiscordCoreAPI::ComponentType::SelectMenu) {
											eventData->responseType = DiscordCoreAPI::InputEventResponseType::Unset;
											*eventData->interactionData = *interactionData;
											std::unique_ptr<DiscordCoreAPI::OnInteractionCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnInteractionCreationData>() };
											dataPackage->interactionData = *interactionData;
											if (DiscordCoreAPI::SelectMenuCollector::selectMenuInteractionBufferMap.contains(
													std::to_string(eventData->getChannelId()) + std::to_string(eventData->getMessageId()))) {
												DiscordCoreAPI::SelectMenuCollector::selectMenuInteractionBufferMap[std::to_string(eventData->getChannelId()) +
													std::to_string(eventData->getMessageId())]
													->send(*interactionData);
											}
											this->discordCoreClient->eventManager.onInteractionCreationEvent(*dataPackage);
										}
									} else if (interactionData->type == DiscordCoreAPI::InteractionType::Modal_Submit) {
										eventData->responseType = DiscordCoreAPI::InputEventResponseType::Unset;
										*eventData->interactionData = *interactionData;
										std::unique_ptr<DiscordCoreAPI::OnInteractionCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnInteractionCreationData>() };
										dataPackage->interactionData = *interactionData;
										std::unique_ptr<DiscordCoreAPI::OnInputEventCreationData> eventCreationData{ std::make_unique<DiscordCoreAPI::OnInputEventCreationData>() };
										eventCreationData->inputEventData = *eventData;
										if (DiscordCoreAPI::ModalCollector::modalInteractionBufferMap.contains(std::to_string(eventData->getChannelId()))) {
											DiscordCoreAPI::ModalCollector::modalInteractionBufferMap[std::to_string(eventData->getChannelId())]->send(
												eventData->getInteractionData());
										}
										this->discordCoreClient->eventManager.onInteractionCreationEvent(*dataPackage);
									} else if (interactionData->type == DiscordCoreAPI::InteractionType::Application_Command_Autocomplete) {
										eventData->responseType = DiscordCoreAPI::InputEventResponseType::Unset;
										*eventData->interactionData = *interactionData;
										std::unique_ptr<DiscordCoreAPI::OnInteractionCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnInteractionCreationData>() };
										dataPackage->interactionData = *interactionData;
										std::unique_ptr<DiscordCoreAPI::OnAutoCompleteEntryData> autocompleteEntryData{
											std::make_unique<DiscordCoreAPI::OnAutoCompleteEntryData>()
										};
										autocompleteEntryData->inputEvent = *eventData;
										this->discordCoreClient->eventManager.onAutoCompleteEntryEvent(*autocompleteEntryData);
										this->discordCoreClient->eventManager.onInteractionCreationEvent(*dataPackage);
									}
								} else if (payload["t"] == "INVITE_CREATE") {
									std::unique_ptr<DiscordCoreAPI::OnInviteCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnInviteCreationData>() };
									dataPackage->invite = payload["d"];
									this->discordCoreClient->eventManager.onInviteCreationEvent(*dataPackage);
								} else if (payload["t"] == "INVITE_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnInviteDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnInviteDeletionData>() };
									if (payload["d"].contains("channel_id") && !payload["d"]["channel_id"].is_null()) {
										dataPackage->channelId = stoull(payload["d"]["channel_id"].get<std::string>());
									}
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("code") && !payload["d"]["code"].is_null()) {
										dataPackage->code = payload["d"]["code"];
									}
									this->discordCoreClient->eventManager.onInviteDeletionEvent(*dataPackage);
								} else if (payload["t"] == "MESSAGE_CREATE") {
									std::unique_ptr<DiscordCoreAPI::Message> message{ std::make_unique<DiscordCoreAPI::Message>() };
									*message = payload["d"];
									std::unique_ptr<DiscordCoreAPI::OnMessageCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnMessageCreationData>() };
									dataPackage->message = *message;
									for (auto& [key, value]: DiscordCoreAPI::MessageCollector::messagesBufferMap) {
										value->send(*message);
									}
									this->discordCoreClient->eventManager.onMessageCreationEvent(*dataPackage);
									if (message->content.find("!registerapplicationcommands") != std::string::npos) {
										std::unique_ptr<DiscordCoreAPI::CommandData> commandData{ std::make_unique<DiscordCoreAPI::CommandData>() };
										commandData->commandName = "registerapplicationcommands";
										DiscordCoreAPI::CommandData commandDataNew = *commandData;
										this->discordCoreClient->commandController.checkForAndRunCommand(commandDataNew);
									}
									std::unique_ptr<DiscordCoreAPI::OnInputEventCreationData> eventCreationData{ std::make_unique<DiscordCoreAPI::OnInputEventCreationData>() };
								} else if (payload["t"] == "MESSAGE_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnMessageUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnMessageUpdateData>() };
									dataPackage->messageNew = payload["d"];
									for (auto& [key, value]: DiscordCoreAPI::MessageCollector::messagesBufferMap) {
										value->send(dataPackage->messageNew);
									}
									this->discordCoreClient->eventManager.onMessageUpdateEvent(*dataPackage);
								} else if (payload["t"] == "MESSAGE_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnMessageDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnMessageDeletionData>() };
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("channel_id") && !payload["d"]["channel_id"].is_null()) {
										dataPackage->channelId = stoull(payload["d"]["channel_id"].get<std::string>());
									}
									if (payload["d"].contains("id") && !payload["d"]["id"].is_null()) {
										dataPackage->messageId = stoull(payload["d"]["id"].get<std::string>());
									}
									this->discordCoreClient->eventManager.onMessageDeletionEvent(*dataPackage);
								} else if (payload["t"] == "MESSAGE_DELETE_BULK") {
									std::unique_ptr<DiscordCoreAPI::OnMessageDeleteBulkData> dataPackage{ std::make_unique<DiscordCoreAPI::OnMessageDeleteBulkData>() };
									if (payload["d"].contains("channel_id") && !payload["d"]["channel_id"].is_null()) {
										dataPackage->channelId = stoull(payload["d"]["channel_id"].get<std::string>());
									}
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("ids") && !payload["d"]["ids"].is_null()) {
										for (auto& value: payload["d"]["ids"]) {
											dataPackage->ids.push_back(stoull(value.get<std::string>()));
										}
									}
									this->discordCoreClient->eventManager.onMessageDeleteBulkEvent(*dataPackage);
								} else if (payload["t"] == "MESSAGE_REACTION_ADD") {
									std::unique_ptr<DiscordCoreAPI::OnReactionAddData> dataPackage{ std::make_unique<DiscordCoreAPI::OnReactionAddData>() };
									dataPackage->reaction = payload["d"];
									this->discordCoreClient->eventManager.onReactionAddEvent(*dataPackage);
								} else if (payload["t"] == "MESSAGE_REACTION_REMOVE") {
									std::unique_ptr<DiscordCoreAPI::OnReactionRemoveData> dataPackage{ std::make_unique<DiscordCoreAPI::OnReactionRemoveData>() };
									dataPackage->reactionRemoveData = payload["d"];
									this->discordCoreClient->eventManager.onReactionRemoveEvent(*dataPackage);
								} else if (payload["t"] == "MESSAGE_REACTION_REMOVE_ALL") {
									std::unique_ptr<DiscordCoreAPI::OnReactionRemoveAllData> dataPackage{ std::make_unique<DiscordCoreAPI::OnReactionRemoveAllData>() };
									if (payload["d"].contains("channel_id") && !payload["d"]["channel_id"].is_null()) {
										dataPackage->channelId = stoull(payload["d"]["channel_id"].get<std::string>());
									}
									if (payload["d"].contains("message_id") && !payload["d"]["message_id"].is_null()) {
										dataPackage->messageId = stoull(payload["d"]["message_id"].get<std::string>());
									}
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									this->discordCoreClient->eventManager.onReactionRemoveAllEvent(*dataPackage);
								} else if (payload["t"] == "MESSAGE_REACTION_REMOVE_EMOJI") {
									std::unique_ptr<DiscordCoreAPI::OnReactionRemoveEmojiData> dataPackage{ std::make_unique<DiscordCoreAPI::OnReactionRemoveEmojiData>() };
									if (payload["d"].contains("channel_id") && !payload["d"]["channel_id"].is_null()) {
										dataPackage->channelId = stoull(payload["d"]["channel_id"].get<std::string>());
									}
									if (payload["d"].contains("message_id") && !payload["d"]["message_id"].is_null()) {
										dataPackage->messageId = stoull(payload["d"]["message_id"].get<std::string>());
									}
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									dataPackage->emoji = payload["d"]["emoji"];
									this->discordCoreClient->eventManager.onReactionRemoveEmojiEvent(*dataPackage);
								} else if (payload["t"] == "PRESENCE_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnPresenceUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnPresenceUpdateData>() };
									dataPackage->presenceData = payload["d"];
									this->discordCoreClient->eventManager.onPresenceUpdateEvent(*dataPackage);
								} else if (payload["t"] == "STAGE_INSTANCE_CREATE") {
									std::unique_ptr<DiscordCoreAPI::OnStageInstanceCreationData> dataPackage{ std::make_unique<DiscordCoreAPI::OnStageInstanceCreationData>() };
									dataPackage->stageInstance = payload["d"];
									this->discordCoreClient->eventManager.onStageInstanceCreationEvent(*dataPackage);
								} else if (payload["t"] == "STAGE_INSTANCE_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnStageInstanceUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnStageInstanceUpdateData>() };
									dataPackage->stageInstance = payload["d"];
									this->discordCoreClient->eventManager.onStageInstanceUpdateEvent(*dataPackage);
								} else if (payload["t"] == "STAGE_INSTANCE_DELETE") {
									std::unique_ptr<DiscordCoreAPI::OnStageInstanceDeletionData> dataPackage{ std::make_unique<DiscordCoreAPI::OnStageInstanceDeletionData>() };
									dataPackage->stageInstance = payload["d"];
									this->discordCoreClient->eventManager.onStageInstanceDeletionEvent(*dataPackage);
								} else if (payload["t"] == "TYPING_START") {
									std::unique_ptr<DiscordCoreAPI::OnTypingStartData> dataPackage{ std::make_unique<DiscordCoreAPI::OnTypingStartData>() };
									dataPackage->typingStartData = payload["d"];
									this->discordCoreClient->eventManager.onTypingStartEvent(*dataPackage);
								} else if (payload["t"] == "USER_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnUserUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnUserUpdateData>() };
									DiscordCoreAPI::User newData{};
									newData = payload["d"]["user"];
									dataPackage->userNew = newData;
									dataPackage->userOld = DiscordCoreAPI::Users::getCachedUserAsync({ .userId = dataPackage->userNew.id }).get();
									this->discordCoreClient->eventManager.onUserUpdateEvent(*dataPackage);
								} else if (payload["t"] == "VOICE_STATE_UPDATE") {
									if (theShard->areWeCollectingData && !theShard->stateUpdateCollected && !theShard->serverUpdateCollected &&
										payload["d"]["member"]["user"]["id"] == std::to_string(theShard->userId)) {
										theShard->voiceConnectionData = VoiceConnectionData{};
										theShard->voiceConnectionData.sessionId = payload["d"]["session_id"].get<std::string>();
										theShard->stateUpdateCollected = true;
									} else if (theShard->areWeCollectingData && !theShard->stateUpdateCollected) {
										theShard->voiceConnectionData.sessionId = payload["d"]["session_id"].get<std::string>();
										if (theShard->voiceConnectionDataBufferMap.contains(stoull(payload["d"]["guild_id"].get<std::string>()))) {
											theShard->voiceConnectionDataBufferMap[stoull(payload["d"]["guild_id"].get<std::string>())]->send(theShard->voiceConnectionData);
										}
										theShard->serverUpdateCollected = false;
										theShard->stateUpdateCollected = false;
										theShard->areWeCollectingData = false;
									}
									std::unique_ptr<DiscordCoreAPI::OnVoiceStateUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnVoiceStateUpdateData>() };
									dataPackage->voiceStateData = payload["d"];
									this->discordCoreClient->eventManager.onVoiceStateUpdateEvent(*dataPackage);
								} else if (payload["t"] == "VOICE_SERVER_UPDATE") {
									if (theShard->areWeCollectingData && !theShard->serverUpdateCollected && !theShard->stateUpdateCollected) {
										theShard->voiceConnectionData = VoiceConnectionData{};
										theShard->voiceConnectionData.endPoint = payload["d"]["endpoint"].get<std::string>();
										theShard->voiceConnectionData.token = payload["d"]["token"].get<std::string>();
										theShard->serverUpdateCollected = true;
									} else if (theShard->areWeCollectingData && !theShard->serverUpdateCollected) {
										theShard->voiceConnectionData.endPoint = payload["d"]["endpoint"].get<std::string>();
										theShard->voiceConnectionData.token = payload["d"]["token"].get<std::string>();
										if (theShard->voiceConnectionDataBufferMap.contains(stoull(payload["d"]["guild_id"].get<std::string>()))) {
											theShard->voiceConnectionDataBufferMap[stoull(payload["d"]["guild_id"].get<std::string>())]->send(theShard->voiceConnectionData);
										}
										theShard->serverUpdateCollected = false;
										theShard->stateUpdateCollected = false;
										theShard->areWeCollectingData = false;
									}
									std::unique_ptr<DiscordCoreAPI::OnVoiceServerUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnVoiceServerUpdateData>() };
									if (payload["d"].contains("endpoint") && !payload["d"]["endpoint"].is_null()) {
										dataPackage->endpoint = payload["d"]["endpoint"];
									}
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									if (payload["d"].contains("token") && !payload["d"]["token"].is_null()) {
										dataPackage->token = payload["d"]["token"];
									}
									this->discordCoreClient->eventManager.onVoiceServerUpdateEvent(*dataPackage);
								} else if (payload["t"] == "WEBHOOKS_UPDATE") {
									std::unique_ptr<DiscordCoreAPI::OnWebhookUpdateData> dataPackage{ std::make_unique<DiscordCoreAPI::OnWebhookUpdateData>() };
									if (payload["d"].contains("channel_id") && !payload["d"]["channel_id"].is_null()) {
										dataPackage->channelId = stoull(payload["d"]["channel_id"].get<std::string>());
									}
									if (payload["d"].contains("guild_id") && !payload["d"]["guild_id"].is_null()) {
										dataPackage->guildId = stoull(payload["d"]["guild_id"].get<std::string>());
									}
									this->discordCoreClient->eventManager.onWebhookUpdateEvent(*dataPackage);
								}
							}
							break;
						}
						case 1: {
							this->checkForAndSendHeartBeat(theShard, true);
							break;
						}
						case 7: {
							if (this->configManager->doWePrintWebSocketErrorMessages()) {
								std::cout << DiscordCoreAPI::shiftToBrightBlue() << "Shard " + theShard->shard.dump() + " Reconnecting (Type 7)!" << DiscordCoreAPI::reset()
										  << std::endl
										  << std::endl;
							}
							theShard->areWeResuming = true;
							this->onClosed(theShard);
							return;
						}
						case 9: {
							if (this->configManager->doWePrintWebSocketErrorMessages()) {
								std::cout << DiscordCoreAPI::shiftToBrightBlue() << "Shard " + theShard->shard.dump() + " Reconnecting (Type 9)!" << DiscordCoreAPI::reset()
										  << std::endl
										  << std::endl;
							}
							std::mt19937_64 randomEngine{ static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()) };
							int32_t numOfMsToWait =
								static_cast<int32_t>(1000.0f + ((static_cast<float>(randomEngine()) / static_cast<float>(randomEngine.max())) * static_cast<float>(4000.0f)));
							std::this_thread::sleep_for(std::chrono::milliseconds{ numOfMsToWait });
							if (payload["d"] == true) {
								theShard->areWeResuming = true;
							} else {
								theShard->areWeResuming = false;
							}
							this->onClosed(theShard);
							return;
						}
						case 10: {
							if (payload["d"].contains("heartbeat_interval") && !payload["d"]["heartbeat_interval"].is_null()) {
								theShard->heartBeatStopWatch =
									DiscordCoreAPI::StopWatch<std::chrono::milliseconds>{ std::chrono::milliseconds{ payload["d"]["heartbeat_interval"] } };
							}
							if (theShard->areWeResuming) {
								WebSocketResumeData resumeData{};
								resumeData.botToken = this->configManager->getBotToken();
								resumeData.sessionId = theShard->sessionId;
								resumeData.lastNumberReceived = theShard->lastNumberReceived;
								nlohmann::json resumePayload = resumeData;
								std::string theString{};
								if (theShard->dataOpCode == WebSocketOpCode::Op_Binary) {
									this->stringifyJsonData(resumePayload, theString, WebSocketOpCode::Op_Binary);
								} else {
									this->stringifyJsonData(resumePayload, theString, WebSocketOpCode::Op_Text);
								}
								this->sendMessage(theString, theShard, true);
							} else {
								WebSocketIdentifyData identityData{};
								identityData.botToken = this->configManager->getBotToken();
								identityData.currentShard = theShard->shard[0];
								identityData.numberOfShards = theShard->shard[1];
								identityData.intents = static_cast<int64_t>(this->configManager->getGatewayIntents());
								nlohmann::json identityJson = identityData;
								std::string theString{};
								if (theShard->dataOpCode == WebSocketOpCode::Op_Binary) {
									this->stringifyJsonData(identityJson, theString, WebSocketOpCode::Op_Binary);
								} else {
									this->stringifyJsonData(identityJson, theString, WebSocketOpCode::Op_Text);
								}
								this->sendMessage(theString, theShard, true);
							}
							theShard->areWeHeartBeating = false;
							break;
						}
						case 11: {
							theShard->haveWeReceivedHeartbeatAck = true;
							break;
						}
					}
				}

				if (this->configManager->doWePrintWebSocketSuccessMessages() && !payload.is_null()) {
					std::cout << DiscordCoreAPI::shiftToBrightGreen() << "Message received from WebSocket " + theShard->shard.dump() + std::string(": ") << payload.dump()
							  << DiscordCoreAPI::reset() << std::endl
							  << std::endl;
				}
			} catch (...) {
				if (this->configManager->doWePrintWebSocketErrorMessages()) {
					DiscordCoreAPI::reportException("BaseSocketAgent::onMessageReceived()");
				}
				this->onClosed(theShard);
			}
		}
	}

	void BaseSocketAgent::connectVoiceInternal() noexcept {
		while (!this->theVCStopWatch.hasTimePassed()) {
			std::this_thread::sleep_for(1ms);
		}
		this->theVCStopWatch.resetTimer();
		while (!this->theVCStopWatch.hasTimePassed()) {
			std::this_thread::sleep_for(1ms);
		}
		this->theVCStopWatch.resetTimer();
		VoiceConnectInitData theConnectionData = this->voiceConnections.front();
		this->voiceConnections.pop();
		DiscordCoreAPI::getVoiceConnectionMap()[theConnectionData.guildId] =
			std::make_unique<DiscordCoreAPI::VoiceConnection>(this, theConnectionData, &this->discordCoreClient->configManager);
		DiscordCoreAPI::getVoiceConnectionMap()[theConnectionData.guildId]->connect();
	}

	void BaseSocketAgent::run(std::stop_token stopToken) noexcept {
		try {
			while (!stopToken.stop_requested() && !this->doWeQuit->load()) {
				if (this->connections.size() > 0) {
					this->internalConnect();
				}
				if (this->voiceConnectionsToDisconnect.size() > 0) {
					this->disconnectVoice();
				}
				if (this->voiceConnections.size() > 0) {
					this->connectVoiceInternal();
				}
				WebSocketSSLShard::processIO(this->sslShards);
				for (auto& [key, value]: this->sslShards) {
					if (value->areWeStillConnected() && value->inputBuffer.size() > 0) {
						this->parseHeadersAndMessage(value.get());
					}
					if (value->areWeStillConnected() && value->processedMessages.size() > 0) {
						while (value->processedMessages.size() > 0) {
							if (value) {
								this->onMessageReceived(value.get());
							}
						}
					}
					if (value->areWeStillConnected()) {
						this->checkForAndSendHeartBeat(value.get());
						if (value && this->heartbeatInterval != 0 && !value->areWeHeartBeating) {
							value->areWeHeartBeating = true;
							value->heartBeatStopWatch = DiscordCoreAPI::StopWatch{ std::chrono::milliseconds{ this->heartbeatInterval } };
						}
					}
				}
				std::this_thread::sleep_for(1ms);
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("BaseSocketAgent::run()");
			}
		}
	}

	void BaseSocketAgent::disconnectVoice() noexcept {
		uint64_t theDCData = this->voiceConnectionsToDisconnect.front();
		this->voiceConnectionsToDisconnect.pop();
		DiscordCoreAPI::getVoiceConnectionMap()[theDCData]->disconnectInternal();
	}

	void BaseSocketAgent::internalConnect() noexcept {
		try {
			if (this->connections.size() > 0) {
				DiscordCoreAPI::ConnectionPackage connectData = this->connections.front();
				this->connections.pop();
				if (!this->sslShards.contains(connectData.currentShard)) {
					this->sslShards[connectData.currentShard] =
						std::make_unique<WebSocketSSLShard>(&this->connections, this->currentBaseSocketAgent, connectData.currentShard, this->configManager);
				}
				this->sslShards[connectData.currentShard]->currentReconnectTries = connectData.currentReconnectTries;
				this->sslShards[connectData.currentShard]->currentReconnectTries++;
				this->sslShards[connectData.currentShard]->voiceConnectionDataBufferMap = std::move(connectData.voiceConnectionDataBufferMap);

				if (this->sslShards[connectData.currentShard]->currentReconnectTries >= this->maxReconnectTries) {
					this->doWeQuit->store(true);
					return;
				}

				if (!this->sslShards[connectData.currentShard]->connect(this->configManager->getConnectionAddress(), this->configManager->getConnectionPort())) {
					this->sslShards[connectData.currentShard]->disconnect();
					return;
				}
				std::string sendString{};
				sendString = "GET /?v=10&encoding=";
				sendString += this->configManager->getTextFormat() == DiscordCoreAPI::TextFormat::Etf ? "etf" : "json";
				sendString += " HTTP/1.1\r\nHost: " + this->configManager->getConnectionAddress() +
					"\r\nPragma: no-cache\r\nUser-Agent: DiscordCoreAPI/1.0\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: " +
					DiscordCoreAPI::generateBase64EncodedKey() + "\r\nSec-WebSocket-Version: 13\r\n\r\n";
				bool didWeWrite{ false };
				DiscordCoreAPI::StopWatch theStopWatch{ 5000ms };
				do {
					if (theStopWatch.hasTimePassed()) {
						this->connections.push(connectData);
						return;
					}
					didWeWrite = this->sslShards[connectData.currentShard]->writeData(sendString, true);
				} while (!didWeWrite);
				if (!didWeWrite) {
					this->sslShards[connectData.currentShard]->disconnect();
					return;
				}

				while (!this->doWeQuit->load()) {
					if (this->sslShards[connectData.currentShard]->areWeConnected02.load()) {
						break;
					}
					WebSocketSSLShard::processIO(this->sslShards, 10000);
					if (this->sslShards[connectData.currentShard]->areWeStillConnected()) {
						this->parseHeadersAndMessage(this->sslShards[connectData.currentShard].get());
					}
					if (this->sslShards[connectData.currentShard]->areWeStillConnected()) {
						this->onMessageReceived(this->sslShards[connectData.currentShard].get());
					}
					std::this_thread::sleep_for(1ms);
				}
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("BaseSocketAgent::internalConnect()");
			}
		}
	}

	BaseSocketAgent::~BaseSocketAgent() noexcept {
		this->taskThread->request_stop();
		if (this->taskThread->joinable()) {
			this->taskThread->join();
		}
	}
}