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
/// StageInstanceEntities.hpp - Source file for the Stage Instance related stuff.
/// Nov 29, 2021 Chris M.
/// https://discordcoreapi.com
/// \file StageInstanceEntities.hpp

#pragma once

#include <discordcoreapi/FoundationEntities.hpp>
#include <discordcoreapi/Https.hpp>

namespace DiscordCoreAPI {

	/// \brief For creating a StageInstance.
	struct DiscordCoreAPI_Dll CreateStageInstanceData {
		StageInstancePrivacyLevel privacyLevel{};///< The privacy level of the Stage instance(default Guild_Only).
		Snowflake channelId{};///< The id of the Stage Channel.
		std::string reason{};///< The reason for starting it.
		std::string topic{};///< The topic of the Stage instance(1 - 120 characters).
	};

	/// \brief For collecting a single StageInstance.
	struct DiscordCoreAPI_Dll GetStageInstanceData {
		Snowflake channelId{};///< The Channel id from which you would like to collect the StageInstance.
	};

	/// \brief For modifying a single StageInstance.
	struct DiscordCoreAPI_Dll ModifyStageInstanceData {
		StageInstancePrivacyLevel privacyLevel{};///< The privacy level of the Stage instance.
		Snowflake channelId{};///< The Channel Id of the StageInstance.
		std::string reason{};///< Reason for modifying the StageInstance.
		std::string topic{};///< The topic of the Stage instance(1 - 120 characters).
	};

	/// \brief For deleting a single StageInstance.
	struct DiscordCoreAPI_Dll DeleteStageInstanceData {
		Snowflake channelId{};///< The Channel Id of the StageInstance.
		std::string reason{};///< Reason for deleting the StageInstance.
	};

	/// \brief A single StageInstance.
	class DiscordCoreAPI_Dll StageInstance : public StageInstanceData {
	  public:
		StageInstance() noexcept = default;

		StageInstance(simdjson::ondemand::value jsonObjectData);

		virtual ~StageInstance() noexcept = default;
	};

	/**
	 * \addtogroup main_endpoints
	 * @{
	 */
	/// \brief An interface class for the StageInstance related Discord endpoints.
	class DiscordCoreAPI_Dll StageInstances {
	  public:
		static void initialize(DiscordCoreInternal::HttpsClient*);

		/// \brief Creates a StageInstance.
		/// \param dataPackage A CreateStageInstanceData structure.
		/// \returns A CoRoutine containing a StageInstance.
		static CoRoutine<StageInstance> createStageInstanceAsync(CreateStageInstanceData dataPackage);

		/// \brief Collects a StageInstance.
		/// \param dataPackage A GetStageInstanceData structure.
		/// \returns A CoRoutine containing a StageInstance.
		static CoRoutine<StageInstance> getStageInstanceAsync(GetStageInstanceData dataPackage);

		/// \brief Modifies a StageInstance.
		/// \param dataPackage A ModifyStageInstanceData structure.
		/// \returns A CoRoutine containing a StageInstance.
		static CoRoutine<StageInstance> modifyStageInstanceAsync(ModifyStageInstanceData dataPackage);

		/// \brief Deletes a StageInstance.
		/// \param dataPackage A DeleteStageInstanceData structure.
		/// \returns A CoRoutine containing void.
		static CoRoutine<void> deleteStageInstanceAsync(DeleteStageInstanceData dataPackage);

	  protected:
		static DiscordCoreInternal::HttpsClient* httpsClient;
	};

	/**@}*/
}// namespace DiscordCoreAPI