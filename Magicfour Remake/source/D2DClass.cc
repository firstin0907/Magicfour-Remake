#include "../include/D2DClass.hh"

#include "../include/GameException.hh"

D2DClass::D2DClass(IDXGISwapChain* swapChain, HWND hwnd)
	: m_hwnd(hwnd)
{
	HRESULT hr;

	ComPtr<IDXGISurface> backBuffer;
	swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

	ComPtr<ID2D1Factory> d2dFactory;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
		d2dFactory.GetAddressOf());
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to initialize Direct2D factory");

	D2D1_RENDER_TARGET_PROPERTIES props =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
		);
	d2dFactory->CreateDxgiSurfaceRenderTarget(
		backBuffer.Get(), &props, &m_D2Rtg);

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(m_dwFactory.GetAddressOf())
	);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to initialize Dwrite factory");
	

	hr = m_D2Rtg->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_brush);
}

void D2DClass::BeginDraw()
{
	m_D2Rtg->BeginDraw();
}

void D2DClass::EndDraw()
{
	HRESULT hr = m_D2Rtg->EndDraw();
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to draw text");
}

IDWriteTextFormat* D2DClass::CreateTextFormat(const wchar_t* fontFamily, float fontSize,
	DWRITE_TEXT_ALIGNMENT text_alignment, DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment)
{
	IDWriteTextFormat* format;

	HRESULT hr = m_dwFactory->CreateTextFormat(
		fontFamily, 0, DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
		fontSize, L"ko", &format
	);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to create text format");

	hr = format->SetTextAlignment(text_alignment);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to set text alignment");

	hr = format->SetParagraphAlignment(paragraph_alignment);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to set text alignment");

	return format;
}

void D2DClass::SetBrushColor(D2D1_COLOR_F color)
{
	m_brush->SetColor(color);
}

void D2DClass::RenderText(IDWriteTextFormat* format,
	const wchar_t* contents, float left, float top, float right, float bottom)
{
	D2D1_RECT_F layoutRect = D2D1::RectF(left, top, right, bottom);

	m_D2Rtg->DrawText(contents, static_cast<UINT32>(wcslen(contents)),
		format, layoutRect, m_brush.Get());
}

void D2DClass::RenderTextWithInstantFormat(IDWriteTextFormat* format,
	const wchar_t* contents, float left, float top, float right, float bottom)
{
	D2D1_RECT_F layoutRect = D2D1::RectF(left, top, right, bottom);

	m_D2Rtg->DrawText(contents, static_cast<UINT32>(wcslen(contents)),
		format, layoutRect, m_brush.Get());

	format->Release();
}