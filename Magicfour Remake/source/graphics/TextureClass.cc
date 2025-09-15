#include "graphics/TextureClass.hh"

#include <string>

#include "core/GameException.hh"
#include "../third-party/DirectXTex.h"

using namespace DirectX;

IMAGE_FILE_EXTENSION getExtension(const std::wstring filename)
{
	auto extension = filename.substr(filename.rfind(L'.'));

	if (extension == L".jpg" || extension == L".jpeg") return FILE_EXTENSION_JPEG;
	if (extension == L".png") return FILE_EXTENSION_PNG;
	if (extension == L".tga") return FILE_EXTENSION_TGA;

	return FILE_EXTENSION_UNKNOWN;
}


TextureClass::TextureClass(ID3D11Device* device, const wchar_t* filename)
{
	HRESULT hResult;
	ScratchImage image;

	switch (getExtension(filename))
	{
	case FILE_EXTENSION_JPEG:
	case FILE_EXTENSION_PNG:
		hResult = LoadFromWICFile(filename, WIC_FLAGS_NONE, nullptr, image);
		if (FAILED(hResult)) throw GAME_EXCEPTION(L"Failed to Read " + std::wstring(filename));
		break;

	case FILE_EXTENSION_TGA:
		hResult = LoadFromTGAFile(filename, TGA_FLAGS_NONE, nullptr, image);
		if (FAILED(hResult)) throw GAME_EXCEPTION(L"Failed to Read " + std::wstring(filename));
		break;
	}

	width_ = image.GetMetadata().width;
	height_ = image.GetMetadata().height;

	hResult = CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), textureView_.GetAddressOf());
	if (FAILED(hResult)) throw GAME_EXCEPTION(L"Failed to create Shader Resource View of " + std::wstring(filename));
}

TextureClass::TextureClass(ID3D11Device* device, const std::string& filename)
	: TextureClass(device, std::wstring(filename.begin(), filename.end()).c_str())
{

}

TextureClass::~TextureClass()
{
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return textureView_.Get();
}

int TextureClass::GetWidth()
{
	return width_;
}

int TextureClass::GetHeight()
{
	return height_;
}
