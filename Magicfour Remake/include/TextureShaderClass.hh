#pragma once

#include "ShaderClass.hh"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>

class TextureShaderClass : public ShaderClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

	// cbuffer
	struct MatrixBufferType
	{
		XMMATRIX mvp; // world * view * projection matrix
	};

public:
	TextureShaderClass(ID3D11Device* device, HWND hwnd);
	TextureShaderClass(const TextureShaderClass& other) = delete;
	~TextureShaderClass();

	void Render(ID3D11DeviceContext* deviceContext, int indexCount,
		XMMATRIX worldMatrix, XMMATRIX vpMatrix, ID3D11ShaderResourceView* texture);

private:
	void InitializeShader(ID3D11Device* device, HWND hwnd,
		const WCHAR* vsFilename, const WCHAR* psFilename);

	void SetShaderParameters(ID3D11DeviceContext* deviceContext,
		XMMATRIX worldMatrix, XMMATRIX vpMatrix, ID3D11ShaderResourceView* texture);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ComPtr<ID3D11SamplerState>	m_sampleState;
	ComPtr<ID3D11Buffer>		m_matrixBuffer;
};

