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
/// ChannelEntities.hpp - Header for the classes related classes and structs.
/// May 13, 2021
/// https://discordcoreapi.com
/// \file ChannelEntities.hpp

#pragma once

#ifndef CHANNEL_ENTITIES
	#define CHANNEL_ENTITIES

	#include <discordcoreapi/FoundationEntities.hpp>
	#include <discordcoreapi/Https.hpp>

namespace DiscordCoreAPI {

	inline Bool operator==(const DiscordCoreAPI::ChannelData& lhs, const DiscordCoreAPI::ChannelData& rhs) {
		if (lhs.id == rhs.id) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * \addtogroup foundation_entities
	 * @{
	 */

	/// For collecting a Channel from Discord's servers. \brief For collecting a Channel from Discord's servers.
	struct DiscordCoreAPI_Dll GetChannelData {
		Snowflake channelId{};///< The id of the Channel to acquire.
	};

	/// For deleting or closing a Channel. \brief For deleting or closing a Channel.
	struct DiscordCoreAPI_Dll DeleteOrCloseChannelData {
		Snowflake channelId{};///< The id of the Channel to close/delete.
		String reason{};///< The reason for deleting/closing the Channel.
	};

	/// For editing the PermissionTypes overwrites of a given Channel, for a given Role or User. \brief For editing the PermissionTypes overwrites of a given Channel, for a given Role or User.
	struct DiscordCoreAPI_Dll EditChannelPermissionOverwritesData {
		PermissionOverwritesType type{};///< The type of overwrite.
		Snowflake roleOrUserId{};///< The Role or User Id to modify the permissions for.
		Snowflake channelId{};///< The Channel id of the desired Channel to modify the permissions for.
		String reason{};///< Reason for editing the permission overwrites.
		String allow{};///< The permissions to list as "allowed".
		String deny{};///< The permissions to list as "deny".

		operator JsonObject();
	};

	/// For collecting the invites to a given Channel. \brief For collecting the invites to a given Channel.
	struct DiscordCoreAPI_Dll GetChannelInvitesData {
		Snowflake channelId{};
	};

	/// For creating an invite to a given Channel. \brief For creating an invite to a given Channel.
	struct DiscordCoreAPI_Dll CreateChannelInviteData {
		Snowflake targetApplicationId{};///< The id of the embedded application to open for this invite, required if target_type is 2.
		InviteTargetTypes targetType{};///<	The type of target for this voice Channel invite.
		Snowflake targetUserId{};///< Id of the user whose stream to display for this invite, required if target_type is 1.
		Bool temporary{ false };///< Whether this invite only grants temporary membership.
		Snowflake channelId{};///< The id of the Channel to create the invite for.
		String reason{};///< Reason for creating the invite.
		Bool unique{ false };///< If true, don't try to reuse a similar invite (useful for creating many unique one time use invites).
		Int32 maxUses{ 0 };///< Max number of uses or 0 for unlimited.between 0 and 100.
		Int32 maxAge{ 0 };///< Duration of invite in seconds before expiry, or 0 for never.between 0 and 604800 (7 days)	86400 (24 hours).

		operator JsonObject();
	};

	/// For deleting the PermissionTypes overwrites of a given Channel for a given Role or User. \brief For deleting the PermissionTypes overwrites of a given Channel for a given Role or User.
	struct DiscordCoreAPI_Dll DeleteChannelPermissionOverwritesData {
		Snowflake roleOrUserId{};///< The Role or User id for whom to delete the PermissionTypes overwrites.
		Snowflake channelId{};///< The id of the Channel to modify permissions for.
		String reason{};///< Reason for deleting these permission overwrites.
	};

	/// For following a news Channel. \brief For following a news Channel.
	struct DiscordCoreAPI_Dll FollowNewsChannelData {
		Snowflake targetChannelId{};
		Snowflake channelId{};

		operator JsonObject();
	};

	/// For triggering the typing indicator in a given Channel. \brief For triggering the typing indicator in a given Channel.
	struct DiscordCoreAPI_Dll TriggerTypingIndicatorData {
		Snowflake channelId{};
	};

	/// For acquiring a list of Channels from a chosen Guild. \brief For acquiring a list of Channels from a chosen Guild.
	struct DiscordCoreAPI_Dll GetGuildChannelsData {
		Snowflake guildId{};///< The Guild from which to collect the Channels from.
	};

	/// For creating a new Channel within a chosen Guild. \brief For creating a new Channel within a chosen Guild.
	struct DiscordCoreAPI_Dll CreateGuildChannelData {
		Vector<OverWriteData> permissionOverwrites{};///< Array of overwrite objects	the Channel's permission overwrites.
		Int32 defaultAutoArchiveDuration{ 0 };///< Defualt duration of time before archiving a thread.
		Int32 rateLimitPerUser{ 0 };///< Amount of seconds a user has to wait before sending another message(0 - 21600).
		Int32 userLimit{ 0 };///< The user limit of the voice Channel(voice only).
		Int32 position{ 0 };///< Sorting position of the Channel.
		Int32 bitrate{ 0 };///< The bitrate(in bits) of the voice Channel(voice only).
		String reason{};///< Reason for creating the Channel.
		Snowflake parentId{};///< Id of the parent category for a Channel.
		String topic{};///< Channel topic(0 - 1024 characters).
		String name{};///< The name of the Channel.
		ChannelType type{};///< The type of Channel.
		Bool nsfw{ false };///<  Whether the Channel is nsfw.
		Snowflake guildId{};///< The Guild within which to create the Channel.

		operator JsonObject();
	};

	/// For modifying the Channel position responseData of a single Channel. \brief For modifying the Channel position responseData of a single Channel.
	struct DiscordCoreAPI_Dll ModifyGuildChannelPositionData {
		Bool lockPermissions{ false };///< Syncs the permission overwrites with the new parent, if moving to a new category.
		Int32 position{ 0 };///< Sorting position of the Channel.
		Snowflake parentId{};///< The new parent ID for the Channel that is moved.
		Snowflake id{};///< Channel id.
	};

	/// For modifying the current positions of one or more Channels in the Guild. \brief For modifying the current positions of one or more Channels in the Guild.
	struct DiscordCoreAPI_Dll ModifyGuildChannelPositionsData {
		Vector<ModifyGuildChannelPositionData> modifyChannelData{};///< Array of new Channel position's responseData.
		String reason{};///< Reason for re-ordering the Channel positions.
		Snowflake guildId{};///< Guild within which to re-order the Channel positions.

		operator JsonObject();
	};

	/// For collecting a direct-messaging Channel. \brief For collecting a direct-messaging Channel.
	struct DiscordCoreAPI_Dll CreateDMChannelData {
		Snowflake userId{};///< The User for whom to collect the direct-messaging Channel to.
	};

	/// A Channel object. \brief A Channel object.
	class DiscordCoreAPI_Dll Channel : public ChannelData {
	  public:
		TimeStamp<std::chrono::milliseconds> lastPinTimestamp{};///< Timestamp of the last pinned Message.
		UMap<Uint64, UserData> recipients{};///< Recipients, in the case of a group Dm or m.
		Int32 defaultAutoArchiveDuration{ 0 };///< Default time it takes to archive a thread.
		ThreadMetadataData threadMetadata{};///< Metadata in the case that this Channel is a Thread.
		Int32 videoQualityMode{ 0 };///< Video quality mode.
		Int32 rateLimitPerUser{ 0 };///< Amount of seconds a User has to wait before sending another Message.
		StringWrapper lastMessageId{};///< Id of the last Message.
		StringWrapper permissions{};///< Computed permissions for the invoking user in the channel, including overwrites.
		ThreadMemberData member{};///< Thread member object for the current User, if they have joined the Thread.
		Int32 messageCount{ 0 };///< An approximate count of Messages in a Thread stops counting at 50.
		Snowflake applicationId{};///< Application id of the current application.
		StringWrapper rtcRegion{};///< Real-time clock region.
		Int32 userLimit{ 0 };///< User limit, in the case of voice channels.
		StringWrapper topic{};///< The Channel's topic.
		Int32 bitrate{ 0 };///< Bitrate of the Channel, if it is a voice Channel.
		IconHash icon{};///< Icon for the Channel, if applicable.

		Channel() noexcept = default;

		Channel& operator=(ChannelData&&) noexcept;

		Channel(ChannelData&&) noexcept;

		Channel& operator=(const ChannelData&) noexcept;

		Channel(const ChannelData&) noexcept;

		Channel(simdjson::ondemand::value jsonObjectData);

		~Channel() noexcept = default;

		String getIconUrl() noexcept;
	};

	class DiscordCoreAPI_Dll ChannelVector {
	  public:
		ChannelVector() noexcept = default;

		operator Vector<Channel>();

		ChannelVector(simdjson::ondemand::value jsonObjectData);

		virtual ~ChannelVector() noexcept = default;

	  protected:
		Vector<Channel> theChannels{};
	};

	/// For modifying a Channel's properties. \brief For modifying a Channel's properties.
	struct DiscordCoreAPI_Dll ModifyChannelData {
		UpdateChannelData channelData{};///< The responseData of the Channel to be updated.
		Snowflake channelId{};///< The id of the Channel to modify.
		String reason{};///< A reason for modifying the Channel.

		operator JsonObject();

		ModifyChannelData(Channel newData);
	};

	/**@}*/

	/**
	 * \addtogroup main_endpoints
	 * @{
	 */
	/// An interface class for the Channel related endpoints. \brief An interface class for the Channel-related endpoints.
	class DiscordCoreAPI_Dll Channels {
	  public:
		friend class DiscordCoreInternal::WebSocketSSLShard;
		friend class DiscordCoreClient;
		friend class ChannelData;
		friend class GuildData;
		friend class Guild;

		static Void initialize(DiscordCoreInternal::HttpsClient*, ConfigManager* configManager);

		/// Collects a Channel from the Discord servers. \brief Collects a Channel from the Discord servers.
		/// \param dataPackage A GetChannelData structure.
		/// \returns A CoRoutine containing a Channel.
		static CoRoutine<Channel> getChannelAsync(GetChannelData dataPackage);

		/// Collects a Channel from the library's cache. \brief Collects a Channel from the library's cache.
		/// \param dataPackage A GetChannelData structure.
		/// \returns A CoRoutine containing a Channel.
		static CoRoutine<ChannelData> getCachedChannelAsync(GetChannelData dataPackage);

		/// Modifies a Channel's properties. \brief Modifies a Channel's properties.
		/// \param dataPackage A ModifyChannelData structure.
		/// \returns A CoRoutine containing a Channel.
		static CoRoutine<Channel> modifyChannelAsync(ModifyChannelData dataPackage);

		/// Delete a Channel, or close a private message. \brief Delete a Channel, or close a private message.
		/// \param dataPackage A DeleteOrCloseChannelData structure.
		/// \returns A CoRoutine containing a Channel.
		static CoRoutine<Void> deleteOrCloseChannelAsync(DeleteOrCloseChannelData dataPackage);

		/// Edit the given Permissions overwrites for a given User or Role. \brief Edit the given Permissions overwrites for a given User or Role.
		/// \param dataPackage An EditChannelPermissionOverwritesData structure.
		/// \returns A CoRoutine containing Void.
		static CoRoutine<Void> editChannelPermissionOverwritesAsync(EditChannelPermissionOverwritesData dataPackage);

		/// Collects a vector of the invites to a given Channel. \brief Collects a vector of the invites to a given Channel.
		/// \param dataPackage A GetChannelInvitesData structure.
		/// \returns A CoRoutine containing a InviteDataVector.
		static CoRoutine<Vector<InviteData>> getChannelInvitesAsync(GetChannelInvitesData dataPackage);

		/// Creates an invite to a selected Channel. \brief Creates an invite to a selected Channel.
		/// \param dataPackage A CreateChannelInviteData structure.
		/// \returns A CoRoutine containing an InviteData.
		static CoRoutine<InviteData> createChannelInviteAsync(CreateChannelInviteData dataPackage);

		/// Delete the given Permissions overwrites for a given User or Role. \brief Delete the given Permissions overwrites for a given User or Role.
		/// \param dataPackage A DeleteChannelPermissionOverwritesData structure
		/// \returns A CoRoutine containing Void.
		static CoRoutine<Void> deleteChannelPermissionOverwritesAsync(DeleteChannelPermissionOverwritesData dataPackage);

		/// Follows a given new Channel with another Channel. \brief Follows a given new Channel with another Channel.
		/// \param dataPackage A FollowNewsChannelData structure.
		/// \returns A CoRoutine containing a Channel.
		static CoRoutine<Channel> followNewsChannelAsync(FollowNewsChannelData dataPackage);

		/// Triggers the typing indicator for the bot in the given Channel. \brief Triggers the typing indicator for the bot in the given Channel.
		/// \param dataPackage A TriggerTypingIndicatorData structure.
		/// \returns A CoRoutine containing Void.
		static CoRoutine<Void> triggerTypingIndicatorAsync(TriggerTypingIndicatorData dataPackage);

		/// Collects a list of Channels from a chosen Guild. \brief Collects a list of Channels from a chosen Guild.
		/// \param dataPackage A GetGuildChannelsData structure.
		/// \returns A CoRoutine containing a ChannelVector.
		static CoRoutine<Vector<Channel>> getGuildChannelsAsync(GetGuildChannelsData dataPackage);

		/// Creates a new Channel within a chosen Guild. \brief Creates a new Channel within a chosen Guild.
		/// \param dataPackage A CreateGuildChannelData structure.
		/// \returns A CoRoutine containing a Channel.
		static CoRoutine<Channel> createGuildChannelAsync(CreateGuildChannelData dataPackage);

		/// Re-orders the Channel positions, within a chosen Guild. \brief Re-orders the Channel positions, within a chosen Guild.
		/// \param dataPackage A ModifyGuildChannelPositionsData  structure.
		/// \returns A CoRoutine containing Void.
		static CoRoutine<Void> modifyGuildChannelPositionsAsync(ModifyGuildChannelPositionsData dataPackage);

		/// Collect a direct-Message Channel between the bot and the User. \brief Collect a direct-Message Channel between the bot and the User.
		/// \param dataPackage A CreateDMChannelData structure.
		/// \returns A CoRoutine containing a Channel.
		static CoRoutine<Channel> createDMChannelAsync(CreateDMChannelData dataPackage);

		/// Collect a list of voice regions that are usable for the RTC-Region option of a given Channel.
		/// \brief Collect a list of voice regions that are usable for the RTC-Region option of a given Channel.
		/// \returns A CoRoutine containing a VoiceRegionDataVector.
		static CoRoutine<Vector<VoiceRegionData>> getVoiceRegionsAsync();

		static Void insertChannel(ChannelData channel);

		static Void removeChannel(Snowflake channelId);

	  protected:
		static DiscordCoreInternal::HttpsClient* httpsClient;
		static ObjectCache<ChannelData> cache;
		static Bool doWeCacheChannels;
	};
	/**@}*/
}// namespace DiscordCoreAPI
#endif