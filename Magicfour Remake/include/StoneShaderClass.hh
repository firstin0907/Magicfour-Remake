#pragma once

#include "../include/ShaderClass.hh"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>

class StoneShaderClass : public ShaderClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

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

		XMFLOAT4 ambient_weight;
		XMFLOAT4 diffuse_weight;
		XMFLOAT4 specular_weight;
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

	void Render(ID3D11DeviceContext* deviceContext,
		class ModelClass* model, XMMATRIX worldMatrix, XMMATRIX vpMatrix,
		XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition);

	void Render(ID3D11DeviceContext* deviceContext, int indexCount,
		XMMATRIX worldMatrix, XMMATRIX vpMatrix,
		XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition);

private:
	void InitializeShader(ID3D11Device* device, HWND hwnd,
		const WCHAR* vsFilename, const WCHAR* psFilename);

	void SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX,
		XMFLOAT3, XMFLOAT4,
		XMFLOAT3 cameraPosition, XMFLOAT3, XMFLOAT3, XMFLOAT3);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ComPtr<ID3D11SamplerState>	m_sampleState;

	ComPtr<ID3D11Buffer>		m_matrixBuffer;
	ComPtr<ID3D11Buffer>		m_lightBuffer;
	ComPtr<ID3D11Buffer>		m_cameraBuffer;
};