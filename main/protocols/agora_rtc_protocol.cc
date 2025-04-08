#include "agora_rtc_protocol.h"
#include "application.h"
#include "board.h"
#include "system_info.h"

#define TAG "RTC"

AgoraRtcProtocol::AgoraRtcProtocol() {
    base_url = CONFIG_REALTIME_SERVER_URL;
	std::string token = "Bearer " + std::string(CONFIG_SENSEFLOW_APP_KEY);
    headers_["Authorization"] = token;
    headers_["Content-Type"] = "application/json";
	event_group_handle_ = xEventGroupCreate();
	join_event = xEventGroupCreate();
}

AgoraRtcProtocol::~AgoraRtcProtocol() {
	vEventGroupDelete(event_group_handle_);
	vEventGroupDelete(join_event);
}

void AgoraRtcProtocol::Start() {
	std::string url = base_url + "realtime";
	auto http = Board::GetInstance().CreateHttp();
}

void AgoraRtcProtocol::SendAudio(const std::vector<uint8_t>& data) {}

bool AgoraRtcProtocol::OpenAudioChannel() {
	error_occurred_ = false;
	//   std::string url = CONFIG_WEBSOCKET_URL;
	//   std::string token = "Bearer " +
	//   std::string(CONFIG_WEBSOCKET_ACCESS_TOKEN);
	agora_rtc_event_handler_t handler = {
	    //   .on_join_channel_success =
	    //   AgoraRtcProtocol::_on_join_channel_success,
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
	int rval = agora_rtc_init(g_app.app_id, &handler, &service_opt);
	return false;
}

void AgoraRtcProtocol::CloseAudioChannel() {}

bool AgoraRtcProtocol::IsAudioChannelOpened() const { return false; }

void AgoraRtcProtocol::ParseServerHello(const cJSON* root) {}

void AgoraRtcProtocol::SendText(const std::string& text) {}

void AgoraRtcProtocol::_on_join_channel_success(connection_id_t conn_id, uint32_t uid, int elapsed) {
	ESP_LOGI(TAG, "__on_join_channel_success\n");
	connection_info_t conn_info = {0};
	agora_rtc_get_connection_info(conn_id, &conn_info);
	ESP_LOGI(TAG, "[conn-%lu] Join the channel %s successfully, uid %lu elapsed %d ms\n", conn_id, conn_info.channel_name, uid, elapsed);
	xEventGroupSetBits(join_event, JOIN_EVENT_BIT);
}
