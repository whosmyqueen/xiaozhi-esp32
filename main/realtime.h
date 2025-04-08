#ifndef _REALTIME_H
#define _REALTIME_H

#include <functional>
#include <map>
#include <string>

class Realtime {
public:
	Realtime();
	~Realtime();

	void SetCheckVersionUrl(std::string check_version_url);
	void SetHeader(const std::string& key, const std::string& value);
	void SetPostData(const std::string& post_data);
	bool CheckVersion();
	bool HasNewVersion() { return has_new_version_; }
	bool HasMqttConfig() { return has_mqtt_config_; }
	bool HasActivationCode() { return has_activation_code_; }
	bool HasServerTime() { return has_server_time_; }
	void StartUpgrade(std::function<void(int progress, size_t speed)> callback);
	void MarkCurrentVersionValid();

	const std::string& GetFirmwareVersion() const { return firmware_version_; }
	const std::string& GetCurrentVersion() const { return current_version_; }
	const std::string& GetActivationMessage() const { return activation_message_; }
	const std::string& GetActivationCode() const { return activation_code_; }

private:
	std::string real_time_server_base_url_;
	std::string activation_message_;
	std::string activation_code_;
	bool has_new_version_ = false;
	bool has_mqtt_config_ = false;
	bool has_server_time_ = false;
	bool has_activation_code_ = false;
	std::string current_version_;
	std::string firmware_version_;
	std::string firmware_url_;
	std::string post_data_;
	std::map<std::string, std::string> headers_;

	void Upgrade(const std::string& firmware_url);
	std::function<void(int progress, size_t speed)> upgrade_callback_;
	std::vector<int> ParseVersion(const std::string& version);
	bool IsNewVersionAvailable(const std::string& currentVersion, const std::string& newVersion);
};

#endif // _OTA_H
