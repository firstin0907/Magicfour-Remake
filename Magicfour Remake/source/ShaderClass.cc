#include "../include/ShaderClass.hh"

#include <fstream>

#include "../include/GameException.hh"

ShaderClass::ShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context)
	: device_(device), device_context_(device_context)
{
}

void ShaderClass::CreateShaderObject(HWND hwnd,
	const WCHAR* vs_filename, const WCHAR* ps_filename,
	D3D11_INPUT_ELEMENT_DESC polygon_layout[], int num_of_elements)
{
	HRESULT result;
	ComPtr<ID3D10Blob> error_message = nullptr;

	ComPtr<ID3D10Blob> vs_buffer;
	ComPtr<ID3D10Blob> ps_buffer;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vs_filename, NULL, NULL, "vsMain", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		vs_buffer.GetAddressOf(), error_message.GetAddressOf());

	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (error_message) OutputShaderErrorMessage(error_message.Get(), hwnd, vs_filename);
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else MessageBox(hwnd, vs_filename, L"Missing Shader File", MB_OK);

		throw GAME_EXCEPTION(L"Could not initialize the shader object.");
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(ps_filename, NULL, NULL, "psMain", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		ps_buffer.GetAddressOf(), error_message.GetAddressOf());
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (error_message) OutputShaderErrorMessage(error_message.Get(), hwnd, ps_filename);
		// If there was nothing in the error message then it simply could not find the file itself.
		else MessageBox(hwnd, ps_filename, L"Missing Shader File", MB_OK);
		
		throw GAME_EXCEPTION(L"Could not initialize the shader object.");
	}
	
	// Create the vertex shader from the buffer.
	result = device_->CreateVertexShader(vs_buffer->GetBufferPointer(),
		vs_buffer->GetBufferSize(), NULL, vertex_shader_.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Could not initialize the shader object.");

	// Create the pixel shader from the buffer.
	result = device_->CreatePixelShader(ps_buffer->GetBufferPointer(),
		ps_buffer->GetBufferSize(), NULL, pixel_shader_.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Could not initialize the shader object.");

	// Create the vertex input layout.
	result = device_->CreateInputLayout(polygon_layout, num_of_elements,
		vs_buffer->GetBufferPointer(), vs_buffer->GetBufferSize(),
		input_layout_.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Could not initialize the shader object.");
}

ID3D11SamplerState* ShaderClass::CreateSamplerState()
{
	ID3D11SamplerState* sampler_state;
	D3D11_SAMPLER_DESC sampler_desc;

	// Create a texture sampler state description.
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.MaxAnisotropy = 1;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.BorderColor[0] = 0;
	sampler_desc.BorderColor[1] = 0;
	sampler_desc.BorderColor[2] = 0;
	sampler_desc.BorderColor[3] = 0;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = device_->CreateSamplerState(&sampler_desc, &sampler_state);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create sampler state.");

	return sampler_state;
}
;

void ShaderClass::OutputShaderErrorMessage(
	ID3D10Blob* error_message, HWND hwnd, const WCHAR* shader_filename)
{
	char* compileErrors;
	std::ofstream fout("shader-error.txt");

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(error_message->GetBufferPointer());

	// Get the length of the message.
	size_t bufferSize = error_message->GetBufferSize();

	// Write out the error message.
	for (size_t i = 0; i < bufferSize; i++) fout << compileErrors[i];

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shader_filename, MB_OK);

	return;
}
