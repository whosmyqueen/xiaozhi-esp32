#include "rtc_protocol.h"

RtcProtocol::RtcProtocol() {
    event_group_handle_ = xEventGroupCreate();
}

RtcProtocol::~RtcProtocol() {
    vEventGroupDelete(event_group_handle_);
}

void RtcProtocol::Start() {}

void RtcProtocol::SendAudio(const std::vector<uint8_t> &data)
{
}

bool RtcProtocol::OpenAudioChannel()
{
    return false;
}

void RtcProtocol::CloseAudioChannel() {}

bool RtcProtocol::IsAudioChannelOpened() const
{
    return false;
}
