#include "../include/TextureShaderClass.hh"

#include "../include/GameException.hh"

TextureShaderClass::TextureShaderClass(ID3D11Device* device, HWND hwnd)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(device, hwnd, L"shader/texture.vs", L"shader/texture.ps");
}

TextureShaderClass::~TextureShaderClass()
{

}


void TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	XMMATRIX worldMatrix, XMMATRIX vpMatrix, ID3D11ShaderResourceView* texture)
{
	SetShaderParameters(deviceContext, worldMatrix, vpMatrix, texture);
	RenderShader(deviceContext, indexCount);
}

void TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd,
	const WCHAR* vsFilename, const WCHAR* psFilename)
{
	HRESULT result;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;

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
	}

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].AlignedByteOffset = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

	CreateShaderObject(device, hwnd, vsFilename, psFilename, polygonLayout, numElements);

	// Create the texture sampler state.
	m_sampleState = CreateSamplerState(device);

	// cbuffer(constant buffer)
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, m_matrixBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create matrix buffer");
}

void TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX worldMatrix, XMMATRIX vpMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock Matrix Buffer
	// (정기적으로[ex) 프레임마다] 갱신할 애들에게만 Map, Unmap을 쓰고
	// 그게 아니면 UpdateSubresource를 써라(at. 5[texture])
	result = deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
		0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock matrix buffer to set shader parameter.");

	auto dataPtr = static_cast<MatrixBufferType*>(mappedResource.pData);

	dataPtr->mvp = XMMatrixTranspose(worldMatrix * vpMatrix);

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer.Get(), 0);

	// position of cbuffer in shader?
	unsigned int bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, m_matrixBuffer.GetAddressOf());

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
}

void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
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
}