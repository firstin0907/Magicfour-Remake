#include <xstring>
#include <unordered_map>
#include <charconv>
#include <stdexcept>
#include <typeinfo>

#include "ConfigDefinitions.hh"

template<typename T>
inline constexpr bool always_false_v = false;

class ConfigManager
{
private:
	static constexpr const wchar_t* kResolutionKey = L"Resolution";

public:
	ConfigManager(const std::string& config_filepath);

	template<typename ConfigValueType>
	ConfigValueType GetConfigValue(const std::wstring& key)
	{
		std::string key_string(key.begin(), key.end());

		auto it = config_map_.find(key);
		if (it == config_map_.end()) throw std::invalid_argument("Nonexistent Config Key: " + key_string);
		
		if constexpr (std::is_same_v<ConfigValueType, std::wstring>) return it->second;
		if constexpr (std::is_same_v<ConfigValueType, std::string>) return std::string(it->second.begin(), it->second.end());
		if constexpr (std::is_arithmetic_v<ConfigValueType>)
		{
			ConfigValueType result;
			auto [ptr, ec] = std::from_chars(it->second.data(), it->second.data() + it->second.size(), result);

			if (ec == std::errc()) return result;
			else throw std::invalid_argument("Invalid Type for key " + key_string + "(Expected " + typeid(ConfigValueType).name() + ")");
		}
		else
		{
			throw std::invalid_argument("Invalid Type for key " + key_string + "(Expected " + typeid(ConfigValueType).name() + ")");
			//static_assert(always_false_v<ConfigValueType>, "Unsupported type for GetConfigValue!");
		}
	}

	std::pair<int, int> GetResolution() { return resolution_; }
	
	void SaveConfigToFile(const Config::ConfigMap& config) const;

private:
	static Config::ConfigMap LoadConfigFromFile(const std::string& config_filepath);
	static Config::ConfigMap CreateDefaultConfig();
	static bool ValidateConfig(const Config::ConfigMap& config);

	static std::wstring Trim(const std::wstring& str);
	static std::string Trim(const std::string& str);

	std::pair<int, int> ParseResolution()
	{
		std::pair<int, int> result;

		std::string resolution_value = GetConfigValue<std::string>(kResolutionKey);
		size_t x_pos = resolution_value.find_first_of("xX");
		std::string width_string = Trim(resolution_value.substr(0, x_pos));
		std::string height_string = Trim(resolution_value.substr(x_pos + 1));

		auto [ptr1, ec1] = std::from_chars(width_string.data(), width_string.data() + width_string.size(), result.first);
		auto [ptr2, ec2] = std::from_chars(height_string.data(), height_string.data() + height_string.size(), result.second);

		if (ec1 != std::errc() || ec2 != std::errc()) throw std::runtime_error("Invaild Resolution!");
		return result;
	}

private:
	std::string config_filepath_;
	const Config::ConfigMap config_map_;
	const std::pair<int, int> resolution_;
};