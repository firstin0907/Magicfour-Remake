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

	float dpi = GetDpiForWindow(hwnd);
	D2D1_RENDER_TARGET_PROPERTIES props =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
		);
	d2dFactory->CreateDxgiSurfaceRenderTarget(
		backBuffer.Get(), &props, &m_D2Rtg);

	ComPtr<IDWriteFactory> dwFactory;
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(dwFactory.GetAddressOf())
	);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to initialize Dwrite factory");
	
	hr = dwFactory->CreateTextFormat(
		L"ÇÔÃÊ·Òµ¸¿ò", 0, DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
		45, L"ko", m_Format.GetAddressOf()
	);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to create text format");

	hr = m_Format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to set text alignment");

	hr = m_Format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to set text alignment");

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


void D2DClass::DrawText(const wchar_t* contents)
{
	D2D1_RECT_F layoutRect = D2D1::RectF(0.f, 0.f, 1000.0f, 100.f);

	m_D2Rtg->DrawText(contents, wcslen(contents),
		m_Format.Get(), layoutRect, m_brush.Get()
	);
}