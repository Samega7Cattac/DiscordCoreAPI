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
/// VoiceConnection.cpp - Source file for the voice connection class.
/// Jul 15, 2021
/// https://discordcoreapi.com
/// \file VoiceConnection.cpp

#include <discordcoreapi/VoiceConnection.hpp>
#include <discordcoreapi/DiscordCoreClient.hpp>

namespace DiscordCoreAPI {

	namespace Globals {
		extern std::atomic_bool doWeQuit;
	}

	RTPPacket::RTPPacket(uint32_t timestampNew, uint16_t sequenceNew, uint32_t ssrcNew, const std::vector<uint8_t>& audioDataNew, const std::string& theKeys) : theKeys(theKeys) {
		this->audioData = audioDataNew;
		this->timestamp = timestampNew;
		this->sequence = sequenceNew;
		this->ssrc = ssrcNew;
	}

	RTPPacket::operator std::string() {
		if (this->theKeys.size() > 0) {
			const uint8_t nonceSize{ crypto_secretbox_NONCEBYTES };
			const uint8_t headerSize{ 12 };
			const uint8_t byteSize{ 8 };
			std::string headerFinal{};
			storeBits(headerFinal, this->version);
			storeBits(headerFinal, this->flags);
			storeBits(headerFinal, this->sequence);
			storeBits(headerFinal, this->timestamp);
			storeBits(headerFinal, this->ssrc);
			std::unique_ptr<uint8_t[]> nonceForLibSodium{ std::make_unique<uint8_t[]>(nonceSize) };
			for (uint8_t x = 0; x < headerSize; x++) {
				nonceForLibSodium[x] = headerFinal[x];
			}
			for (uint8_t x = headerSize; x < nonceSize; x++) {
				nonceForLibSodium[x] = 0;
			}
			uint64_t numOfBytes{ headerSize + this->audioData.size() + crypto_secretbox_MACBYTES };
			std::unique_ptr<uint8_t[]> audioDataPacket{ std::make_unique<uint8_t[]>(numOfBytes) };
			for (uint8_t x = 0; x < headerSize; x++) {
				audioDataPacket[x] = headerFinal[x];
			}
			std::unique_ptr<uint8_t[]> encryptionKeys{ std::make_unique<uint8_t[]>(this->theKeys.size()) };
			for (uint64_t x = 0; x < this->theKeys.size(); x++) {
				encryptionKeys[x] = this->theKeys[x];
			}
			if (crypto_secretbox_easy(audioDataPacket.get() + headerSize, this->audioData.data(), this->audioData.size(), nonceForLibSodium.get(), encryptionKeys.get()) != 0) {
				return "";
			};
			std::string audioDataPacketNew{};
			audioDataPacketNew.insert(audioDataPacketNew.begin(), audioDataPacket.get(), audioDataPacket.get() + numOfBytes);
			return audioDataPacketNew;
		}
		return std::string{};
	}

	VoiceConnection::VoiceConnection(DiscordCoreInternal::BaseSocketAgent* BaseSocketAgentNew, const DiscordCoreInternal::VoiceConnectInitData& initDataNew,
		DiscordCoreAPI::ConfigManager* configManagerNew) noexcept {
		this->baseShard = BaseSocketAgentNew->sslShards[initDataNew.currentShard].get();
		this->activeState.store(VoiceActiveState::Connecting);
		this->baseSocketAgent = BaseSocketAgentNew;
		this->voiceConnectInitData = initDataNew;
		this->configManager = configManagerNew;
	}

	Snowflake VoiceConnection::getChannelId() noexcept {
		if (this && this->voiceConnectInitData.channelId != 0) {
			return this->voiceConnectInitData.channelId;
		} else {
			return Snowflake();
		}
	}

	void VoiceConnection::stringifyJsonData(const nlohmann::json& dataToSend, std::string& theString, DiscordCoreInternal::WebSocketOpCode theOpCode) noexcept {
		std::string theVector{};
		std::string header{};
		theVector = dataToSend.dump();
		this->createHeader(header, theVector.size(), theOpCode);
		std::string theVectorNew{};
		theVectorNew.insert(theVectorNew.begin(), header.begin(), header.end());
		theVectorNew.insert(theVectorNew.begin() + header.size(), theVector.begin(), theVector.end());
		theString = theVectorNew;
	}

