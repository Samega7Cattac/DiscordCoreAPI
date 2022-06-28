/*
*
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
/// AutoModerationEntities.hpp - Header for the Auto-Moderation related classes and structs.
/// Jun 17, 2022 Chris M.
/// https://discordcoreapi.com
/// \file AutoModerationEntities.hpp

#pragma once

#include <discordcoreapi/FoundationEntities.hpp>
#include <discordcoreapi/CoRoutine.hpp>
#include <discordcoreapi/Https.hpp>

namespace DiscordCoreAPI {

	/**
	 * \addtogroup foundation_entities
	 * @{
	 */

	/// Represents an auto-moderation-rule. \brief Represents an auto-moderation-rule.
	struct AutoModerationRule : public AutoModerationRuleData {
		AutoModerationRule() = default;

		AutoModerationRule& operator=(const nlohmann::json& jsonObjectData) {
			this->parseObjectReal(jsonObjectData, this);
			return *this;
		}

		AutoModerationRule(const nlohmann::json& jsonObjectData) {
			*this = jsonObjectData;
		}

		virtual ~AutoModerationRule() = default;

		inline void parseObjectReal(const nlohmann::json& jsonObjectData, AutoModerationRule* pDataStructure) {
			if (jsonObjectData.contains("name") && !jsonObjectData["name"].is_null()) {
				pDataStructure->name = jsonObjectData["name"].get<std::string>();
			}

			if (jsonObjectData.contains("id") && !jsonObjectData["id"].is_null()) {
				pDataStructure->id = stoull(jsonObjectData["id"].get<std::string>());
			}

			if (jsonObjectData.contains("enabled") && !jsonObjectData["enabled"].is_null()) {
				pDataStructure->enabled = jsonObjectData["enabled"].get<bool>();
			}

			if (jsonObjectData.contains("trigger_type") && !jsonObjectData["trigger_type"].is_null()) {
				pDataStructure->triggerType = static_cast<TriggerType>(jsonObjectData["trigger_type"].get<uint64_t>());
			}

			if (jsonObjectData.contains("event_type") && !jsonObjectData["event_type"].is_null()) {
				pDataStructure->eventType = static_cast<EventType>(jsonObjectData["event_type"].get<uint64_t>());
			}

			if (jsonObjectData.contains("creator_id") && !jsonObjectData["creator_id"].is_null()) {
				pDataStructure->creatorId = jsonObjectData["creator_id"].get<uint64_t>();
			}

			if (jsonObjectData.contains("actions") && !jsonObjectData["actions"].is_null()) {
				for (auto& value: jsonObjectData["actions"]) {
					ActionData newData{ value };
					pDataStructure->actions.push_back(newData);
				}
			}

			if (jsonObjectData.contains("exempt_roles") && !jsonObjectData["exempt_roles"].is_null()) {
				for (auto& value: jsonObjectData["exempt_roles"]) {
					pDataStructure->exemptRoles.push_back(value.get<uint64_t>());
				}
			}

			if (jsonObjectData.contains("trigger_metadata") && !jsonObjectData["trigger_metadata"].is_null()) {
				pDataStructure->triggerMetaData = jsonObjectData["trigger_metadata"];
			}

			if (jsonObjectData.contains("exempt_channels") && !jsonObjectData["exempt_channels"].is_null()) {
				for (auto& value: jsonObjectData["exempt_channels"]) {
					pDataStructure->exemptChannels.push_back(value.get<uint64_t>());
				}
			}

			if (jsonObjectData.contains("guild_id") && !jsonObjectData["guild_id"].is_null()) {
				pDataStructure->guildId = jsonObjectData["guild_id"].get<uint64_t>();
			}
		}
	};

	class AutoModerationRuleVector {
	  public:
		std::vector<AutoModerationRule> theAutoModerationRules{};

		AutoModerationRuleVector() = default;

		AutoModerationRuleVector& operator=(const nlohmann::json& jsonObjectData) {
			this->parseObjectReal(jsonObjectData, this);
			return *this;
		}

		AutoModerationRuleVector(const nlohmann::json& jsonObjectData) {
			*this = jsonObjectData;
		}

		virtual ~AutoModerationRuleVector() = default;

		inline void parseObjectReal(const nlohmann::json& jsonObjectData, AutoModerationRuleVector* pDataStructure) {
			pDataStructure->theAutoModerationRules.reserve(jsonObjectData.size());
			for (auto& value: jsonObjectData) {
				DiscordCoreAPI::AutoModerationRule newData{ value };
				pDataStructure->theAutoModerationRules.push_back(newData);
			}
			pDataStructure->theAutoModerationRules.shrink_to_fit();
		}
	};

	/// For listing all of the auto-moderation-rules for a particular AutoModerationRule. \brief For listing all of the auto-moderation-rules for a particular AutoModerationRule.
	struct ListAutoModerationRulesForGuildData {
		uint64_t guildId{};///< The id of the guild for which you would like to list the auto-moderation rules.
	};

	/// For collecting an auto-moderation-rule for a particular AutoModerationRule. \brief For collecting an auto-moderation-rule for a particular AutoModerationRule.
	struct GetAutoModerationRuleData {
		uint64_t autoModerationRuleId{};///< The id of the auto-moderation-rule you would like to collect.
		uint64_t guildId{};///< The id of the AutoModerationRule from which you would like to collect the auto-moderation-rule from.
	};

	/// For creating an auto-moderation-rule. \brief For creating an auto-moderation-rule.
	struct CreateAutoModerationRuleData {
		std::vector<uint64_t> exemptChannels{};///< The channel ids that should not be affected by the rule(Maximum of 50).
		std::vector<uint64_t> exemptRoles{};///< The role ids that should not be affected by the rule(Maximum of 20).
		TriggerMetaData triggerMetadata{};///< The trigger metadata.
		std::vector<ActionData> actions{};///< The actions which will execute when the rule is triggered
		TriggerType triggerType{};///< The trigger type.
		EventType eventType{};///< The event type.
		uint64_t guildId{};///< The AutoModerationRule within which to create the auto-moderation-rule.
		std::string name{};///< The rule name.
		bool enabled{};///< Whether the rule is enabled(False by default).
	};

	/// For when an auto-moderation-rule is executed. \brief For when an auto-moderation-rule is executed.
	struct AutoModerationActionExecutionEventData {
		uint64_t alertSystemMessageId{};///< The id of any system auto moderation messages posted as a result of this action.
		TriggerType ruleTriggerType{};///< The trigger type of rule which was triggered.
		std::string matchedKeyword{};///< The word or phrase configured in the rule that triggered the rule
		std::string matchedContent{};///< The substring in content that triggered the rule.
		std::string content{};///< The user generated text content.
		uint64_t channelId{};///< The id of the channel in which user content was posted.
		uint64_t messageId{};///< The id of any user message which content belongs to.
		ActionData action{};///< The action which was executed.
		uint64_t guildId{};///< The id of the guild in which action was executed.
		uint64_t ruleId{};///< The id of the rule which action belongs to.
		uint64_t userId{};///< The id of the user which generated the content which triggered the rule.

		AutoModerationActionExecutionEventData() = default;

		AutoModerationActionExecutionEventData& operator=(const nlohmann::json& jsonObjectData) {
			this->parseObjectReal(jsonObjectData, this);
			return *this;
		}

		AutoModerationActionExecutionEventData(const nlohmann::json& jsonObjectData) {
			*this = jsonObjectData;
		}

		virtual ~AutoModerationActionExecutionEventData() = default;

	  	inline void parseObjectReal(const nlohmann::json& jsonObjectData, AutoModerationActionExecutionEventData* pDataStructure) {
			if (jsonObjectData.contains("alert_system_message_id") && !jsonObjectData["alert_system_message_id"].is_null()) {
				pDataStructure->alertSystemMessageId = stoull(jsonObjectData["alert_system_message_id"].get<std::string>());
			}

			if (jsonObjectData.contains("rule_trigger_type") && !jsonObjectData["rule_trigger_type"].is_null()) {
				pDataStructure->ruleTriggerType = jsonObjectData["rule_trigger_type"].get<TriggerType>();
			}

			if (jsonObjectData.contains("matched_keyword") && !jsonObjectData["matched_keyword"].is_null()) {
				pDataStructure->matchedKeyword = jsonObjectData["matched_keyword"].get<std::string>();
			}

			if (jsonObjectData.contains("matched_content") && !jsonObjectData["matched_content"].is_null()) {
				pDataStructure->matchedContent = jsonObjectData["matched_content"].get<std::string>();
			}

			if (jsonObjectData.contains("action") && !jsonObjectData["action"].is_null()) {
				pDataStructure->action = jsonObjectData["action"];
			}

			if (jsonObjectData.contains("content") && !jsonObjectData["content"].is_null()) {
				pDataStructure->content = jsonObjectData["content"].get<std::string>();
			}

			if (jsonObjectData.contains("message_id") && !jsonObjectData["message_id"].is_null()) {
				pDataStructure->messageId = stoull(jsonObjectData["message_id"].get<std::string>());
			}

			if (jsonObjectData.contains("channel_id") && !jsonObjectData["channel_id"].is_null()) {
				pDataStructure->channelId = stoull(jsonObjectData["channel_id"].get<std::string>());
			}

			if (jsonObjectData.contains("guild_id") && !jsonObjectData["guild_id"].is_null()) {
				pDataStructure->guildId = stoull(jsonObjectData["guild_id"].get<std::string>());
			}

			if (jsonObjectData.contains("rule_id") && !jsonObjectData["rule_id"].is_null()) {
				pDataStructure->ruleId = stoull(jsonObjectData["rule_id"].get<std::string>());
			}

			if (jsonObjectData.contains("user_id") && !jsonObjectData["user_id"].is_null()) {
				pDataStructure->userId = stoull(jsonObjectData["user_id"].get<std::string>());
			}
		}
	};

	/// For modifying an auto-moderation-rule. \brief For modifying an auto-moderation-rule.
	struct ModifyAutoModerationRuleData {
		std::vector<uint64_t> exemptChannels{};///< The channel ids that should not be affected by the rule(Maximum of 50).
		std::vector<uint64_t> exemptRoles{};///< The role ids that should not be affected by the rule(Maximum of 20).
		TriggerMetaData triggerMetadata{};///< The trigger metadata.
		std::vector<ActionData> actions{};///< The actions which will execute when the rule is triggered
		uint64_t autoModerationRuleId{};///< The id of the auto-moderation-rule you would like to modify.
		EventType eventType{};///< The event type.
		uint64_t guildId{};///< The AutoModerationRule within which to modify the auto-moderation-rule.
		std::string name{};///< The rule name.
		bool enabled{};///< Whether the rule is enabled(False by default).
	};

	/// For deleting an auto-moderation-rule. \brief For deleting an auto-moderation-rule.
	struct DeleteAutoModerationRuleData {
		uint64_t autoModerationRuleId{};///< The id of the auto-moderation-rule you would like to delete.
		uint64_t guildId{};///< The AutoModerationRule within which to delete the auto-moderation-rule.
	};

	/**@}*/

	/**
	 * \addtogroup main_endpoints
	 * @{
	 */

	class AutoModerationRules {
	  public:
		static void initialize(DiscordCoreInternal::HttpsClient*);

		CoRoutine<AutoModerationRuleVector> listAutoModerationRulesForGuildAsync(ListAutoModerationRulesForGuildData dataPackage);

		CoRoutine<AutoModerationRule> getAutoModerationRuleAsync(GetAutoModerationRuleData dataPackage);

		CoRoutine<AutoModerationRule> createAutoModerationRuleAsync(CreateAutoModerationRuleData dataPackage);

		CoRoutine<AutoModerationRule> modifyAutoModerationRuleAsync(ModifyAutoModerationRuleData dataPackage);

		CoRoutine<void> deleteAutoModerationRuleAsync(DeleteAutoModerationRuleData dataPackage);

	  protected:
		static DiscordCoreInternal::HttpsClient* httpsClient;
	};

	/**@}*/

}// namespace DiscordCoreInternal
