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
	TextureShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);
	TextureShaderClass(const TextureShaderClass& other) = delete;
	~TextureShaderClass() = default;

	void Render(int indexCount,
		XMMATRIX world_matrix, XMMATRIX vp_matrix, ID3D11ShaderResourceView* texture);

private:
	void InitializeShader(HWND hwnd,
		const WCHAR* vs_filename, const WCHAR* ps_filename);

	void SetShaderParameters(
		XMMATRIX world_matrix, XMMATRIX vp_matrix, ID3D11ShaderResourceView* texture);

	void RenderShader(int indexCount);

private:
	ComPtr<ID3D11SamplerState>	sample_state_;
	ComPtr<ID3D11Buffer>		matrix_buffer_;
};

