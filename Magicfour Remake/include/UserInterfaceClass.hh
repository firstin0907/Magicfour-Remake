#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl.h>

#include <DirectXMath.h>

class UserInterfaceClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;
	using MonsterVector = std::vector<std::unique_ptr<class MonsterClass> >;

public:
	UserInterfaceClass(ID3D11Device* device, int screenWidth, int screenHeight,
		const wchar_t* monsterHpFrameFilename, const wchar_t* monsterHpGaugeFilename);

	void InitializeBuffers(ID3D11Device* device);

	void Render(class TextureShaderClass* textureShader,
		ID3D11DeviceContext* deviceContext, class CharacterClass* character,
		MonsterVector& monsters,
		const XMMATRIX& vpMatrix, const XMMATRIX& orthoMatrix, time_t curr_time);

private:
	std::unique_ptr<class SkillGaugeClass> m_SkillGauge;

	std::unique_ptr<class TextureClass> m_MonsterHpFrameTexture;
	std::unique_ptr<class TextureClass> m_MonsterHpGaugeTexture;

	ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;

};