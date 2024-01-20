#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>

using namespace DirectX;

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class LightShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
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

	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*, const WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ComPtr<ID3D11VertexShader>	m_vertexShader;
	ComPtr<ID3D11PixelShader>	m_pixelShader;
	ComPtr<ID3D11InputLayout>	m_layout;
	ComPtr<ID3D11SamplerState>	m_sampleState;
	ComPtr<ID3D11Buffer>		m_matrixBuffer;
	ComPtr<ID3D11Buffer>		m_lightBuffer;
};