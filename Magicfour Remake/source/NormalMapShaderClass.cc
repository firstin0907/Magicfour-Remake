#include "../include/NormalMapShaderClass.hh"

#include <algorithm>
#include <fstream>

#include "../include/GameException.hh"
#include "../include/ModelClass.hh"


NormalMapShaderClass::NormalMapShaderClass(ID3D11Device* device, HWND hwnd)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(device, hwnd, L"shader/normalmap.vs", L"shader/normalmap.ps");

}

NormalMapShaderClass::~NormalMapShaderClass()
{

}


void NormalMapShaderClass::Render(ID3D11DeviceContext* deviceContext,
	ModelClass* model, XMMATRIX worldMatrix, XMMATRIX vpMatrix,
	XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition)
{
	auto& material_list = model->GetMaterial();

	int curr = 0;
	// Set the shader parameters that it will use for rendering.
	for (size_t i = 0; i < material_list.size(); i++)
	{
		SetShaderParameters(deviceContext, worldMatrix, vpMatrix,
			model->GetDiffuseTexture(), model->GetNormalTexture(),
			model->GetEmissiveTexture(),
			lightDirection, diffuseColor, cameraPosition,
			material_list[i].first.ambient,
			material_list[i].first.diffuse,
			material_list[i].first.specular);

		// Set the vertex input layout.
		deviceContext->IASetInputLayout(m_layout.Get());

		// Set the vertex and pixel shaders that will be used to render this triangle.
		deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
		deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

		// Set the sampler state in the pixel shader.
		deviceContext->PSSetSamplers(0, 1, m_sampleState.GetAddressOf());

		// Render the triangle.
		if (i == material_list.size() - 1)
			deviceContext->DrawIndexed(model->GetIndexCount() - material_list[i].second, material_list[i].second, 0);
		else deviceContext->DrawIndexed(material_list[i + 1].second - material_list[i].second, material_list[i].second, 0);
	}
}

void NormalMapShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX vpMatrix,
	ID3D11ShaderResourceView* diffuse_texture,
	ID3D11ShaderResourceView* normal_texture,
	ID3D11ShaderResourceView* emissive_texture,
	XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(deviceContext, worldMatrix, vpMatrix,
		diffuse_texture, normal_texture, emissive_texture, lightDirection, diffuseColor,
		cameraPosition,	{ 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } );

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);
}


void NormalMapShaderClass::InitializeShader(
	ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename)
{
	HRESULT result;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;

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
	D3D11_BUFFER_DESC constantBufferDesc;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;

	constantBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&constantBufferDesc, NULL, m_matrixBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create matrix buffer");

	constantBufferDesc.ByteWidth = sizeof(CameraBufferType);
	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&constantBufferDesc, NULL, m_cameraBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create camera buffer");

	constantBufferDesc.ByteWidth = sizeof(LightBufferType);
	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&constantBufferDesc, NULL, m_lightBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create light buffer");

}


void NormalMapShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX worldMatrix, XMMATRIX vpMatrix,
	ID3D11ShaderResourceView* diffuse_texture,
	ID3D11ShaderResourceView* normal_texture,
	ID3D11ShaderResourceView* emissive_texture,
	XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition,
	XMFLOAT3 ambient_weight, XMFLOAT3 diffuse_weight, XMFLOAT3 specular_weight)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;
	CameraBufferType* dataPtr3;

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock matrix buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	// Transpose the matrices to prepare them for the shader.
	// And Copy them.
	dataPtr->mvp = XMMatrixTranspose(worldMatrix * vpMatrix);
	dataPtr->worldMatrix = XMMatrixTranspose(worldMatrix);
	dataPtr->world_tr_inv = XMMatrixInverse(nullptr, worldMatrix);
	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer.Get(), 0);
	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;
	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, m_matrixBuffer.GetAddressOf());

	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock constant buffer to set shader parameter.");
	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;
	// Copy the lighting variables into the constant buffer.
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;
	dataPtr2->ambient_weight = XMFLOAT4(ambient_weight.x, ambient_weight.y, ambient_weight.z, 1.0f);
	dataPtr2->diffuse_weight = XMFLOAT4(diffuse_weight.x, diffuse_weight.y, diffuse_weight.z, 1.0f);
	dataPtr2->specular_weight = XMFLOAT4(specular_weight.x, specular_weight.y, specular_weight.z, 1.0f);
	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer.Get(), 0);
	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;
	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, m_lightBuffer.GetAddressOf());


	result = deviceContext->Map(m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to camera buffer to set shader parameter.");
	// Get a pointer to the data in the constant buffer.
	dataPtr3 = (CameraBufferType*)mappedResource.pData;
	// Copy the camera position into the constant buffer.
	dataPtr3->cameraPosition = cameraPosition;
	dataPtr3->padding = 0.0f;
	// Unlock the camera constant buffer.
	deviceContext->Unmap(m_cameraBuffer.Get(), 0);
	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 1;
	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, m_cameraBuffer.GetAddressOf());


	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &diffuse_texture);
	deviceContext->PSSetShaderResources(1, 1, &normal_texture);
	deviceContext->PSSetShaderResources(2, 1, &emissive_texture);

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