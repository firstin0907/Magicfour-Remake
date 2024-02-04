#pragma once

#include "ShaderClass.hh"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>

using namespace DirectX;

class LightShaderClass : public ShaderClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

	struct MatrixBufferType
	{
		XMMATRIX mvp; // world * view * projection matrix
		XMMATRIX world_tr_inv;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;  // Added extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
	};

public:
	LightShaderClass(ID3D11Device* device, HWND hwnd);
	LightShaderClass(const LightShaderClass&) = delete;
	~LightShaderClass();

	void Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);

private:
	void InitializeShader(ID3D11Device* device, HWND hwnd,
		const WCHAR* vsFilename, const WCHAR* psFilename);

	void SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ComPtr<ID3D11SamplerState>	m_sampleState;
	ComPtr<ID3D11Buffer>		m_matrixBuffer;
	ComPtr<ID3D11Buffer>		m_lightBuffer;
};