#pragma once

#include <wrl.h>
#include <string>
#include <dwrite.h>

class FontClass final
{
public:
	FontClass(class D2DClass* direct2D, const std::string& font_family, float font_size,
		DWRITE_TEXT_ALIGNMENT text_alignment, DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment);

	~FontClass() = default;

	inline struct IDWriteTextFormat* get();

	Microsoft::WRL::ComPtr<struct IDWriteTextFormat> text_format_;
};

struct IDWriteTextFormat* FontClass::get()
{
	return text_format_.Get();
}