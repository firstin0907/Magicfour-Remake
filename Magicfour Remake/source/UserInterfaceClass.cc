#include "../include/UserInterfaceClass.hh"

#include "../include/TextureClass.hh"
#include "../include/ModelClass.hh"
#include "../include/MonsterClass.hh"
#include "../include/GameException.hh"
#include "../include/TextureShaderClass.hh"

using namespace std;
using namespace DirectX;

UserInterfaceClass::UserInterfaceClass(ID3D11Device* device,
	const wchar_t* monsterHpFrameFilename, const wchar_t* monsterHpGaugeFilename)
{
	m_MonsterHpFrameTexture = make_unique<TextureClass>(device, monsterHpFrameFilename);
	m_MonsterHpGaugeTexture = make_unique<TextureClass>(device, monsterHpGaugeFilename);

	InitializeBuffers(device);
}

void UserInterfaceClass::InitializeBuffers(ID3D11Device* device)
{
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	const int w = m_MonsterHpFrameTexture->GetWidth();
	const int h = m_MonsterHpFrameTexture->GetHeight();

	VertexType vertices[4] = {
		{ XMFLOAT3(0, 0, 0.0f), XMFLOAT2(0.0f, 1.0f)},
		{ XMFLOAT3(0, h, 0.0f), XMFLOAT2(0.0f, 0.0f)},
		{ XMFLOAT3(w, h, 0.0f), XMFLOAT2(1.0f, 0.0f)},
		{ XMFLOAT3(w, 0, 0.0f), XMFLOAT2(1.0f, 1.0f)}
	};
	unsigned long indices[6] = { 0, 1, 2, 0, 2, 3 };

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create vertex buffer.");

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * 6;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create index buffer.");
}

void UserInterfaceClass::Render(TextureShaderClass* textureShader,
	ID3D11DeviceContext* deviceContext, MonsterVector& monsters,
	const XMMATRIX& vpMatrix, const XMMATRIX& orthoMatrix, const XMMATRIX& orthoInverseMatrix)
{
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	// Set vertex buffer stride and offset.
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& monster : monsters)
	{
		// Calculate the coordinate of monster, with repect to window(screen) coordinate system.
		XMVECTOR t = { 0, 0, 0, 1 };
		t = XMVector4Transform(t, monster->GetLocalWorldMatrix() * vpMatrix);
		t /= t.m128_f32[3];
		t = XMVector4Transform(t, orthoInverseMatrix);
		
		// Adjust the position where hp bar will be drawn.
		t.m128_f32[0] -= m_MonsterHpFrameTexture->GetWidth() / 2.0f;
		t.m128_f32[1] += m_MonsterHpFrameTexture->GetHeight();

		// Draw hp frame
		bool result = textureShader->Render(deviceContext, 6, XMMatrixTranslationFromVector(t),
			XMMatrixIdentity(), orthoMatrix, m_MonsterHpFrameTexture->GetTexture());
		if (!result) throw GAME_EXCEPTION(L"Failed to draw monster hp bar.");

		// Draw hp gauge according to hp of the monster.
		result = textureShader->Render(deviceContext, 6,
			XMMatrixScaling(monster->GetHpRatio(), 1, 1) * XMMatrixTranslationFromVector(t),
			XMMatrixIdentity(), orthoMatrix, m_MonsterHpGaugeTexture->GetTexture());
		if (!result) throw GAME_EXCEPTION(L"Failed to draw monster hp bar.");
	}
}