	void VoiceConnection::createHeader(std::string& outBuffer, uint64_t sendLength, DiscordCoreInternal::WebSocketOpCode opCode) noexcept {
		try {
			outBuffer.push_back(static_cast<uint8_t>(opCode) | DiscordCoreInternal::webSocketFinishBit);

			int32_t indexCount{ 0 };
			if (sendLength <= DiscordCoreInternal::webSocketMaxPayloadLengthSmall) {
				outBuffer.push_back(static_cast<uint8_t>(sendLength));
				indexCount = 0;
			} else if (sendLength <= DiscordCoreInternal::webSocketMaxPayloadLengthLarge) {
				outBuffer.push_back(DiscordCoreInternal::webSocketPayloadLengthMagicLarge);
				indexCount = 2;
			} else {
				outBuffer.push_back(DiscordCoreInternal::webSocketPayloadLengthMagicHuge);
				indexCount = 8;
			}
			for (int32_t x = indexCount - 1; x >= 0; x--) {
				outBuffer.push_back(static_cast<uint8_t>(sendLength >> x * 8));
			}

			outBuffer[1] |= DiscordCoreInternal::webSocketMaskBit;
			outBuffer.push_back(0);
			outBuffer.push_back(0);
			outBuffer.push_back(0);
			outBuffer.push_back(0);
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::createHeader()");
			}
		}
	}

	void VoiceConnection::parseHeadersAndMessage(DiscordCoreInternal::WebSocketSSLShard* theShard) noexcept {
		if (theShard->theSSLState.load() == DiscordCoreInternal::SSLConnectionState::Connected) {
			if (this->connectionState.load() == VoiceConnectionState::Initializing_WebSocket) {
				std::string newVector = theShard->getInputBuffer();
				if (newVector.find("\r\n\r\n") != std::string::npos) {
					newVector.erase(0, newVector.find("\r\n\r\n") + 4);
					theShard->inputBuffer.clear();
					theShard->inputBuffer.insert(theShard->inputBuffer.end(), newVector.begin(), newVector.end());
					std::string finalMessage{};
					theShard->processedMessages.push(finalMessage);
				}
			}
			if (theShard->inputBuffer.size() < 4) {
				return;
			}
			theShard->dataOpCode = static_cast<DiscordCoreInternal::WebSocketOpCode>(theShard->inputBuffer[0] & ~DiscordCoreInternal::webSocketFinishBit);
			switch (theShard->dataOpCode) {
				case DiscordCoreInternal::WebSocketOpCode::Op_Continuation:
					[[fallthrough]];
				case DiscordCoreInternal::WebSocketOpCode::Op_Text:
					[[fallthrough]];
				case DiscordCoreInternal::WebSocketOpCode::Op_Binary:
					[[fallthrough]];
				case DiscordCoreInternal::WebSocketOpCode::Op_Ping:
					[[fallthrough]];
				case DiscordCoreInternal::WebSocketOpCode::Op_Pong: {
					uint8_t length01 = theShard->inputBuffer[1];
					theShard->messageOffset = 2;
					if (length01 & DiscordCoreInternal::webSocketMaskBit) {
						return;
					}
					theShard->messageLength = length01;
					if (length01 == DiscordCoreInternal::webSocketPayloadLengthMagicLarge) {
						if (theShard->inputBuffer.size() < 8) {
							return;
						}
						uint8_t length03 = theShard->inputBuffer[2];
						uint8_t length04 = theShard->inputBuffer[3];
						theShard->messageLength = static_cast<uint64_t>((length03 << 8) | length04);
						theShard->messageOffset += 2;
					} else if (length01 == DiscordCoreInternal::webSocketPayloadLengthMagicHuge) {
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
				case DiscordCoreInternal::WebSocketOpCode::Op_Close: {
					uint16_t close = theShard->inputBuffer[2] & 0xff;
					close <<= 8;
					close |= theShard->inputBuffer[3] & 0xff;
					theShard->closeCode = static_cast<DiscordCoreInternal::WebSocketCloseCode>(close);
					theShard->inputBuffer.erase(theShard->inputBuffer.begin(), theShard->inputBuffer.begin() + 4);
					if (this->configManager->doWePrintWebSocketErrorMessages()) {
						std::cout << DiscordCoreAPI::shiftToBrightRed()
								  << "Voice WebSocket " + this->sslShards[0]->shard.dump() + " Closed; Code: " << +static_cast<uint16_t>(this->sslShards[0]->closeCode)
								  << DiscordCoreAPI::reset() << std::endl
								  << std::endl;
					}
					this->onClosed();
				}
				default: {
				}
			}
		}
	}

	std::string VoiceConnection::encryptSingleAudioFrame(const EncodedFrameData& bufferToSend) noexcept {
		if (this->voiceConnectionData.secretKey.size() > 0) {
			this->sequenceIndex++;
			this->timeStamp += 960;
			return RTPPacket{ this->timeStamp, this->sequenceIndex, this->voiceConnectionData.audioSSRC, bufferToSend.data, this->voiceConnectionData.secretKey };
		}
		return std::string{};
	}

	void VoiceConnection::sendSingleAudioFrame(std::string& audioDataPacketNew) noexcept {
		if (this->datagramSocket && this->datagramSocket->areWeStillConnected()) {
			this->datagramSocket->writeData(audioDataPacketNew);
		}
	}

	UnboundedMessageBlock<AudioFrameData>& VoiceConnection::getAudioBuffer() noexcept {
		return this->audioDataBuffer;
	}

	void VoiceConnection::sendSingleFrame(const AudioFrameData& frameData) noexcept {
		this->audioDataBuffer.send(frameData);
	}

	void VoiceConnection::onMessageReceived(const std::string& theMessage) noexcept {
		try {
			if (this->sslShards.contains(0) && this->sslShards[0] && theMessage.size() > 0) {
				nlohmann::json payload = payload.parse(theMessage);
				if (this->configManager->doWePrintWebSocketSuccessMessages()) {
					std::cout << DiscordCoreAPI::shiftToBrightGreen() << "Message received from Voice WebSocket: " << theMessage << DiscordCoreAPI::reset() << std::endl
							  << std::endl;
				}
				if (payload.contains("op") && !payload["op"].is_null()) {
					switch (payload["op"].get<int32_t>()) {
						case 2: {
							this->voiceConnectionData.audioSSRC = payload["d"]["ssrc"].get<uint32_t>();
							this->voiceConnectionData.voiceIp = payload["d"]["ip"].get<std::string>();
							this->voiceConnectionData.voicePort = std::to_string(payload["d"]["port"].get<int64_t>());
							for (auto& value: payload["d"]["modes"]) {
								if (value == "xsalsa20_poly1305") {
									this->voiceConnectionData.voiceEncryptionMode = value;
								}
							}
							break;
						}
						case 4: {
							for (uint32_t x = 0; x < payload["d"]["secret_key"].size(); x++) {
								this->voiceConnectionData.secretKey.push_back(payload["d"]["secret_key"][x].get<uint8_t>());
							}
							break;
						}
						case 6: {
							this->sslShards[0]->haveWeReceivedHeartbeatAck = true;
							break;
						}
						case 8: {
							if (payload["d"].contains("heartbeat_interval")) {
								this->heartbeatInterval = static_cast<int32_t>(payload["d"]["heartbeat_interval"].get<float>());
								this->sslShards[0]->areWeHeartBeating = false;
							}
							break;
						}
					}
				}
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::onMessageReceived()");
			}
			this->onClosed();
		}
	}

	void VoiceConnection::sendVoiceData(const std::string& responseData) noexcept {
		try {
			if (responseData.size() == 0) {
				if (this->configManager->doWePrintWebSocketErrorMessages()) {
					std::cout << DiscordCoreAPI::shiftToBrightRed() << "Please specify voice data to send" << DiscordCoreAPI::reset() << std::endl << std::endl;
				}
				return;
			} else {
				if (this->datagramSocket && this->datagramSocket->areWeStillConnected()) {
					std::string theData = responseData;
					this->datagramSocket->writeData(theData);
				}
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::sendVoiceData()");
			}
			this->onClosed();
		}
	}

	bool VoiceConnection::sendMessage(const std::string& dataToSend, bool priority) noexcept {
		try {
			if (this->sslShards.contains(0) && this->sslShards[0]->areWeStillConnected()) {
				if (this->configManager->doWePrintWebSocketSuccessMessages()) {
					std::cout << DiscordCoreAPI::shiftToBrightBlue() << "Sending Voice WebSocket Message: " << dataToSend << DiscordCoreAPI::reset() << std::endl << std::endl;
				}
				DiscordCoreAPI::StopWatch theStopWatch{ 5000ms };
				bool didWeWrite{ false };
				do {
					if (theStopWatch.hasTimePassed()) {
						return false;
					}
					didWeWrite = this->sslShards[0]->writeData(dataToSend, priority);
				} while (!didWeWrite);
				return true;
			} else {
				return false;
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::sendMessage()");
			}
			this->onClosed();
			return false;
		}
	}

	void VoiceConnection::sendSpeakingMessage(const bool isSpeaking) noexcept {
		if (!isSpeaking) {
			this->sendSilence();
		} else {
			DiscordCoreInternal::SendSpeakingData theData{};
			theData.delay = 0;
			theData.ssrc = this->voiceConnectionData.audioSSRC;
			nlohmann::json newString = theData;
			std::string theString{};
			this->stringifyJsonData(newString, theString, DiscordCoreInternal::WebSocketOpCode::Op_Text);
			if (!this->sendMessage(theString, true)) {
				this->onClosed();
			}
		}
	}

	void VoiceConnection::runWebSocket(std::stop_token stopToken) noexcept {
		try {
			while (!stopToken.stop_requested() && !Globals::doWeQuit.load() && this->activeState.load() != VoiceActiveState::Exiting) {
				if (!stopToken.stop_requested() && this->connections.size() > 0) {
					DiscordCoreAPI::StopWatch theStopWatch{ 10000ms };
					VoiceActiveState theState{};
					if (this->activeState.load() == VoiceActiveState::Connecting) {
						theState = VoiceActiveState::Stopped;
					} else {
						theState = this->activeState.load();
					}
					this->activeState.store(VoiceActiveState::Connecting);
					this->connectionState.store(VoiceConnectionState::Collecting_Init_Data);
					while (!stopToken.stop_requested() && this->baseShard->theWebSocketState.load() != DiscordCoreInternal::WebSocketSSLShardState::Authenticated) {
						if (theStopWatch.hasTimePassed() || this->activeState.load() == VoiceActiveState::Exiting) {
							return;
						}
						std::this_thread::sleep_for(1ms);
					}
					this->connectInternal();
					this->sendSpeakingMessage(false);
					this->sendSpeakingMessage(true);
					this->activeState.store(theState);
				}
				if (!stopToken.stop_requested() && this->heartbeatInterval != 0 && !this->sslShards[0]->areWeHeartBeating) {
					this->sslShards[0]->areWeHeartBeating = true;
					this->sslShards[0]->heartBeatStopWatch = DiscordCoreAPI::StopWatch{ std::chrono::milliseconds{ this->heartbeatInterval } };
				}
				if (!stopToken.stop_requested() && this->sslShards[0]->areWeStillConnected() && this->sslShards[0]->heartBeatStopWatch.hasTimePassed() &&
					this->sslShards[0]->areWeHeartBeating) {
					this->sendHeartBeat();
					this->sslShards[0]->heartBeatStopWatch.resetTimer();
				}
				if (!stopToken.stop_requested() && this->sslShards[0]->areWeStillConnected()) {
					DiscordCoreInternal::WebSocketSSLShard::processIO(this->sslShards, 10000);
				}
				if (!stopToken.stop_requested() && this->sslShards[0]->areWeStillConnected()) {
					this->parseHeadersAndMessage(this->sslShards[0].get());
				}
				if (!stopToken.stop_requested() && this->sslShards[0]->areWeStillConnected() && this->sslShards[0]->processedMessages.size() > 0) {
					std::string theMessage = this->sslShards[0]->processedMessages.front();
					if (!stopToken.stop_requested() && theMessage.size() > 0) {
						this->onMessageReceived(theMessage);
					}
					this->sslShards[0]->processedMessages.pop();
				}
				std::this_thread::sleep_for(1ms);
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::run()");
			}
			this->onClosed();
		}
	}

	bool VoiceConnection::collectAndProcessAMessage() noexcept {
		DiscordCoreAPI::StopWatch theStopWatch{ 3500ms };
		while (!Globals::doWeQuit.load()) {
			DiscordCoreInternal::WebSocketSSLShard::processIO(this->sslShards, 100000);
			if (!sslShards[0]->areWeStillConnected()) {
				return false;
			}
			if (this->sslShards[0]->inputBuffer.size() > 0) {
				this->parseHeadersAndMessage(this->sslShards[0].get());
			}
			if (this->sslShards[0]->processedMessages.size() > 0) {
				std::string theMessage = this->sslShards[0]->processedMessages.front();
				this->sslShards[0]->processedMessages.pop();
				this->onMessageReceived(theMessage);
				return true;
			}
			if (theStopWatch.hasTimePassed()) {
				this->onClosed();
				return false;
			}
			std::this_thread::sleep_for(1ms);
		}
		return false;
	}

	void VoiceConnection::runVoice(std::stop_token stopToken) noexcept {
		StopWatch theStopWatch{ 20000ms };
		while (!this->datagramSocket) {
			if (theStopWatch.hasTimePassed() || this->activeState.load() == VoiceActiveState::Exiting) {
				return;
			}
			std::this_thread::sleep_for(1ms);
		}
		while (!stopToken.stop_requested() && !Globals::doWeQuit.load() && this->activeState.load() != VoiceActiveState::Exiting) {
			if (!this->datagramSocket->areWeStillConnected()) {
				this->onClosed();
			}
			switch (this->activeState.load()) {
				case VoiceActiveState::Connecting: {
					while (!stopToken.stop_requested() && this->activeState.load() == VoiceActiveState::Connecting) {
						std::this_thread::sleep_for(1ms);
					}
					break;
				}
				case VoiceActiveState::Stopped: {
					this->audioDataBuffer.clearContents();
					this->clearAudioData();
					while (!stopToken.stop_requested() && this->activeState.load() == VoiceActiveState::Stopped) {
						std::this_thread::sleep_for(1ms);
					}
					break;
				}
				case VoiceActiveState::Paused: {
					while (!stopToken.stop_requested() && this->activeState.load() == VoiceActiveState::Paused) {
						std::this_thread::sleep_for(1ms);
					}
					break;
				}
				case VoiceActiveState::Playing: {
					this->audioData.type = AudioFrameType::Unset;
					this->audioData.encodedFrameData.data.clear();
					this->audioData.rawFrameData.data.clear();
					this->audioDataBuffer.tryReceive(this->audioData);
					DoubleTimePointNs startingValue{ std::chrono::system_clock::now().time_since_epoch() };
					DoubleTimePointNs intervalCount{ std::chrono::nanoseconds{ 20000000 } };
					DoubleTimePointNs targetTime{ startingValue.time_since_epoch() + intervalCount.time_since_epoch() };
					int32_t frameCounter{ 0 };
					DoubleTimePointNs totalTime{ std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::nanoseconds{ 0 }) };
					if (this->disconnectStartTime.count() != 0) {
						DoubleMilliSecond currentTime = std::chrono::system_clock::now().time_since_epoch();
						if ((currentTime - this->disconnectStartTime).count() >= 60000) {
							this->connect();
							this->play();
						}
					}
					while (!stopToken.stop_requested() && this->activeState.load() == VoiceActiveState::Playing) {
						theStopWatch.resetTimer();
						while (!stopToken.stop_requested() && !this->datagramSocket->areWeStillConnected()) {
							if (theStopWatch.hasTimePassed() || this->activeState.load() == VoiceActiveState::Exiting) {
								return;
							}
							std::this_thread::sleep_for(1ms);
						}
						frameCounter++;
						if (this->audioData.guildMemberId != 0) {
							this->currentGuildMemberId = this->audioData.guildMemberId;
						}
						if (this->audioData.type != AudioFrameType::Unset && this->audioData.type != AudioFrameType::Skip) {
							std::string newFrame{};
							if (this->audioData.type == AudioFrameType::RawPCM) {
								std::vector<RawFrameData> rawFrames{};
								rawFrames.push_back(this->audioData.rawFrameData);
								auto encodedFrameData = this->encoder->encodeFrames(rawFrames);
								newFrame =
									this->encryptSingleAudioFrame(encodedFrameData[0].encodedFrameData);
							} else {
								newFrame =
									this->encryptSingleAudioFrame(this->audioData.encodedFrameData);
							}
							if (newFrame.size() == 0) {
								continue;
							}
							auto waitTime = targetTime - std::chrono::system_clock::now();
							if (waitTime.count() > 500000 && !stopToken.stop_requested() && this->datagramSocket->areWeStillConnected()) {
								this->datagramSocket->readData();
								this->datagramSocket->getInputBuffer();
							}
							waitTime = targetTime - std::chrono::system_clock::now();
							nanoSleep(static_cast<int64_t>(static_cast<float>(waitTime.count()) * 0.95f));
							waitTime = targetTime - std::chrono::system_clock::now();
							if (waitTime.count() > 0) {
								spinLock(waitTime.count());
							}
							startingValue = std::chrono::system_clock::now();
							this->sendSingleAudioFrame(newFrame);
							totalTime += std::chrono::system_clock::now() - startingValue;
							auto intervalCountNew =
								DoubleTimePointNs{ std::chrono::nanoseconds{ 20000000 } - totalTime.time_since_epoch() / frameCounter }.time_since_epoch().count();
							intervalCount = DoubleTimePointNs{ std::chrono::nanoseconds{ static_cast<uint64_t>(intervalCountNew) } };
							targetTime = std::chrono::system_clock::now().time_since_epoch() + intervalCount;
							this->audioData.type = AudioFrameType::Unset;
							this->audioData.encodedFrameData.data.clear();
							this->audioData.rawFrameData.data.clear();
						} else if (this->audioData.type == AudioFrameType::Skip) {
							SongCompletionEventData completionEventData{};
							completionEventData.guild = Guilds::getCachedGuildAsync({ .guildId = this->voiceConnectInitData.guildId }).get();
							completionEventData.guildMember =
								GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = this->currentGuildMemberId, .guildId = this->voiceConnectInitData.guildId }).get();
							completionEventData.wasItAFail = false;
							getSongAPIMap()[this->voiceConnectInitData.guildId]->onSongCompletionEvent(completionEventData);
							break;
						} else {
							break;
						}
						this->audioDataBuffer.tryReceive(this->audioData);
					}
					this->disconnectStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
					break;
				}
				case VoiceActiveState::Exiting: {
					return;
				}
				default: {
					break;
				}
			}
			if (stopToken.stop_requested()) {
				return;
			}
		}
	};

	bool VoiceConnection::areWeCurrentlyPlaying() noexcept {
		if (this == nullptr) {
			return false;
		} else {
			bool areWePlaying{};
			if (this->activeState.load() == VoiceActiveState::Playing || this->activeState.load() == VoiceActiveState::Paused) {
				return true;
			} else {
				return false;
			}
		}
	}

	void VoiceConnection::disconnectInternal() noexcept {
		if (this->taskThread01) {
			this->taskThread01->request_stop();
			if (this->taskThread01->joinable()) {
				this->taskThread01->join();
			}
			this->taskThread01.reset(nullptr);
		}
		if (this->taskThread02) {
			this->taskThread02->request_stop();
			if (this->taskThread02->joinable()) {
				this->taskThread02->join();
			}
			this->taskThread02.reset(nullptr);
		}
		if (this->datagramSocket) {
			this->datagramSocket->disconnect();
		}
		if (this->sslShards[0]) {
			this->sslShards[0]->disconnect(false);
		}
		auto thePtr = getSongAPIMap()[this->voiceConnectInitData.guildId].get();
		if (thePtr) {
			thePtr->onSongCompletionEvent.remove(thePtr->eventToken);
		}
		this->areWeConnectedBool.store(false);
		this->activeState.store(VoiceActiveState::Connecting);
	}

	void VoiceConnection::collectExternalIP() noexcept {
		try {
			std::string packet{};
			packet.resize(74);
			uint16_t val1601{ 0x01 };
			uint16_t val1602{ 70 };
			packet[0] = static_cast<uint8_t>(val1601 >> 8);
			packet[1] = static_cast<uint8_t>(val1601 >> 0);
			packet[2] = static_cast<uint8_t>(val1602 >> 8);
			packet[3] = static_cast<uint8_t>(val1602 >> 0);
			packet[4] = static_cast<uint8_t>(this->voiceConnectionData.audioSSRC >> 24);
			packet[5] = static_cast<uint8_t>(this->voiceConnectionData.audioSSRC >> 16);
			packet[6] = static_cast<uint8_t>(this->voiceConnectionData.audioSSRC >> 8);
			packet[7] = static_cast<uint8_t>(this->voiceConnectionData.audioSSRC);
			this->datagramSocket->writeData(packet);
			std::string inputString{};
			StopWatch theStopWatch{ 3500ms };
			while (inputString.size() < 74 && !Globals::doWeQuit.load() && this->activeState.load() != VoiceActiveState::Exiting) {
				this->datagramSocket->readData();
				std::string theNewString = this->datagramSocket->getInputBuffer();
				inputString.insert(inputString.end(), theNewString.begin(), theNewString.end());
				std::this_thread::sleep_for(1ms);
				if (theStopWatch.hasTimePassed()) {
					break;
				}
			}
			std::string message{};
			message.insert(message.begin(), inputString.begin() + 8, inputString.begin() + 64);
			if (message.find('\u0000') != std::string::npos) {
				message = message.substr(0, message.find('\u0000', 5));
			}
			this->voiceConnectionData.externalIp = message;
			this->areWeConnectedBool.store(true);
			this->voiceConnectionDataBuffer.clearContents();
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::collectExternalIP()");
			}
			this->onClosed();
		}
	}
	void VoiceConnection::connectInternal() noexcept {

		std::lock_guard theLock{ this->baseSocketAgent->sslShards[voiceConnectInitData.currentShard]->theMutex02 };
		if (this->connections.size() > 0) {
			this->connections.pop();
		}
		if (this->currentReconnectTries >= this->maxReconnectTries) {
			Globals::doWeQuit.store(true);
			std::cout << "VoiceConnection::connectInternal() Error: Failed to connect to voice channel!" << std::endl << std::endl;
		}
		switch (this->connectionState.load()) {
			case VoiceConnectionState::Collecting_Init_Data: {
				this->voiceConnectionData = DiscordCoreInternal::VoiceConnectionData{};
				this->baseShard->voiceConnectionDataBufferMap[this->voiceConnectInitData.guildId] = &this->voiceConnectionDataBuffer;
				this->baseShard->voiceConnectionDataBufferMap[this->voiceConnectInitData.guildId]->clearContents();
				this->baseSocketAgent->getVoiceConnectionData(this->voiceConnectInitData, this->baseShard);

				if (DiscordCoreAPI::waitForTimeToPass(this->voiceConnectionDataBuffer, this->voiceConnectionData, 20000)) {
					this->onClosed();
					return;
				}
				this->baseUrl = this->voiceConnectionData.endPoint.substr(0, this->voiceConnectionData.endPoint.find(":"));
				this->connectionState.store(VoiceConnectionState::Initializing_WebSocket);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Initializing_WebSocket: {
				if (this->sslShards.contains(0)) {
					this->sslShards[0]->disconnect(true);
				}
				auto theClient = std::make_unique<DiscordCoreInternal::WebSocketSSLShard>(&this->connections, 0, 0, this->configManager);
				if (!theClient->connect(this->baseUrl, "443")) {
					this->sslShards[0] = std::move(theClient);
					this->onClosed();
					return;
				}
				std::string sendVector = "GET /?v=4 HTTP/1.1\r\nHost: " + this->baseUrl +
					"\r\nPragma: no-cache\r\nUser-Agent: DiscordCoreAPI/1.0\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: " +
					DiscordCoreAPI::generateBase64EncodedKey() + "\r\nSec-WebSocket-Version: 13\r\n\r\n";
				this->sslShards[0] = std::move(theClient);
				this->sslShards[0]->shard[0] = 0;
				this->sslShards[0]->shard[1] = 1;
				if (!this->sendMessage(sendVector, true)) {
					this->onClosed();
					return;
				}
				if (!this->collectAndProcessAMessage()) {
					this->onClosed();
					return;
				}
				this->connectionState.store(VoiceConnectionState::Collecting_Hello);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Collecting_Hello: {
				if (!this->collectAndProcessAMessage()) {
					this->onClosed();
					return;
				}
				this->currentReconnectTries = 0;
				this->connectionState.store(VoiceConnectionState::Sending_Identify);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Sending_Identify: {
				this->sslShards[0]->haveWeReceivedHeartbeatAck = true;
				DiscordCoreInternal::VoiceIdentifyData identifyData{};
				identifyData.connectInitData = this->voiceConnectInitData;
				identifyData.connectionData = this->voiceConnectionData;
				std::string sendVector{};
				this->stringifyJsonData(identifyData, sendVector, DiscordCoreInternal::WebSocketOpCode::Op_Text);
				if (!this->sendMessage(sendVector, true)) {
					this->onClosed();
					return;
				}
				DiscordCoreInternal::WebSocketSSLShard::processIO(this->sslShards, 100000);
				this->connectionState.store(VoiceConnectionState::Collecting_Ready);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Collecting_Ready: {
				if (!this->collectAndProcessAMessage()) {
					this->onClosed();
					return;
				}
				this->connectionState.store(VoiceConnectionState::Initializing_DatagramSocket);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Initializing_DatagramSocket: {
				this->voiceConnect();
				this->connectionState.store(VoiceConnectionState::Collecting_External_Ip);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Collecting_External_Ip: {
				this->collectExternalIP();
				this->connectionState.store(VoiceConnectionState::Sending_Select_Protocol);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Sending_Select_Protocol: {
				DiscordCoreInternal::VoiceSocketProtocolPayloadData protocolPayloadData{};
				protocolPayloadData.externalIp = this->voiceConnectionData.externalIp;
				protocolPayloadData.voiceEncryptionMode = this->voiceConnectionData.voiceEncryptionMode;
				protocolPayloadData.voicePort = this->voiceConnectionData.voicePort;
				nlohmann::json protocolPayloadSelectString = protocolPayloadData;
				std::string sendVector{};
				this->stringifyJsonData(protocolPayloadSelectString, sendVector, DiscordCoreInternal::WebSocketOpCode::Op_Text);
				if (!this->sendMessage(sendVector, true)) {
					this->onClosed();
					return;
				}
				this->connectionState.store(VoiceConnectionState::Collecting_Session_Description);
				this->connectInternal();
				break;
			}
			case VoiceConnectionState::Collecting_Session_Description: {
				if (!this->collectAndProcessAMessage()) {
					this->onClosed();
					return;
				}
				this->baseShard->voiceConnectionDataBufferMap[this->voiceConnectInitData.guildId]->clearContents();
				return;
			}
		}
	}

	void VoiceConnection::clearAudioData() noexcept {
		if (this->audioData.encodedFrameData.data.size() != 0 && this->audioData.rawFrameData.data.size() != 0) {
			this->audioData.encodedFrameData.data.clear();
			this->audioData.rawFrameData.data.clear();
			this->audioData = AudioFrameData();
		}
		AudioFrameData frameData{};
		while (this->audioDataBuffer.tryReceive(frameData)) {
		};
	}

	bool VoiceConnection::areWeConnected() noexcept {
		if (this == nullptr) {
			return false;
		} else {
			return this->areWeConnectedBool.load();
		}
	}

	void VoiceConnection::sendHeartBeat() noexcept {
		try {
			if (this->sslShards[0]->areWeStillConnected() && this->sslShards[0]->haveWeReceivedHeartbeatAck) {
				nlohmann::json data{};
				data["d"] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				data["op"] = int32_t(3);
				std::string theString{};
				this->stringifyJsonData(data, theString, DiscordCoreInternal::WebSocketOpCode::Op_Text);
				if (!this->sendMessage(theString, true)) {
					this->onClosed();
				}
				this->sslShards[0]->haveWeReceivedHeartbeatAck = false;
			} else {
				this->onClosed();
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::sendHeartBeat()");
			}
			this->onClosed();
		}
	}

	void VoiceConnection::voiceConnect() noexcept {
		try {
			if (!this->datagramSocket) {
				this->datagramSocket = std::make_unique<DiscordCoreInternal::DatagramSocketSSLClient>();
			}
			if (!this->datagramSocket->connect(this->voiceConnectionData.voiceIp, this->voiceConnectionData.voicePort)) {
				this->onClosed();
			}
		} catch (...) {
			if (this->configManager->doWePrintWebSocketErrorMessages()) {
				DiscordCoreAPI::reportException("VoiceConnection::voiceConnect()");
			}
			this->onClosed();
		}
	}

	void VoiceConnection::sendSilence() noexcept {
		EncodedFrameData newFrame{};
		newFrame.data.push_back(0xf8);
		newFrame.data.push_back(0xff);
		newFrame.data.push_back(0xfe);
		auto theFrame = this->encryptSingleAudioFrame(newFrame);
		this->sendSingleAudioFrame(theFrame);
	}

	void VoiceConnection::pauseToggle() noexcept {
		if (this) {
			if (this->activeState.load() == VoiceActiveState::Paused) {
				sendSpeakingMessage(true);
				this->activeState.store(VoiceActiveState::Playing);
			} else {
				sendSpeakingMessage(false);
				this->activeState.store(VoiceActiveState::Paused);
			}
		}
	}
	
	void VoiceConnection::disconnect() noexcept {
		std::lock_guard theLock{ this->baseSocketAgent->theMutex };
		this->baseSocketAgent->voiceConnectionsToDisconnect.push(this->voiceConnectInitData.guildId);
		this->activeState.store(VoiceActiveState::Exiting);
	}

	void VoiceConnection::reconnect() noexcept {
		if (this->datagramSocket) {
			this->datagramSocket->disconnect();
		}
		if (this->sslShards[0]) {
			this->sslShards[0]->disconnect(true);
			this->sslShards[0]->areWeHeartBeating = false;
			this->sslShards[0]->wantWrite = true;
			this->sslShards[0]->wantRead = false;
		}
		this->currentReconnectTries++;
		this->areWeConnectedBool.store(false);
	}

	void VoiceConnection::onClosed() noexcept {
		this->connectionState.store(VoiceConnectionState::Collecting_Init_Data);
		if (this->activeState.load() != VoiceActiveState::Exiting && this->currentReconnectTries < this->maxReconnectTries) {
			this->reconnect();
		}
		else if (this->currentReconnectTries >= this->maxReconnectTries) {
			this->disconnect();
		}
	}

	void VoiceConnection::connect() noexcept {
		if (this->baseSocketAgent->sslShards.contains(this->voiceConnectInitData.currentShard) && this->baseSocketAgent->sslShards[voiceConnectInitData.currentShard]) {
			ConnectionPackage dataPackage{};
			dataPackage.currentShard = 0;
			this->connections.push(dataPackage);
			this->activeState.store(VoiceActiveState::Connecting);
			if (!this->taskThread01) {
				this->taskThread01 = std::make_unique<std::jthread>([=, this](std::stop_token stopToken) {
					this->runWebSocket(stopToken);
				});
			}

			if (!this->taskThread02) {
				this->taskThread02 = std::make_unique<std::jthread>([=, this](std::stop_token stopToken) {
					this->runVoice(stopToken);
				});
			}
		}
	}

	bool VoiceConnection::stop() noexcept {
		this->sendSpeakingMessage(false);
		this->activeState.store(VoiceActiveState::Stopped);
		return true;
	}

	bool VoiceConnection::play() noexcept {
		this->sendSpeakingMessage(true);
		this->activeState.store(VoiceActiveState::Playing);
		return true;
	}

	VoiceConnection::~VoiceConnection() noexcept {
		if (this->taskThread02) {
			this->taskThread02->request_stop();
			if (this->taskThread02->joinable()) {
				this->taskThread02->join();
				this->taskThread02.reset(nullptr);
			}
		}

		if (this->taskThread01) {
			this->taskThread01->request_stop();
			if (this->taskThread01->joinable()) {
				this->taskThread01->join();
				this->taskThread01.reset(nullptr);
			}
		}
	};

}