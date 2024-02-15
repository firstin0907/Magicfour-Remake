#include "../include/SkillGaugeClass.hh"
#include "../include/TextureClass.hh"
#include "../include/GameException.hh"

using namespace std;

SkillGaugeClass::SkillGaugeClass(ID3D11Device* device,
	int screenWidth, int screenHeight, const wchar_t* textureFilenameGray,
	const wchar_t* textureFilenameWhite, int renderX, int renderY)
{
	screen_width_ = screenWidth;
	screen_height_ = screenHeight;

	renderX_ = renderX;
	renderY_ = renderY;

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
	return indexCount_;
}

ID3D11ShaderResourceView* SkillGaugeClass::GetTexture(float skill_ratio)
{
	if (skill_ratio > 0.0f) return textureGray_->GetTexture();
	else return textureWhite_->GetTexture();
}

void SkillGaugeClass::InitializeBuffers(ID3D11Device* device)
{
	HRESULT result;
	VertexType* vertices;
	unsigned long* indices;

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Initialize the previous rendering position to negative one.
	prevPosX_ = -1, prevPosY_ = -1;
	prevHeight_ = -(1 << 30);

	// Set the number of vertices in the vertex array.
	vertexCount_ = 4;

	// Set the number of indices in the index array.
	indexCount_ = 6;

	// Create the vertex array.
	vertices = new VertexType[vertexCount_];

	// Create the index array.
	indices = new unsigned long[indexCount_];

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * vertexCount_));

	// Load the index array with data.
	indices[0] = 0, indices[1] = 1, indices[2] = 2;
	indices[3] = 0, indices[4] = 3, indices[5] = 1;

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // !!!!!!!!!!! Dynamic
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount_;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer_.GetAddressOf());
	if (FAILED(result))
	{
		throw GAME_EXCEPTION(L"Failed to create vertex buffer for SkillGaugeClass.");
	}

	// 인덱스 버퍼의 description을 작성합니다.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount_;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer_.GetAddressOf());
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

	float x1 = renderX_, y1 = renderY_;

	float x2 = x1 + (float)bitmapWidth_;
	float y2 = y1 - (float)bitmapHeight_;

	int height = (int)(round(ratio * bitmapHeight_));
	int width = (int)bitmapWidth_;

	if (height == prevHeight_) return true;
	prevHeight_ = height;

	// Create the vertex array.
	VertexType* vertices = new VertexType[vertexCount_];

					// POSITION							// TEXTURE
	vertices[0] = { XMFLOAT3(x1, y2 + height, 0.0f), XMFLOAT2(0.0f, 1 - ratio) };
	vertices[1] = { XMFLOAT3(x2, y2, 0.0f), XMFLOAT2(1.0f, 1.0f) };
	vertices[2] = { XMFLOAT3(x1, y2, 0.0f), XMFLOAT2(0.0f, 1.0f) };
	vertices[3] = { XMFLOAT3(x2, y2 + height, 0.0f), XMFLOAT2(1.0f, 1 - ratio) };

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = deviceContext->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;

	auto dataPtr = (VertexType*)mappedResource.pData;
	memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * vertexCount_));

	deviceContext->Unmap(vertexBuffer_.Get(), 0);

	delete[] vertices;
	return true;
}

void SkillGaugeClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType), offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SkillGaugeClass::LoadTexture(ID3D11Device* device,
	const wchar_t* grayFilename, const wchar_t* whiteFilename )
{
	textureGray_ = make_unique<TextureClass>(device, grayFilename);
	textureWhite_ = make_unique<TextureClass>(device, whiteFilename);

	bitmapWidth_ = textureGray_->GetWidth();
	bitmapHeight_ = textureGray_->GetHeight();
}
