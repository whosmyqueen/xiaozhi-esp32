#ifndef _AGORA_RTC_PROTOCOL_H_
#define _AGORA_RTC_PROTOCOL_H_

#include "agora_rtc_api.h"
#include "protocol.h"
#include <cJSON.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <functional>
#include <map>
#include <mutex>
#include <string>

#define JOIN_EVENT_BIT (1 << 0)

class AgoraRtcProtocol : public Protocol {
public:
	AgoraRtcProtocol();
	~AgoraRtcProtocol();

	bool Start() override;
	void SendAudio(const std::vector<uint8_t>& data) override;
	bool OpenAudioChannel() override;
	void CloseAudioChannel() override;
	bool IsAudioChannelOpened() const override;

private:
	EventGroupHandle_t event_group_handle_;

	std::string app_id_;
	std::string channel_name_;
	std::string token_;
	int room_user_id_;
	std::string user_;

	bool engine_initd_ = false;
	bool engine_connected_ = false;
	bool engine_joined_ = false;
	connection_id_t g_conn_id;

	void ParseServerHello(const cJSON* root);
	bool SendText(const std::string& text) override;
	bool StartRtcClient(bool report_error = false);
	static void _on_join_channel_success(connection_id_t conn_id, uint32_t uid, int elapsed);
	static void _on_audio_data(connection_id_t conn_id, const uint32_t uid, uint16_t sent_ts, const void *data, size_t len,const audio_frame_info_t *info_ptr);
	static void _on_stream_message(connection_id_t conn_id, uint32_t uid, int stream_id, const char *data, size_t length, uint64_t sent_ts);
};

#endif