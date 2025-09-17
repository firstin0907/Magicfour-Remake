#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <wrl.h>

class ShaderClass
{
public:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	ShaderClass();
	~ShaderClass() = default;

	template<typename T>
	ID3D11Buffer* CreateBasicConstantBuffer(ID3D11Device* device);

protected:
	void CreateShaderObject(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd,
		const WCHAR* vs_filename, const WCHAR* ps_filename,
		D3D11_INPUT_ELEMENT_DESC polygon_layout[], int kNumOfElements);

	ID3D11SamplerState* CreateSamplerState(ID3D11Device* device,
		D3D11_TEXTURE_ADDRESS_MODE mode = D3D11_TEXTURE_ADDRESS_WRAP);

	void OutputShaderErrorMessage(ID3D10Blob* error_message,
		HWND hwnd, const WCHAR* shader_filename);

protected:
	ComPtr<ID3D11VertexShader>	vertex_shader_;
	ComPtr<ID3D11PixelShader>	pixel_shader_;
	ComPtr<ID3D11InputLayout>	input_layout_;
};

template<typename T>
inline ID3D11Buffer* ShaderClass::CreateBasicConstantBuffer(ID3D11Device* device)
{
	ID3D11Buffer* buffer;

	D3D11_BUFFER_DESC constantBufferDesc;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	constantBufferDesc.ByteWidth = sizeof(T);

	HRESULT result = device->CreateBuffer(&constantBufferDesc, NULL, &buffer);
	if (FAILED(result)) return nullptr;
	else return buffer;
}
