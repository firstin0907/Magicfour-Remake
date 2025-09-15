#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <wrl.h>
#include <string>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

enum IMAGE_FILE_EXTENSION
{
    FILE_EXTENSION_JPEG,
    FILE_EXTENSION_PNG,
    FILE_EXTENSION_TGA,
    FILE_EXTENSION_UNKNOWN
};


class TextureClass
{
public:
    TextureClass(ID3D11Device* device, const wchar_t* filename);
    TextureClass(ID3D11Device* device, const std::string& filename);
    TextureClass(const TextureClass& other) = delete;
    ~TextureClass();

    ID3D11ShaderResourceView* GetTexture();

    int GetWidth();
    int GetHeight();

private:
    ComPtr<ID3D11ShaderResourceView> textureView_; // texture ���ٿ�
    int width_, height_;
};

