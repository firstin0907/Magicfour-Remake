#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <wrl.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class TextureClass
{
private:
    struct TargaHeader
    {
        unsigned char data1[12];
        unsigned short width;
        unsigned short height;
        unsigned char bpp;
        unsigned char data2;
    };

public:
    TextureClass(ID3D11Device* device,
        ID3D11DeviceContext* deviceContext, const char* filename);
    TextureClass(const TextureClass& other);
    ~TextureClass();

    bool Initialize(ID3D11Device* device,
        ID3D11DeviceContext* deviceContext, const char* filename);
    void Shutdown();

    ID3D11ShaderResourceView* GetTexture();

    int GetWidth();
    int GetHeight();

private:
    bool LoadTarga32Bit(const char* filename);
    bool LoadTarga24Bit(const char* filename);

    unsigned char* m_targaData; // raw data
    ComPtr<ID3D11Texture2D> m_texture; // real texture
    ComPtr<ID3D11ShaderResourceView> m_textureView; // texture Á¢±Ù¿ë
    int m_width, m_height;
};

