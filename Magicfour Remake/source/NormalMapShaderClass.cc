#include "../include/NormalMapShaderClass.hh"

#include <algorithm>
#include <fstream>

#include "../include/GameException.hh"


NormalMapShaderClass::NormalMapShaderClass(ID3D11Device* device, HWND hwnd)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(device, hwnd, L"shader/normalmap.vs", L"shader/normalmap.ps");

}

NormalMapShaderClass::~NormalMapShaderClass()
{

}


void NormalMapShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX vpMatrix,
	ID3D11ShaderResourceView* diffuse_texture, ID3D11ShaderResourceView* normal_texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(deviceContext, worldMatrix, vpMatrix,
		diffuse_texture, normal_texture, lightDirection, diffuseColor);

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);
}


void NormalMapShaderClass::InitializeShader(
	ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename)
{
	HRESULT result;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;


	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	for (int i = 0; i < numElements; i++)
	{
		polygonLayout[i].SemanticIndex = 0;
		polygonLayout[i].InputSlot = 0;
		polygonLayout[i].InstanceDataStepRate = 0;
		polygonLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	}
	polygonLayout[0].AlignedByteOffset = 0;

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	polygonLayout[3].SemanticName = "TANGENT";
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	polygonLayout[4].SemanticName = "BINORMAL";
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	CreateShaderObject(device, hwnd, vsFilename, psFilename, polygonLayout, numElements);

	// Create the texture sampler state.
	m_sampleState = CreateSamplerState(device);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, m_matrixBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create matrix buffer");


	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, m_lightBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create light buffer");
}


void NormalMapShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX vpMatrix,
	ID3D11ShaderResourceView* diffuse_texture, ID3D11ShaderResourceView* normal_texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock matrix buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Transpose the matrices to prepare them for the shader.
	// And Copy them.
	dataPtr->mvp = XMMatrixTranspose(worldMatrix * vpMatrix);
	dataPtr->world_tr_inv = XMMatrixInverse(nullptr, worldMatrix);

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer.Get(), 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, m_matrixBuffer.GetAddressOf());

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &diffuse_texture);
	deviceContext->PSSetShaderResources(1, 1, &normal_texture);

	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock constant buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer.Get(), 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, m_lightBuffer.GetAddressOf());
}


void NormalMapShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, m_sampleState.GetAddressOf());

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}