#include "../include/UserInterfaceClass.hh"

#include "../include/TextureClass.hh"
#include "../include/ModelClass.hh"
#include "../include/MonsterClass.hh"
#include "../include/GameException.hh"
#include "../include/TextureShaderClass.hh"
#include "../include/SkillGaugeClass.hh"
#include "../include/CharacterClass.hh"
#include "../include/D2DClass.hh"

using namespace std;
using namespace DirectX;

UserInterfaceClass::UserInterfaceClass(class D2DClass* direct2D,
	ID3D11Device* device, int screenWidth, int screenHeight,
	const wchar_t* monsterHpFrameFilename, const wchar_t* monsterHpGaugeFilename)
	: m_ScreenHeight(screenHeight), m_ScreenWidth(screenWidth)
{
	m_SkillGauge = make_unique<SkillGaugeClass>(device,
		screenWidth, screenHeight, L"data/texture/skill_gauge_gray.png",
		L"data/texture/skill_gauge_white.png", -80 + 40, 180 + 16);
	m_MonsterHpFrameTexture = make_unique<TextureClass>(device, monsterHpFrameFilename);
	m_MonsterHpGaugeTexture[0] = make_unique<TextureClass>(device, L"data/texture/user_interface/hp_gauge_green.png");
	m_MonsterHpGaugeTexture[1] = make_unique<TextureClass>(device, L"data/texture/user_interface/hp_gauge_yellow.png");
	m_MonsterHpGaugeTexture[2] = make_unique<TextureClass>(device, L"data/texture/user_interface/hp_gauge_red.png");
	m_MonsterHpGaugeTexture[3] = make_unique<TextureClass>(device, L"data/texture/user_interface/hp_gauge_white.png");

	InitializeBuffers(device);

	m_ScoreTextFormat = direct2D->CreateTextFormat(L"Arial", 40,
		DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

void UserInterfaceClass::InitializeBuffers(ID3D11Device* device)
{
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	const float w = (float)m_MonsterHpFrameTexture->GetWidth();
	const float h = (float)m_MonsterHpFrameTexture->GetHeight();

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

void UserInterfaceClass::Render(class D2DClass* direct2D, TextureShaderClass* textureShader,
	ID3D11DeviceContext* deviceContext, CharacterClass* character, MonsterVector& monsters,
	const XMMATRIX& vpMatrix, const XMMATRIX& orthoMatrix, time_t curr_time)
{
	const XMMATRIX orthoInverseMatrix = XMMatrixInverse(nullptr, orthoMatrix);

	float skill_ratio = character->GetCooltimeGaugeRatio(curr_time);
	if (skill_ratio > -0.03f)
	{
		// get Character coordinate in viewport coordinate system.
		XMVECTOR t = { 0, 0, 0, 1 };
		t = XMVector4Transform(t, character->GetLocalWorldMatrix() * vpMatrix);
		t = XMVector4Transform(t / t.m128_f32[3], orthoInverseMatrix);

		m_SkillGauge->Render(deviceContext, skill_ratio);
		textureShader->Render(deviceContext,
			m_SkillGauge->GetIndexCount(), XMMatrixTranslationFromVector(t),
			orthoMatrix, m_SkillGauge->GetTexture(skill_ratio));
	}

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	// Set vertex buffer stride and offset.
	unsigned int stride = sizeof(VertexType), offset = 0;

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
		t = XMVector4Transform(t / t.m128_f32[3], orthoInverseMatrix);
		
		// Adjust the position where hp bar will be drawn.
		t.m128_f32[0] -= m_MonsterHpFrameTexture->GetWidth() / 2.0f;
		t.m128_f32[1] += m_MonsterHpFrameTexture->GetHeight();

		const float hp_ratio = monster->GetHpRatio();
		TextureClass* gauge_texture = nullptr;

		if (hp_ratio > 0.5) gauge_texture = m_MonsterHpGaugeTexture[0].get();
		else if (hp_ratio > 0.2) gauge_texture = m_MonsterHpGaugeTexture[1].get();
		else gauge_texture = m_MonsterHpGaugeTexture[2].get();
		
		// Draw hp gauge according to hp of the monster.
		textureShader->Render(deviceContext, 6,
			XMMatrixScaling(monster->GetPrevHpRatio(), 1, 1)* XMMatrixTranslationFromVector(t),
			orthoMatrix, m_MonsterHpGaugeTexture[3]->GetTexture()); // white portion
		textureShader->Render(deviceContext, 6,
			XMMatrixScaling(hp_ratio, 1, 1)* XMMatrixTranslationFromVector(t),
			orthoMatrix, gauge_texture->GetTexture()); // real hp portion

		// Draw hp frame
		textureShader->Render(deviceContext, 6, XMMatrixTranslationFromVector(t),
			orthoMatrix, m_MonsterHpFrameTexture->GetTexture());
	}

	// Direct2D rendering
	direct2D->BeginDraw();

	// Draw Score
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black));
	direct2D->RenderText(m_ScoreTextFormat.Get(), std::to_wstring(character->GetTotalScore(curr_time)).c_str(),
		0, 30.0f, (float)(m_ScreenWidth - 30), 200.0f);

	// Draw Combo
	const int combo = character->GetCombo();
	if (combo > 0)
	{
		// Remained time for combo.
		const time_t combo_durable_time = character->GetComboDurableTime(curr_time);
		const float combo_text_alpha_value = SATURATE(0.0f, (combo_durable_time - 500.0f) * (1 / 3000.0f), 1.0f);


		if (combo < 10) direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black, combo_text_alpha_value));
		else if (combo < 30) direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::DarkBlue, combo_text_alpha_value));
		else direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::DarkRed, combo_text_alpha_value));


		float font_size_1, font_size_2 = 45.0f;
		font_size_1 = 65.0f + max((combo_durable_time - 4800) / 200.0f, 0) * 30.0f;

		if (combo_durable_time < 4970)
			font_size_2 = 45.0f + max((combo_durable_time - 4800) / 200.0f, 0) * 20.0f;

		int font_offset = combo_durable_time > 4800 ? (combo_durable_time - 4800) / 5 : 0;


		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(L"Arial", font_size_1,
				DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_FAR), std::to_wstring(combo).c_str(),
			0, (float)(m_ScreenHeight / 2), (float)(m_ScreenWidth - 190 - font_offset), (float)(m_ScreenHeight / 2));

		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(L"Arial", font_size_2,
				DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_FAR), L"Combo",
			0, (float)(m_ScreenHeight / 2), (float)(m_ScreenWidth - 30), (float)(m_ScreenHeight / 2));

	}

	direct2D->EndDraw();

}
