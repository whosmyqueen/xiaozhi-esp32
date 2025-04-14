#include "agora_rtc_protocol.h"
#include "application.h"
#include "assets/lang_config.h"
#include "board.h"
#include "settings.h"
#include <cstring>
#include <esp_log.h>

#define TAG "RTC"
static AgoraRtcProtocol* instance_;
AgoraRtcProtocol::AgoraRtcProtocol() {
	instance_ = this;
	event_group_handle_ = xEventGroupCreate();
}

AgoraRtcProtocol::~AgoraRtcProtocol() {
	vEventGroupDelete(event_group_handle_);
	instance_ = NULL;
}

void AgoraRtcProtocol::Start() { StartRtcClient(false); }

void AgoraRtcProtocol::SendAudio(const std::vector<uint8_t>& data) {
	if (engine_initd_ && engine_connected_ && engine_joined_) {
		audio_frame_info_t audio_frame_info = {.data_type = AUDIO_DATA_TYPE_OPUS};
		busy_sending_audio_ = true;
		int rval = agora_rtc_send_audio_data(g_conn_id, data.data(), data.size(), &audio_frame_info);
		if (rval < 0) {
			ESP_LOGI(TAG, "Failed to send audio data, reason: %s\n", agora_rtc_err_2_str(rval));
			SetError(Lang::Strings::SERVER_ERROR);
		}
		busy_sending_audio_ = false;
	}
}

bool AgoraRtcProtocol::OpenAudioChannel() {
	if (!engine_initd_ || !engine_connected_ || !engine_joined_) {
		ESP_LOGI(TAG, "AgoraRtc is not connected, try to connect now");
		if (!StartRtcClient(true)) {
			return false;
		}
	}
	busy_sending_audio_ = false;
	error_occurred_ = false;
	if (on_audio_channel_opened_ != nullptr) {
		on_audio_channel_opened_();
	}
	return true;
}

void AgoraRtcProtocol::CloseAudioChannel() {
	if (engine_initd_) {
		agora_rtc_leave_channel(g_conn_id);
		agora_rtc_destroy_connection(g_conn_id);
		agora_rtc_fini();
		engine_initd_ = false;
		engine_connected_ = false;
		engine_joined_ = false;
	}
	if (on_audio_channel_closed_ != nullptr) {
		on_audio_channel_closed_();
	}
}

bool AgoraRtcProtocol::IsAudioChannelOpened() const {
	return engine_initd_ && engine_connected_ && engine_joined_ && !error_occurred_ && !IsTimeout();
}

void AgoraRtcProtocol::ParseServerHello(const cJSON* root) {}

bool AgoraRtcProtocol::SendText(const std::string& text) { return false; }

bool AgoraRtcProtocol::StartRtcClient(bool report_error) {
	Settings settings("realtime_config", false);
	app_id_ = settings.GetString("app_id");
	channel_name_ = settings.GetString("channel_name");
	token_ = settings.GetString("token");
	room_user_id_ = settings.GetInt("room_user_id");
	user_ = settings.GetString("user");
	agora_rtc_event_handler_t handler = {
	    .on_join_channel_success = _on_join_channel_success,
	    .on_audio_data = _on_audio_data,
	    .on_stream_message = _on_stream_message,
	};
	rtc_service_option_t service_opt = {0};
	service_opt.area_code = AREA_CODE_GLOB;
	service_opt.log_cfg.log_disable = false;
	service_opt.log_cfg.log_level = RTC_LOG_WARNING;
	service_opt.license_value[0] = '\0';
	service_opt.domain_limit = false;
	int rval = agora_rtc_init(app_id_.c_str(), &handler, &service_opt);
	if (rval < 0) {
		ESP_LOGI(TAG, "Failed to initialize Agora sdk, app_id: %s, reason: %s\n", app_id_.c_str(), agora_rtc_err_2_str(rval));
		SetError(Lang::Strings::SERVER_ERROR);
		return ESP_FAIL;
	}
	instance_->engine_initd_ = true;
	ESP_LOGI(TAG, "start Create connection\n");
	// 2. API: Create connection
	rval = agora_rtc_create_connection(&g_conn_id);
	if (rval < 0) {
		ESP_LOGI(TAG, "Failed to create connection, reason: %s\n", agora_rtc_err_2_str(rval));
		SetError(Lang::Strings::SERVER_ERROR);
		return ESP_FAIL;
	}
	instance_->engine_connected_ = true;
	ESP_LOGI(TAG, "Create connection success\n");
	// 3. API: join channel
	rtc_channel_options_t channel_options = {0};

	channel_options.auto_subscribe_audio = true;
	channel_options.auto_subscribe_video = false;
	channel_options.audio_codec_opt.audio_codec_type = AUDIO_CODEC_DISABLED;
	channel_options.audio_codec_opt.pcm_sample_rate = 16000;
	channel_options.audio_codec_opt.pcm_channel_num = 1;
	channel_options.audio_codec_opt.pcm_duration = OPUS_FRAME_DURATION_MS;
	const char* version = agora_rtc_get_version();
	ESP_LOGI(TAG, "start join room\n");
	ESP_LOGI(TAG, "agora version: %s\n", version);
	ESP_LOGI(TAG, "room_user_id: %d", room_user_id_);
	ESP_LOGI(TAG, "app_id: %s", app_id_.c_str());
	ESP_LOGI(TAG, "token: %s", token_.c_str());
	ESP_LOGI(TAG, "channel_name: %s", channel_name_.c_str());
	rval = agora_rtc_join_channel(g_conn_id, channel_name_.c_str(), room_user_id_, token_.c_str(), &channel_options);
	if (rval < 0) {
		ESP_LOGI(TAG, "Failed to join channel, reason: %s \n", agora_rtc_err_2_str(rval));
		SetError(Lang::Strings::SERVER_ERROR);
		return ESP_FAIL;
	}
	xEventGroupWaitBits(event_group_handle_, JOIN_EVENT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
	return true;
}

void AgoraRtcProtocol::_on_join_channel_success(connection_id_t conn_id, uint32_t uid, int elapsed) {
	ESP_LOGI(TAG, "__on_join_channel_success\n");
	connection_info_t conn_info = {0};
	agora_rtc_get_connection_info(conn_id, &conn_info);
	ESP_LOGI(TAG, "[conn-%lu] Join the channel %s successfully, uid %lu elapsed %d ms\n", conn_id, conn_info.channel_name, uid, elapsed);
	instance_->engine_joined_ = true;
	xEventGroupSetBits(instance_->event_group_handle_, JOIN_EVENT_BIT);
}

void AgoraRtcProtocol::_on_audio_data(connection_id_t conn_id, const uint32_t uid, uint16_t sent_ts, const void* data, size_t len,
                                      const audio_frame_info_t* info_ptr) {
	if (instance_ != nullptr && instance_->on_incoming_audio_ != nullptr) {
		instance_->on_incoming_audio_(std::vector<uint8_t>((uint8_t*)data, (uint8_t*)data + len));
		instance_->last_incoming_time_ = std::chrono::steady_clock::now();
	}
}

void AgoraRtcProtocol::_on_stream_message(connection_id_t conn_id, uint32_t uid, int stream_id, const char* data, size_t length,
                                          uint64_t sent_ts) {
	ESP_LOGI(TAG, "[conn-%lu] on_stream_message: %s\n", conn_id, data);
}
