#include "../include/SkillGaugeClass.hh"
#include "../include/TextureClass.hh"
#include "../include/GameException.hh"

using namespace std;

SkillGaugeClass::SkillGaugeClass(ID3D11Device* device,
	int screenWidth, int screenHeight, const wchar_t* textureFilenameGray,
	const wchar_t* textureFilenameWhite, int renderX, int renderY)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_renderX = renderX;
	m_renderY = renderY;

	InitializeBuffers(device);
	LoadTexture(device, textureFilenameGray, textureFilenameWhite);
}

SkillGaugeClass::~SkillGaugeClass()
{
}

void SkillGaugeClass::Render(ID3D11DeviceContext* deviceContext, float ratio)
{
	bool result;

	// Update the buffers if the position of the bitmap has changed from its original position.
	result = UpdateBuffers(deviceContext, ratio);
	if (!result) throw GAME_EXCEPTION(L"Failed to update buffer for bitmap.");

	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
}

int SkillGaugeClass::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* SkillGaugeClass::GetTexture(float skill_ratio)
{
	if (skill_ratio > 0.0f) return m_TextureGray->GetTexture();
	else return m_TextureWhite->GetTexture();
}

void SkillGaugeClass::InitializeBuffers(ID3D11Device* device)
{
	HRESULT result;
	VertexType* vertices;
	unsigned long* indices;

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Initialize the previous rendering position to negative one.
	m_prevPosX = -1, m_prevPosY = -1;
	m_prevHeight = -(1 << 30);

	// Set the number of vertices in the vertex array.
	m_vertexCount = 4;

	// Set the number of indices in the index array.
	m_indexCount = 6;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];

	// Create the index array.
	indices = new unsigned long[m_indexCount];

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	// Load the index array with data.
	indices[0] = 0, indices[1] = 1, indices[2] = 2;
	indices[3] = 0, indices[4] = 3, indices[5] = 1;

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // !!!!!!!!!!! Dynamic
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		throw GAME_EXCEPTION(L"Failed to create vertex buffer for SkillGaugeClass.");
	}

	// 인덱스 버퍼의 description을 작성합니다.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		throw GAME_EXCEPTION(L"Failed to create index buffer for SkillGaugeClass.");
	}
	delete[] vertices;
	delete[] indices;
}


bool SkillGaugeClass::UpdateBuffers(ID3D11DeviceContext* deviceContext, float ratio)
{
	HRESULT result;

	ratio = (ratio < 0) ? 1.0f : ratio;

	float x1 = m_renderX, y1 = m_renderY;

	float x2 = x1 + (float)m_bitmapWidth;
	float y2 = y1 - (float)m_bitmapHeight;

	int height = (int)(round(ratio * m_bitmapHeight));
	int width = (int)m_bitmapWidth;

	if (height == m_prevHeight) return true;
	m_prevHeight = height;

	// Create the vertex array.
	VertexType* vertices = new VertexType[m_vertexCount];

					// POSITION							// TEXTURE
	vertices[0] = { XMFLOAT3(x1, y2 + height, 0.0f), XMFLOAT2(0.0f, 1 - ratio) };
	vertices[1] = { XMFLOAT3(x2, y2, 0.0f), XMFLOAT2(1.0f, 1.0f) };
	vertices[2] = { XMFLOAT3(x1, y2, 0.0f), XMFLOAT2(0.0f, 1.0f) };
	vertices[3] = { XMFLOAT3(x2, y2 + height, 0.0f), XMFLOAT2(1.0f, 1 - ratio) };

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = deviceContext->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;

	auto dataPtr = (VertexType*)mappedResource.pData;
	memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	deviceContext->Unmap(m_vertexBuffer.Get(), 0);

	delete[] vertices;
	return true;
}

void SkillGaugeClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType), offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SkillGaugeClass::LoadTexture(ID3D11Device* device,
	const wchar_t* grayFilename, const wchar_t* whiteFilename )
{
	m_TextureGray = make_unique<TextureClass>(device, grayFilename);
	m_TextureWhite = make_unique<TextureClass>(device, whiteFilename);

	m_bitmapWidth = m_TextureGray->GetWidth();
	m_bitmapHeight = m_TextureGray->GetHeight();
}
