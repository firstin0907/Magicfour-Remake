#pragma once

#include <d2d1.h>
#include <dwrite.h>

#include <wrl.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

class D2DClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	D2DClass(IDXGISwapChain* swapChain, HWND hwnd);

	void BeginDraw();
	void EndDraw();

	void DrawText(const wchar_t* contents);

private:
	HWND m_hwnd;
	ComPtr<ID2D1RenderTarget> m_D2Rtg;
	ComPtr<IDWriteTextFormat> m_Format;
	ComPtr<ID2D1SolidColorBrush> m_brush;
};