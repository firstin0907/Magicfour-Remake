#include "../include/TextureClass.hh"

#include "../include/GameException.hh"
#include "../third-party/DirectXTex.h"

using namespace DirectX;

IMAGE_FILE_EXTENSION getExtension(const std::wstring filename)
{
	auto extension = filename.substr(filename.find(L'.'));

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
		

	case FILE_EXTENSION_TGA:
		hResult = LoadFromTGAFile(filename, TGA_FLAGS_NONE, nullptr, image);

	}

	if (FAILED(hResult)) throw GAME_EXCEPTION(L"Failed to Read" + std::wstring(filename));

	hResult = CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), m_textureView.GetAddressOf());
	if (FAILED(hResult)) throw GAME_EXCEPTION(L"Failed to create Shader Resource View of " + std::wstring(filename));
}

TextureClass::~TextureClass()
{
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_textureView.Get();
}

int TextureClass::GetWidth()
{
	return m_width;
}

int TextureClass::GetHeight()
{
	return m_height;
}
