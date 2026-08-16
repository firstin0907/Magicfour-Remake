#include "core/configuration/ConfigManager.hh"

#include <vector>
#include <xstring>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "core/configuration/ConfigDefinitions.hh"

ConfigManager::ConfigManager(const std::string& config_filepath)
	: config_filepath_(config_filepath)
	, config_map_([this, &config_filepath]() {
		auto loaded_config = LoadConfigFromFile(config_filepath);
		if (ValidateConfig(loaded_config)) return loaded_config;

		SaveConfigToFile(CreateDefaultConfig());
		return CreateDefaultConfig();
	}())
	, resolution_(ParseResolution())
{
}

Config::ConfigMap ConfigManager::LoadConfigFromFile(const std::string& config_filepath)
{
	Config::ConfigMap result;
	std::wifstream file(config_filepath);
	if (!file.is_open()) return result;

	std::wstring line;
	while (std::getline(file, line))
	{
		line = Trim(line);
		if (line.empty() || line[0] == L'#' || line[0] == L';') continue;

		size_t delim_pos = line.find(L'=');
		if (delim_pos == std::wstring::npos) continue;

		std::wstring key = Trim(line.substr(0, delim_pos));
		std::wstring value = Trim(line.substr(delim_pos + 1));

		if (!key.empty()) result[key] = value;
	}
	return result;
}

bool ConfigManager::ValidateConfig(const Config::ConfigMap& config)
{
	for (const auto& [key, value_options] : Config::kSettingOption)
	{
		auto key_it = config.find(key);
		if (key_it == config.end()) return false;

		auto value_it = std::find_if(value_options.begin(), value_options.end(),
			[&](const wchar_t* element) { return element == key_it->second; });
		if (value_it == value_options.end()) return false;
	}
	return true;
}

Config::ConfigMap ConfigManager::CreateDefaultConfig()
{
	Config::ConfigMap result;
	for (const auto& [key, value_options] : Config::kSettingOption)
	{
		if (value_options.empty()) continue;
		result[key] = value_options[0];
	}
	return result;
}


void ConfigManager::SaveConfigToFile(const Config::ConfigMap& config) const
{
	std::wofstream file(config_filepath_);
	if (!file.is_open()) return;

	for (const auto& [key, value] : config) (file << key + L"=" + value + L"\n");
}

std::wstring ConfigManager::Trim(const std::wstring& str)
{
	if (str.empty()) return str;
	size_t first = str.find_first_not_of(L" \t\r\n");
	if (first == std::wstring::npos) return L"";
	size_t last = str.find_last_not_of(L" \t\r\n");
	return str.substr(first, (last - first + 1));
}


std::string ConfigManager::Trim(const std::string& str)
{
	if (str.empty()) return str;
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, (last - first + 1));
}