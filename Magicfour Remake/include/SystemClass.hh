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
	LPCWSTR applicationName_;
	HINSTANCE hinstance_;
	HWND hwnd_;

	unique_ptr<class InputClass> input_;
	unique_ptr<class ApplicationClass> application_;
};


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static SystemClass* ApplicationHandle = nullptr;