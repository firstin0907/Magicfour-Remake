#pragma once

#include <utility>
#include <vector>

namespace Config
{
	using MenuOption = std::pair<const wchar_t*, std::vector<const wchar_t*>>;
	inline const MenuOption kSettingOption[] =
	{
		{ L"Fullscreen",  { L"On", L"Off" } },
		{ L"Resolution",   { L"1280 x 720", L"3840 x 2160", L"2560 x 1440", L"1920 x 1080", L"1600 x 900"} }
	};
	constexpr int kSettingCount = sizeof(kSettingOption) / sizeof(MenuOption);

	using ConfigMap = std::unordered_map<std::wstring, std::wstring>;

}