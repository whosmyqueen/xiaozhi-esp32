#include "realtime.h"
#include "assets/lang_config.h"
#include "board.h"
#include "settings.h"
#include "system_info.h"
#include <algorithm>
#include <cJSON.h>
#include <cstring>
#include <esp_app_format.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <sstream>
#include <vector>

#define TAG "Realtime"

Realtime::Realtime() { SetRealtileServerUrl(CONFIG_REALTIME_SERVER_URL); }

Realtime::~Realtime() {}

void Realtime::SetRealtileServerUrl(std::string realtime_server_base_url) {
	realtime_server_base_url_ = realtime_server_base_url;
}

void Realtime::SetHeader(const std::string& key, const std::string& value) { headers_[key] = value; }

void Realtime::SetPostData(const std::string& post_data) { post_data_ = post_data; }

bool Realtime::StartRealtime() {
	auto& board = Board::GetInstance();
	auto app_desc = esp_app_get_description();

	// Check if there is a new firmware version available
	current_version_ = app_desc->version;
	ESP_LOGI(TAG, "Current version: %s", current_version_.c_str());

	if (realtime_server_base_url_.length() < 10) {
		ESP_LOGE(TAG, "Realtime URL is not properly set");
		return false;
	}

	auto http = SetupHttp();

	std::string data = board.GetRealtimeJson();
	std::string method = data.length() > 0 ? "POST" : "GET";
	std::string url = realtime_server_base_url_ + std::string("realtime");
	if (!http->Open(method, url, data)) {
		ESP_LOGE(TAG, "Failed to open HTTP connection");
		delete http;
		return ESP_FAIL;
	}

	auto response = http->GetBody();
	delete http;

	cJSON* root = cJSON_Parse(response.c_str());
	if (root == NULL) {
		ESP_LOGE(TAG, "Failed to parse JSON response");
		return ESP_FAIL;
	}

	has_config_ = false;
	cJSON* code = cJSON_GetObjectItem(root, "code");
	cJSON* message = cJSON_GetObjectItem(root, "message");
	Settings settings("realtime_config", true);
	if (code != NULL && message != NULL) {
		settings.EraseAll();
		cJSON_Delete(root);
		ESP_LOGE(TAG, "Failed to start Realtime Config");
		return ESP_FAIL;
	}
	cJSON* app_id = cJSON_GetObjectItem(root, "app_id");
	cJSON* channel_name = cJSON_GetObjectItem(root, "channel_name");
	cJSON* token = cJSON_GetObjectItem(root, "token");
	cJSON* room_user_id = cJSON_GetObjectItem(root, "room_user_id");
	cJSON* user = cJSON_GetObjectItem(root, "user");
	settings.SetString("app_id", app_id->valuestring);
	settings.SetString("channel_name", channel_name->valuestring);
	settings.SetString("token", token->valuestring);
	settings.SetInt("room_user_id", room_user_id->valueint);
	settings.SetString("user", user->valuestring);
	has_config_ = true;

	cJSON_Delete(root);
	return ESP_OK;
}

bool Realtime::PingRealtime() {
	auto app_desc = esp_app_get_description();

	// Check if there is a new firmware version available
	current_version_ = app_desc->version;
	ESP_LOGI(TAG, "Current version: %s", current_version_.c_str());

	if (realtime_server_base_url_.length() < 10) {
		ESP_LOGE(TAG, "Realtime URL is not properly set");
		return false;
	}
	auto http = SetupHttp();

	Settings settings("realtime_config", false);
	std::string url = realtime_server_base_url_ + "realtime/" + settings.GetString("channel_name") +
	                  "/ping?user=" + settings.GetString("user");
	if (!http->Open("GET", url)) {
		ESP_LOGE(TAG, "Failed to open HTTP connection");
		delete http;
		return false;
	}

	auto response = http->GetBody();
	http->Close();
	delete http;
	cJSON* root = cJSON_Parse(response.c_str());
	if (root == NULL) {
		ESP_LOGE(TAG, "Failed to parse JSON response");
		return false;
	}

	cJSON* code = cJSON_GetObjectItem(root, "code");
	cJSON* message = cJSON_GetObjectItem(root, "message");
	if (code != NULL || message != NULL) {
		if (code->type != cJSON_Number || code->valueint != 0) {
			ESP_LOGE(TAG, "Failed to ping realtime server: %s", message->valuestring);
			cJSON_Delete(root);
			return false;
		}
	}
	cJSON_Delete(root);
	return true;
}

void Realtime::MarkCurrentVersionValid() {
	auto partition = esp_ota_get_running_partition();
	if (strcmp(partition->label, "factory") == 0) {
		ESP_LOGI(TAG, "Running from factory partition, skipping");
		return;
	}

	ESP_LOGI(TAG, "Running partition: %s", partition->label);
	esp_ota_img_states_t state;
	if (esp_ota_get_state_partition(partition, &state) != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get state of partition");
		return;
	}

	if (state == ESP_OTA_IMG_PENDING_VERIFY) {
		ESP_LOGI(TAG, "Marking firmware as valid");
		esp_ota_mark_app_valid_cancel_rollback();
	}
}

Http* Realtime::SetupHttp() {
	auto& board = Board::GetInstance();
	auto app_desc = esp_app_get_description();

	auto http = board.CreateHttp();
	for (const auto& header : headers_) {
		http->SetHeader(header.first, header.second);
	}

	http->SetHeader("Device-Id", SystemInfo::GetMacAddress().c_str());
	http->SetHeader("Client-Id", board.GetUuid());
	http->SetHeader("Authorization", std::string("Bearer ") + std::string(CONFIG_SENSEFLOW_APP_KEY));
	http->SetHeader("User-Agent", std::string(BOARD_NAME "/") + app_desc->version);
	http->SetHeader("Accept-Language", Lang::CODE);
	http->SetHeader("Content-Type", "application/json");

	return http;
}
