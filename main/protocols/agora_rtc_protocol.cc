#include "agora_rtc_protocol.h"
#include "board.h"
#include "system_info.h"
#include "application.h"

AgoraRtcProtocol::AgoraRtcProtocol() {
    event_group_handle_ = xEventGroupCreate();
}

AgoraRtcProtocol::~AgoraRtcProtocol() {
    vEventGroupDelete(event_group_handle_);
}

void AgoraRtcProtocol::Start() {}

void AgoraRtcProtocol::SendAudio(const std::vector<uint8_t> &data)
{
}

bool AgoraRtcProtocol::OpenAudioChannel()
{
    return false;
}

void AgoraRtcProtocol::CloseAudioChannel() {}

bool AgoraRtcProtocol::IsAudioChannelOpened() const
{
    return false;
}
