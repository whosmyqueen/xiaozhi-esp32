#ifndef _REALTIME_H
#define _REALTIME_H

#include <functional>
#include <map>
#include <string>

class Realtime {
public:
	Realtime();
	~Realtime();

	void SetRealtileServerUrl(std::string realtime_server_base_url);
	void SetHeader(const std::string& key, const std::string& value);
	void SetPostData(const std::string& post_data);
	bool StartRealtime();
	bool StopRealtime();
	bool PingRealtime();
	bool HasConfig() { return has_config_; }
	void MarkCurrentVersionValid();

	const std::string& GetCurrentVersion() const { return current_version_; }


private:
	std::string realtime_server_base_url_;
	bool has_config_ = false;
	std::string current_version_;
	std::string post_data_;
	std::map<std::string, std::string> headers_;
};

#endif // _OTA_H
