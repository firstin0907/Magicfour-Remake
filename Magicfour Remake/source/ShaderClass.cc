#include "../include/ShaderClass.hh"

#include "../include/GameException.hh"

ShaderClass::ShaderClass()
{

}

bool ShaderClass::CreateShaderObject(ID3D11Device* device, HWND hwnd,
	const WCHAR* vsFilename, const WCHAR* psFilename,
	D3D11_INPUT_ELEMENT_DESC polygonLayout[], int numElements)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;

	ComPtr<ID3D10Blob> vertexShaderBuffer;
	ComPtr<ID3D10Blob> pixelShaderBuffer;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "vsMain", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		vertexShaderBuffer.GetAddressOf(), &errorMessage);

	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);

		throw GAME_EXCEPTION(L"Could not initialize the shader object.");
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "psMain", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		pixelShaderBuffer.GetAddressOf(), &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		// If there was nothing in the error message then it simply could not find the file itself.
		else MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		
		throw GAME_EXCEPTION(L"Could not initialize the shader object.");
	}
	
	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, m_vertexShader.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Could not initialize the shader object.");

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, m_pixelShader.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Could not initialize the shader object.");

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements,
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		m_layout.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Could not initialize the shader object.");
	return true;
}

ID3D11SamplerState* ShaderClass::CreateSamplerState(ID3D11Device* device)
{
	ID3D11SamplerState* samplerState;
	D3D11_SAMPLER_DESC samplerDesc;

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	bool result = device->CreateSamplerState(&samplerDesc, &samplerState);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create sampler state.");

	return samplerState;
}
;

void ShaderClass::OutputShaderErrorMessage(
	ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned __int64 bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}
