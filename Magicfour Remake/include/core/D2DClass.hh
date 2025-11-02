#pragma once

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

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

	IDWriteTextFormat* CreateTextFormat(const wchar_t* fontFamily, float fontSize,
		DWRITE_TEXT_ALIGNMENT text_alignment,
		DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment);

	void SetBrushColor(D2D1_COLOR_F color);

	void RenderBitmap(class BitmapClass* bitmap, float left, float top);
	void RenderBitmap(BitmapClass* bitmap,
		D2D1_RECT_F dest_rect, D2D1_RECT_F source_rect, float opacity = 1.0f);
	void RenderRect(float left, float top, float right, float bottom);
	void RenderText(IDWriteTextFormat* format, const wchar_t* contents,
		float left, float top, float right, float bottom);
	void RenderText(class FontClass* format, const wchar_t* contents,
		float left, float top, float right, float bottom);
	void RenderTextWithInstantFormat(IDWriteTextFormat* format,
		const wchar_t* contents, float left, float top, float right, float bottom);

	inline ID2D1RenderTarget* GetRenderTarget()
	{
		return render_target_.Get();
	}

	inline IWICImagingFactory* GetWicFactory()
	{
		return wic_factory_.Get();
	}

private:
	HWND hwnd_;

	ComPtr<IDWriteFactory> dwFactory_;
	ComPtr<ID2D1RenderTarget> render_target_;
	ComPtr<ID2D1SolidColorBrush> brush_;

	ComPtr<IWICImagingFactory> wic_factory_;
};