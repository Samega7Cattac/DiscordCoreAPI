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
/// WebHookEntities.cpp - The source file for WebHook relate stuff.
/// Dec 1, 2021
/// https://discordcoreapi.com
/// \file WebHookEntities.cpp

#include <discordcoreapi/WebHookEntities.hpp>
#include <discordcoreapi/Https.hpp>
#include <discordcoreapi/DataParsingFunctions.hpp>
#include <discordcoreapi/CoRoutine.hpp>

namespace DiscordCoreAPI {

	void WebHooks::initialize(DiscordCoreInternal::HttpClient* theClient) {
		WebHooks::httpClient = theClient;
	}

	CoRoutine<WebHook> WebHooks::createWebHookAsync(CreateWebHookData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Post_Webhook);
		co_await NewThreadAwaitable<WebHook>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Post_Webhook;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Post;
		workload.relativePath = "/channels/" + std::to_string(dataPackage.channelId) + "/webhooks";
		workload.callStack = "WebHooks::createWebHookAsync";
		nlohmann::json responseData = { { "name", dataPackage.name } };
		if (dataPackage.avatar.size() > 0) {
			responseData.update({ { "avatar", dataPackage.avatar } });
		}
		workload.content = responseData.dump();
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<WebHook>(workload);
	}

	CoRoutine<std::vector<WebHook>> WebHooks::getChannelWebHooksAsync(GetChannelWebHooksData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Get_Channel_Webhooks);
		co_await NewThreadAwaitable<std::vector<WebHook>>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Get_Channel_Webhooks;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Get;
		workload.relativePath = "/channels/" + std::to_string(dataPackage.channelId) + "/webhooks";
		workload.callStack = "WebHooks::getChannelWebHooksAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<std::vector<WebHook>>(workload);
	}

	CoRoutine<std::vector<WebHook>> WebHooks::getGuildWebHooksAsync(GetGuildWebHooksData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Get_Guild_Webhooks);
		co_await NewThreadAwaitable<std::vector<WebHook>>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Get_Guild_Webhooks;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Get;
		workload.relativePath = "/guilds/" + std::to_string(dataPackage.guildId) + "/webhooks";
		workload.callStack = "WebHooks::getGuildWebHooksAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<std::vector<WebHook>>(workload);
	}

	CoRoutine<WebHook> WebHooks::getWebHookAsync(GetWebHookData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Get_Webhook);
		co_await NewThreadAwaitable<WebHook>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Get_Webhook;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Get;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId);
		workload.callStack = "WebHooks::getWebHookAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<WebHook>(workload);
	}

	CoRoutine<WebHook> WebHooks::getWebHookWithTokenAsync(GetWebHookWithTokenData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Get_Webhook_With_Token);
		co_await NewThreadAwaitable<WebHook>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Get_Webhook_With_Token;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Get;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId) + "/" + dataPackage.webhookToken;
		workload.callStack = "WebHooks::getWebHookWithTokenAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<WebHook>(workload);
	}

	CoRoutine<WebHook> WebHooks::modifyWebHookAsync(ModifyWebHookData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Patch_Webhook);
		co_await NewThreadAwaitable<WebHook>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Patch_Webhook;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Patch;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId);
		nlohmann::json responseData{};
		if (dataPackage.avatar.size() > 0) {
			responseData.update({ { "avatar", dataPackage.avatar } });
		}
		if (dataPackage.name != "") {
			responseData.update({ { "name", dataPackage.name } });
		}
		if (dataPackage.channelId != 0) {
			responseData.update({ { "channel_id", std::to_string(dataPackage.channelId) } });
		}
		workload.content = responseData.dump();
		workload.callStack = "WebHooks::modifyWebHookAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<WebHook>(workload);
	}

	CoRoutine<WebHook> WebHooks::modifyWebHookWithTokenAsync(ModifyWebHookWithTokenData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Patch_Webhook_With_Token);
		co_await NewThreadAwaitable<WebHook>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Patch_Webhook_With_Token;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Patch;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId) + "/" + dataPackage.webhookToken;
		nlohmann::json responseData{};
		if (dataPackage.avatar.size() > 0) {
			responseData.update({ { "avatar", dataPackage.avatar } });
		}
		if (dataPackage.name != "") {
			responseData.update({ { "name", dataPackage.name } });
		}
		if (dataPackage.channelId != 0) {
			responseData.update({ { "channel_id", std::to_string(dataPackage.channelId) } });
		}
		workload.content = responseData.dump();
		workload.callStack = "WebHooks::modifyWebHookWithTokenAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<WebHook>(workload);
	}

	CoRoutine<void> WebHooks::deleteWebHookAsync(DeleteWebHookData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Delete_Webhook);
		co_await NewThreadAwaitable<void>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Delete_Webhook;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Delete;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId);
		workload.callStack = "WebHooks::deleteWebHookAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<void>(workload);
	}

	CoRoutine<void> WebHooks::deleteWebHookWithTokenAsync(DeleteWebHookWithTokenData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Delete_Webhook_With_Token);
		co_await NewThreadAwaitable<void>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Delete_Webhook_With_Token;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Delete;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId) + "/" + dataPackage.webhookToken;
		workload.callStack = "WebHooks::deleteWebHookWithTokenAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<void>(workload);
	}

	CoRoutine<Message> WebHooks::executeWebHookAsync(ExecuteWebHookData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Post_Execute_Webhook);
		co_await NewThreadAwaitable<Message>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Post_Execute_Webhook;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Post;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId) + "/" + dataPackage.webhookToken;
		workload.callStack = "WebHooks::executeWebHookAsync";
		if (dataPackage.wait) {
			workload.relativePath += "?wait=true";
			if (dataPackage.threadId != 0) {
				workload.relativePath += "&thread_id=" + std::to_string(dataPackage.threadId);
			}
		}
		if (dataPackage.threadId != 0) {
			workload.relativePath += "?thread_id=" + std::to_string(dataPackage.threadId);
		}
		if (dataPackage.files.size() > 0) {
			workload.payloadType = DiscordCoreInternal::PayloadType::Multipart_Form;
			workload.content = constructMultiPartData(nlohmann::json::parse(DiscordCoreInternal::JSONIFY(dataPackage)), dataPackage.files);
		} else {
			workload.content = DiscordCoreInternal::JSONIFY(dataPackage);
		}
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<Message>(workload);
	}

	CoRoutine<Message> WebHooks::getWebHookMessageAsync(GetWebHookMessageData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Get_Webhook_Message);
		co_await NewThreadAwaitable<Message>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Get_Webhook_Message;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Get;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId) + "/" + dataPackage.webhookToken + "/messages/" + std::to_string(dataPackage.messageId);
		if (dataPackage.threadId != 0) {
			workload.relativePath += "?thread_id=" + std::to_string(dataPackage.threadId);
		}
		workload.callStack = "WebHooks::getWebHookMessageAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<Message>(workload);
	}

	CoRoutine<Message> WebHooks::editWebHookMessageAsync(EditWebHookData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Patch_Webhook_Message);
		co_await NewThreadAwaitable<Message>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Patch_Webhook_Message;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Patch;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId) + "/" + dataPackage.webhookToken + "/messages/" + std::to_string(dataPackage.messageId);
		if (dataPackage.threadId != 0) {
			workload.relativePath += "?thread_id=" + std::to_string(dataPackage.threadId);
		}
		if (dataPackage.files.size() > 0) {
			workload.payloadType = DiscordCoreInternal::PayloadType::Multipart_Form;
			workload.content = constructMultiPartData(nlohmann::json::parse(DiscordCoreInternal::JSONIFY(dataPackage)), dataPackage.files);
		} else {
			workload.content = DiscordCoreInternal::JSONIFY(dataPackage);
		}
		workload.callStack = "WebHooks::editWebHookMessageAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<Message>(workload);
	}

	CoRoutine<void> WebHooks::deleteWebHookMessageAsync(DeleteWebHookMessageData dataPackage) {
		DiscordCoreInternal::HttpWorkloadData workload{};
		workload.thisWorkerId = DiscordCoreInternal::HttpWorkloadData::getAndIncrementWorkloadId(DiscordCoreInternal::HttpWorkloadType::Delete_Webhook_Message);
		co_await NewThreadAwaitable<void>();
		workload.workloadType = DiscordCoreInternal::HttpWorkloadType::Delete_Webhook_Message;
		workload.workloadClass = DiscordCoreInternal::HttpWorkloadClass::Delete;
		workload.relativePath = "/webhooks/" + std::to_string(dataPackage.webhookId) + "/" + dataPackage.webhookToken + "/messages/" + std::to_string(dataPackage.messageId);
		if (dataPackage.threadId != 0) {
			workload.relativePath += "?thread_id=" + std::to_string(dataPackage.threadId);
		}
		workload.callStack = "WebHooks::deleteWebHookMessageAsync";
		co_return WebHooks::httpClient->submitWorkloadAndGetResult<void>(workload);
	}
	DiscordCoreInternal::HttpClient* WebHooks::httpClient{ nullptr };
}
