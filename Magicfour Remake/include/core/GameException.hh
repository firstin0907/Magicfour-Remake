#pragma once

#include <stdexcept>
#include <string>
#include <cstring>
#include <ostream>
#include <windows.h>

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

#define GAME_EXCEPTION(ERROR_MESSAGE) GameException(ERROR_MESSAGE, WFILE, __LINE__)

class GameException : public std::exception
{
	std::wstring error_message;
	std::wstring source_name;
	int line_no;

public:
	GameException(const wchar_t* error_message,
		const wchar_t* source_name = L"", const int line_no = 0)
		: std::exception("Game Exception"), error_message(error_message),
		source_name(source_name), line_no(line_no) {}

	GameException(const std::wstring& error_message,
		const wchar_t* source_name = L"", const int line_no = 0)
		: std::exception("Game Exception"), error_message(error_message),
		source_name(source_name), line_no(line_no) {}

	inline std::wstring to_wstring() const
	{
		return error_message + L" in " + source_name
			+ L"(" + std::to_wstring(line_no) + L")";
	}	
};

inline GameException filenotfound_error(
	const char* filename, const wchar_t* source_name = L"", const int line_no = 0)
{
	std::wstring error_message = L"File not found : " + std::wstring(filename, filename + strlen(filename));
	return GameException(error_message, source_name, line_no);
}

inline GameException filenotfound_error(
	const wchar_t* filename, const wchar_t* source_name = L"", const int line_no = 0)
{
	std::wstring error_message = L"File not found : " + std::wstring(filename);
	return GameException(error_message, source_name, line_no);
}

inline GameException fileformat_error(
	const char* filename, const wchar_t* source_name = L"", const int line_no = 0)
{
	std::wstring error_message = L"Unmatched File Format: " + std::wstring(filename, filename + strlen(filename));
	return GameException(error_message, source_name, line_no);
}

inline GameException fileformat_error(
	const wchar_t* filename, const wchar_t* source_name = L"", const int line_no = 0)
{
	std::wstring error_message = L"Unmatched File Format: " + std::wstring(filename);
	return GameException(error_message, source_name, line_no);
}

inline GameException shadercompile_error(
	const wchar_t* filename, const wchar_t* source_name = L"", const int line_no = 0)
{
	return GameException(L"Shader Compilation Error. ("+ std::wstring(filename)
		+ L") Check shader-error.txt for message.", source_name, line_no);
}