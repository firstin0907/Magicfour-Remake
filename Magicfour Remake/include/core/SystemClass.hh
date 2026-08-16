#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <memory>

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&) = delete;
	~SystemClass();

	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR application_name_;
	HINSTANCE hinstance_;
	HWND hwnd_;

	std::unique_ptr<class ConfigManager> config_;
	std::unique_ptr<class InputClass> input_;
	std::unique_ptr<class ApplicationClass> application_;
};


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static SystemClass* ApplicationHandle = nullptr;