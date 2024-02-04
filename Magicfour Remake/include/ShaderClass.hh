#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include <wrl.h>

class ShaderClass
{
public:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	ShaderClass();

protected:
	bool CreateShaderObject(ID3D11Device* device, HWND hwnd,
		const WCHAR* vsFilename, const WCHAR* psFilename,
		D3D11_INPUT_ELEMENT_DESC polygonLayout[], int numElements);

	ID3D11SamplerState* CreateSamplerState(ID3D11Device* device);

	void OutputShaderErrorMessage(ID3D10Blob* errorMessage,
		HWND hwnd, const WCHAR* shaderFilename);

protected:
	ComPtr<ID3D11VertexShader>	m_vertexShader;
	ComPtr<ID3D11PixelShader>	m_pixelShader;
	ComPtr<ID3D11InputLayout>	m_layout;
};
