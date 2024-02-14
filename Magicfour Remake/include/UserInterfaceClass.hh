#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <d2d1.h>
#include <dwrite.h>

#include <DirectXMath.h>

class UserInterfaceClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;
	using MonsterVector = std::vector<std::unique_ptr<class MonsterClass> >;

public:
	UserInterfaceClass(class D2DClass* direct2D, ID3D11Device* device, int screenWidth, int screenHeight,
		const wchar_t* monsterHpFrameFilename, const wchar_t* monsterHpGaugeFilename);

	void InitializeBuffers(ID3D11Device* device);

	void Render(class D2DClass* direct2D,
		class TextureShaderClass* textureShader,
		ID3D11DeviceContext* deviceContext, class CharacterClass* character,
		MonsterVector& monsters,
		const XMMATRIX& vp_matrix, const XMMATRIX& orthoMatrix, time_t curr_time, bool on_paused);

private:
	int screenWidth_, screenHeight_;

	std::unique_ptr<class SkillGaugeClass> skillGauge_;

	ComPtr<struct IDWriteTextFormat> score_text_format_;
	ComPtr<struct IDWriteTextFormat> pause_text_format_;
	ComPtr<struct IDWriteTextFormat> pause_description_format_;

	std::unique_ptr<class TextureClass> monsterHpFrameTexture_;
	std::unique_ptr<class TextureClass> monsterHpGaugeTexture_[4];

	ComPtr<ID3D11Buffer> vertexBuffer_, indexBuffer_;


};