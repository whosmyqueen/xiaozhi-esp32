#ifndef _AGORA_RTC_PROTOCOL_H_
#define _AGORA_RTC_PROTOCOL_H_

#include "agora_rtc_api.h"
#include "protocol.h"
#include <cJSON.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <functional>
#include <string>
#include <map>
#include <mutex>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define JOIN_EVENT_BIT (1 << 0)

class AgoraRtcProtocol : public Protocol {
public:
	AgoraRtcProtocol();
	~AgoraRtcProtocol();

	void Start() override;
	void SendAudio(const std::vector<uint8_t>& data) override;
	bool OpenAudioChannel() override;
	void CloseAudioChannel() override;
	bool IsAudioChannelOpened() const override;

private:
	EventGroupHandle_t event_group_handle_;
	EventGroupHandle_t join_event;
	std::string app_id_;
    std::string channel_name_;
    std::string token_;
    int room_user_id_;
    std::string user_;

	void ParseServerHello(const cJSON* root);
	void SendText(const std::string& text) override;

	void _on_join_channel_success(connection_id_t conn_id, uint32_t uid, int elapsed);
};

#endif