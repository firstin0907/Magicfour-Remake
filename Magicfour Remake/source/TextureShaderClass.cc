#include "../include/TextureShaderClass.hh"

#include "../include/GameException.hh"

TextureShaderClass::TextureShaderClass(ID3D11Device* device, HWND hwnd)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(device, hwnd, L"shader/texture.vs", L"shader/texture.ps");
}


void TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	XMMATRIX world_matrix, XMMATRIX vp_matrix, ID3D11ShaderResourceView* texture)
{
	SetShaderParameters(deviceContext, world_matrix, vp_matrix, texture);
	RenderShader(deviceContext, indexCount);
}

void TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd,
	const WCHAR* vs_filename, const WCHAR* ps_filename)
{
	constexpr int num_of_elements = 2;
	D3D11_INPUT_ELEMENT_DESC polygon_layout[num_of_elements];

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	for (int i = 0; i < num_of_elements; i++)
	{
		polygon_layout[i].SemanticIndex = 0;
		polygon_layout[i].InputSlot = 0;
		polygon_layout[i].InstanceDataStepRate = 0;
		polygon_layout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	}

	polygon_layout[0].SemanticName = "POSITION";
	polygon_layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygon_layout[0].AlignedByteOffset = 0;

	polygon_layout[1].SemanticName = "TEXCOORD";
	polygon_layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygon_layout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

	CreateShaderObject(device, hwnd, vs_filename, ps_filename, polygon_layout, num_of_elements);

	// Create the texture sampler state.
	sample_state_ = CreateSamplerState(device);

	// cbuffer(constant buffer)
	matrix_buffer_ = CreateBasicConstantBuffer<MatrixBufferType>(device);
	if (!matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create matrix buffer");
}

void TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX world_matrix, XMMATRIX vp_matrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock Matrix Buffer
	// (����������[ex) �����Ӹ���] ������ �ֵ鿡�Ը� Map, Unmap�� ����
	// �װ� �ƴϸ� UpdateSubresource�� ���(at. 5[texture])
	result = deviceContext->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
		0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock matrix buffer to set shader parameter.");

	auto dataPtr = static_cast<MatrixBufferType*>(mappedResource.pData);

	dataPtr->mvp = XMMatrixTranspose(world_matrix * vp_matrix);

	// Unlock the constant buffer.
	deviceContext->Unmap(matrix_buffer_.Get(), 0);

	// position of cbuffer in shader?
	unsigned int bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, matrix_buffer_.GetAddressOf());

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
}

void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(input_layout_.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(vertex_shader_.Get(), NULL, 0);
	deviceContext->PSSetShader(pixel_shader_.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}