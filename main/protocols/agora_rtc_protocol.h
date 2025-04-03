#ifndef _AGORA_RTC_PROTOCOL_H_
#define _AGORA_RTC_PROTOCOL_H_

#include "protocol.h"
#include "agora_rtc_api.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

class AgoraRtcProtocol : public Protocol {
public:
  AgoraRtcProtocol();
  ~AgoraRtcProtocol();

  void Start() override;
  void SendAudio(const std::vector<uint8_t> &data) override;
  bool OpenAudioChannel() override;
  void CloseAudioChannel() override;
  bool IsAudioChannelOpened() const override;

private:
  EventGroupHandle_t event_group_handle_;

  void ParseServerHello(const cJSON *root);
  void SendText(const std::string &text) override;
};