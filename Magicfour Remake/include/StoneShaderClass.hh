#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>

using namespace DirectX;

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class StoneShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX mvp;
		XMMATRIX world_tr_inv;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;  // Added extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

public:
	StoneShaderClass(ID3D11Device* device, HWND hwnd);
	StoneShaderClass(const StoneShaderClass&) = delete;
	~StoneShaderClass();

	bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
		XMMATRIX worldMatrix, XMMATRIX vpMatrix,
		XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition);

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*, const WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMFLOAT3, XMFLOAT4,
		XMFLOAT3 cameraPosition);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ComPtr<ID3D11VertexShader>	m_vertexShader;
	ComPtr<ID3D11PixelShader>	m_pixelShader;
	ComPtr<ID3D11InputLayout>	m_layout;
	ComPtr<ID3D11SamplerState>	m_sampleState;

	ComPtr<ID3D11Buffer>		m_matrixBuffer;
	ComPtr<ID3D11Buffer>		m_lightBuffer;
	ComPtr<ID3D11Buffer>		m_cameraBuffer;
};