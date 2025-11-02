#include "graphics/FontClass.hh"

#include <xstring>

#include "core/D2DClass.hh"

FontClass::FontClass(class D2DClass* direct2D, const std::string& font_family, float font_size,
	DWRITE_TEXT_ALIGNMENT text_alignment, DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment)
{
	std::wstring family(font_family.begin(), font_family.end());

	text_format_ = direct2D->CreateTextFormat(family.c_str(), font_size,
		text_alignment, paragraph_alignment);
}
