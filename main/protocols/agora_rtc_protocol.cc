#include "agora_rtc_protocol.h"
#include "application.h"
#include "assets/lang_config.h"
#include "board.h"
#include "settings.h"
#include <cstring>
#include <esp_log.h>

#define TAG "RTC"

AgoraRtcProtocol::AgoraRtcProtocol() {
	instance_ = this;
	event_group_handle_ = xEventGroupCreate();
}

AgoraRtcProtocol::~AgoraRtcProtocol() {
	vEventGroupDelete(event_group_handle_);
	instance_ = NULL;
}

void AgoraRtcProtocol::Start() {}

void AgoraRtcProtocol::SendAudio(const std::vector<uint8_t>& data) {}

bool AgoraRtcProtocol::OpenAudioChannel() {
	error_occurred_ = false;
	Settings settings("realtime_config", false);
	app_id_ = settings.GetString("app_id");
	channel_name_ = settings.GetString("channel_name");
	token_ = settings.GetString("token");
	room_user_id_ = settings.GetInt("room_user_id");
	user_ = settings.GetString("user");
	agora_rtc_event_handler_t handler = {
	    .on_join_channel_success = _on_join_channel_success,
	    // .on_connection_lost = __on_connection_lost,
	    // .on_rejoin_channel_success = __on_rejoin_channel_success,
	    // .on_user_joined = __on_user_joined,
	    // .on_user_offline = __on_user_offline,
	    // .on_user_mute_audio = __on_user_mute_audio,
	    // .on_audio_data = __on_audio_data,
	    // .on_stream_message = __on_stream_message,
	    // .on_user_mute_video = __on_user_mute_video,
	    // .on_target_bitrate_changed = __on_target_bitrate_changed,
	    // .on_key_frame_gen_req = __on_key_frame_gen_req,
	    // .on_error = __on_error,
	};
	rtc_service_option_t service_opt = {0};
	service_opt.area_code = AREA_CODE_GLOB;
	service_opt.log_cfg.log_disable = false;
	service_opt.log_cfg.log_level = RTC_LOG_WARNING;
	service_opt.license_value[0] = '\0';
	service_opt.domain_limit = false;
	int rval = agora_rtc_init(app_id_, &handler, &service_opt, this);
	if (rval < 0)
    {
        ESP_LOGI(TAG, "Failed to initialize Agora sdk, app_id: %s, reason: %s\n", app_id_, agora_rtc_err_2_str(rval));
        return ESP_FAIL;
    }
	ESP_LOGI(TAG, "start Create connection\n");
    // 2. API: Create connection
    rval = agora_rtc_create_connection(&g_conn_id);
    if (rval < 0)
    {
        ESP_LOGI(TAG, "Failed to create connection, reason: %s\n", agora_rtc_err_2_str(rval));
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Create connection success\n");
	// 3. API: join channel
    rtc_channel_options_t channel_options = {0};

    channel_options.auto_subscribe_audio = true;
    channel_options.auto_subscribe_video = false;
	channel_options.audio_codec_opt.audio_codec_type = AUDIO_CODEC_TYPE_OPUS;
    channel_options.audio_codec_opt.pcm_sample_rate = 24000;
    channel_options.audio_codec_opt.pcm_channel_num = 1;
	char *version = agora_rtc_get_version();
    ESP_LOGI(TAG, "start join room\n");
    ESP_LOGI(TAG, "agora version: %s\n", version);
    ESP_LOGI(TAG, "room_user_id: %d", room_user_id_);
    ESP_LOGI(TAG, "app_id: %s", app_id_);
    ESP_LOGI(TAG, "token: %s", token_);
    ESP_LOGI(TAG, "channel_name: %s", channel_name_);
    rval = agora_rtc_join_channel(g_conn_id, channel_name_, room_user_id_, token_, &channel_options);
    if (rval < 0)
    {
        ESP_LOGI(TAG, "Failed to join channel, reason: %s \n", agora_rtc_err_2_str(rval));
        return ESP_FAIL;
    }
	xEventGroupWaitBits(event_group_handle_, JOIN_EVENT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "join room success\n");
	return true;
}

void AgoraRtcProtocol::CloseAudioChannel() {}

bool AgoraRtcProtocol::IsAudioChannelOpened() const { return false; }

void AgoraRtcProtocol::ParseServerHello(const cJSON* root) {}

void AgoraRtcProtocol::SendText(const std::string& text) {}

bool AgoraRtcProtocol::StartRtcClient(bool report_error) { return false; }

void AgoraRtcProtocol::_on_join_channel_success(connection_id_t conn_id, uint32_t uid, int elapsed) {
	ESP_LOGI(TAG, "__on_join_channel_success\n");
	connection_info_t conn_info = {0};
	agora_rtc_get_connection_info(conn_id, &conn_info);
	ESP_LOGI(TAG, "[conn-%lu] Join the channel %s successfully, uid %lu elapsed %d ms\n", conn_id, conn_info.channel_name, uid, elapsed);
	xEventGroupSetBits(instance_->event_group_handle_, JOIN_EVENT_BIT);
}