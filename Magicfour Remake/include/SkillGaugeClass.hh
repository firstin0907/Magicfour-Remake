#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include <wrl.h>
#include <memory>

using namespace DirectX;

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class SkillGaugeClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	SkillGaugeClass(ID3D11Device* device,
		int screenWidth, int screenHeight,
		const wchar_t* textureFilenameGray,
		const wchar_t* textureFilenameWhite,
		int renderX, int renderY);
	SkillGaugeClass(const SkillGaugeClass& other) = delete;
	~SkillGaugeClass();

	void Render(ID3D11DeviceContext* deviceContext, float ratio);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture(float skill_ratio);

private:
	void InitializeBuffers(ID3D11Device* device);

	bool UpdateBuffers(ID3D11DeviceContext* deviceContext, float ratio);
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	void LoadTexture(ID3D11Device* device,
		const wchar_t* grayFilename, const wchar_t* whiteFilename);

private:
	ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
	int m_vertexCount, m_indexCount;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_renderX, m_renderY, m_prevPosX, m_prevPosY;
	int m_prevHeight;

	std::unique_ptr<class TextureClass> m_TextureGray;
	std::unique_ptr<class TextureClass> m_TextureWhite;
};