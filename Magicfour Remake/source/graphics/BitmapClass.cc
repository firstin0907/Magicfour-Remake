#include "graphics/BitmapClass.hh"

#include <d2d1.h>
#include <wincodec.h>
#include <dwrite.h>
#include <xstring>

#include "core/D2DClass.hh"
#include "core/GameException.hh"

BitmapClass::BitmapClass(D2DClass* direct2d, const wchar_t* filename)
{
	ComPtr<IWICBitmapDecoder> decoder;
	ComPtr<IWICBitmapFrameDecode> frame;
	ComPtr<IWICFormatConverter> converter;

	HRESULT hr = direct2d->GetWicFactory()->CreateDecoderFromFilename(
		filename,                        // Image to be decoded
		NULL,                            // Do not prefer a particular vendor
		GENERIC_READ,                    // Desired read access to the file
		WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
		decoder.GetAddressOf()           // Pointer to the decoder
	);
	if (FAILED(hr)) throw filenotfound_error(filename, WFILE, __LINE__);

	hr = decoder->GetFrame(0, frame.GetAddressOf());
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to get frame of decoder");

	hr = direct2d->GetWicFactory()->CreateFormatConverter(converter.GetAddressOf());
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to create converter");

	hr = converter->Initialize(
		frame.Get(), GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		0, 0.0, WICBitmapPaletteTypeCustom
	);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to init converter");

	hr = direct2d->GetRenderTarget()->CreateBitmapFromWicBitmap(converter.Get(), bitmap_.GetAddressOf());
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to create bitmap");

	width_ = bitmap_->GetSize().width;
	height_ = bitmap_->GetSize().height;
}

BitmapClass::BitmapClass(D2DClass* direct2d, const std::string& filename)
	: BitmapClass(direct2d, std::wstring(filename.begin(), filename.end()).c_str())
{

}