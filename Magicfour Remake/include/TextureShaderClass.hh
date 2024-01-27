#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>

using namespace DirectX;

class TextureShaderClass
{
private:
	// cbuffer
	struct MatrixBufferType
	{
		XMMATRIX M;
		XMMATRIX V;
		XMMATRIX P;
	};

public:
	TextureShaderClass(ID3D11Device* device, HWND hwnd);
	TextureShaderClass(const TextureShaderClass& other) = delete;
	~TextureShaderClass();

	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
		XMMATRIX M, XMMATRIX V, XMMATRIX P, ID3D11ShaderResourceView* texture);

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd,
		const WCHAR* vertexShader, const WCHAR* pixelShader);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* blob, HWND hwnd,
		const WCHAR* message);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
		XMMATRIX M, XMMATRIX V, XMMATRIX P, ID3D11ShaderResourceView* texture);
	void RenderShader(ID3D11DeviceContext* deviceContext,
		int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11SamplerState* m_sampleState;
};

