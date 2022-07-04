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
/// VoiceConnection.hpp - Header for the voice connection class.
/// Jul 15, 2021
/// https://discordcoreapi.com
/// \file VoiceConnection.hpp

#pragma once

#include <discordcoreapi/AudioEncoder.hpp>
#include <discordcoreapi/CoRoutine.hpp>
#include <discordcoreapi/FoundationEntities.hpp>
#include <discordcoreapi/WebSocketEntities.hpp>
#include <sodium.h>

namespace DiscordCoreAPI {

	struct RTPPacket {
		std::vector<uint8_t> audioData{};
		uint8_t version{ 0x80 };
		std::string theKeys{};
		uint8_t flags{ 0x78 };
		uint32_t timestamp{};
		uint16_t sequence{};
		uint32_t ssrc{};

		RTPPacket(uint32_t timestampNew, uint16_t sequenceNew, uint32_t ssrcNew, const std::vector<uint8_t>& audioDataNew, const std::string& theKeys);

		operator std::string();
	};

	enum class VoiceConnectionState : int8_t {
		Collecting_Init_Data = 0,
		Initializing_WebSocket = 1,///< Initializing the WebSocket.
		Collecting_Hello = 2,
		Sending_Identify = 3,
		Collecting_Ready = 4,
		Initializing_DatagramSocket = 5,
		Collecting_External_Ip = 6,
		Sending_Select_Protocol = 7,
		Collecting_Session_Description = 8
	};

	enum class VoiceActiveState : int8_t {
		Idle = -1,
		Playing = 1,///< Playing.
		Stopped = 2,
		Paused = 3,
		Exiting = 4
	};

	/**
	 * \addtogroup voice_connection
	 * @{
	 */
	/// VoiceConnection class DiscordCoreAPI_Dll - represents the connection to a given voice Channel. \brief VoiceConnection class DiscordCoreAPI_Dll - represents the connection to a given voice Channel.
	class DiscordCoreAPI_Dll VoiceConnection {
	  public:
		friend class DiscordCoreInternal::SoundCloudAPI;
		friend class DiscordCoreInternal::YouTubeAPI;
		friend class DiscordCoreClient;
		friend GuildData;
		friend SongAPI;

		VoiceConnection(DiscordCoreInternal::BaseSocketAgent* BaseSocketAgentNew, const DiscordCoreInternal::VoiceConnectInitData& initDataNew,
			DiscordCoreAPI::ConfigManager* configManagerNew) noexcept;

		VoiceConnection() noexcept = default;

		/// Collects the currently connected-to voice Channel's id. \brief Collects the currently connected-to voice Channel's id.
		/// \returns uint64_t A Snowflake containing the Channel's id.
		Snowflake getChannelId() noexcept;

		~VoiceConnection() noexcept;

	  protected:
		std::unordered_map<int32_t, std::unique_ptr<DiscordCoreInternal::WebSocketSSLShard>> sslShards{};
		UnboundedMessageBlock<DiscordCoreInternal::VoiceConnectionData> voiceConnectionDataBuffer{};
		std::unique_ptr<DiscordCoreInternal::DatagramSocketSSLClient> datagramSocket{ nullptr };
		VoiceConnectionState connectionState{ VoiceConnectionState::Collecting_Init_Data };
		std::atomic<VoiceActiveState> activeState{ VoiceActiveState::Stopped };
		std::unique_ptr<DiscordCoreInternal::AudioEncoder> encoder{ nullptr };
		DiscordCoreInternal::BaseSocketAgent* baseSocketAgent{ nullptr };
		DiscordCoreInternal::VoiceConnectInitData voiceConnectInitData{};
		DiscordCoreInternal::VoiceConnectionData voiceConnectionData{};
		DiscordCoreInternal::WebSocketSSLShard* baseShard{ nullptr };
		DiscordCoreAPI::ConfigManager* configManager{ nullptr };
		UnboundedMessageBlock<AudioFrameData> audioDataBuffer{};
		std::unique_ptr<std::jthread> taskThread01{ nullptr };
		std::unique_ptr<std::jthread> taskThread02{ nullptr };
		std::atomic_bool areWeConnectedBool{ false };
		std::queue<ConnectionPackage> connections{};
		const int64_t maxReconnectTries{ 10 };
		int64_t currentReconnectTries{ 0 };
		Snowflake currentGuildMemberId{};
		int64_t disconnectStartTime{ 0 };
		int64_t heartbeatInterval{ 0 };
		uint16_t sequenceIndex{ 0 };
		AudioFrameData audioData{};
		uint32_t timeStamp{ 0 };
		std::string baseUrl{};

		void stringifyJsonData(const nlohmann::json& dataToSend, std::string& theString, DiscordCoreInternal::WebSocketOpCode theOpCode) noexcept;

		std::string encryptSingleAudioFrame(const EncodedFrameData& bufferToSend, uint32_t audioSSRC, const std::string& keys) noexcept;

		void createHeader(std::string& outbuf, uint64_t sendlength, DiscordCoreInternal::WebSocketOpCode opCode) noexcept;

		void parseHeadersAndMessage(DiscordCoreInternal::WebSocketSSLShard* theShard) noexcept;

		void sendSingleAudioFrame(std::string& audioDataPacketNew) noexcept;

		UnboundedMessageBlock<AudioFrameData>& getAudioBuffer() noexcept;

		void sendSingleFrame(const AudioFrameData& frameData) noexcept;

		void onMessageReceived(const std::string& theMessage) noexcept;

		void sendVoiceData(const std::string& responseData) noexcept;

		void sendMessage(const std::string& dataToSend) noexcept;

		void sendSpeakingMessage(const bool isSpeaking) noexcept;

		void runWebSocket(std::stop_token) noexcept;

		bool collectAndProcessAMessage() noexcept;

		void runVoice(std::stop_token) noexcept;

		bool areWeCurrentlyPlaying() noexcept;

		void collectExternalIP() noexcept;

		void connectInternal() noexcept;

		void clearAudioData() noexcept;

		bool areWeConnected() noexcept;

		void sendHeartBeat() noexcept;

		void voiceConnect() noexcept;

		void sendSilence() noexcept;

		void pauseToggle() noexcept;

		void disconnect() noexcept;

		void onClosed() noexcept;

		void connect() noexcept;

		bool stop() noexcept;

		bool play() noexcept;
	};
	/**@}*/

};// namespace DiscordCoreAPI
