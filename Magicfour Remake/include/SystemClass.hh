#pragma once

#include <windows.h>

#include <memory>

#define WIN32_LEAN_AND_MEAN

using namespace std;

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
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	unique_ptr<class InputClass> m_Input;
	unique_ptr<class ApplicationClass> m_Application;
};


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static SystemClass* ApplicationHandle = nullptr;