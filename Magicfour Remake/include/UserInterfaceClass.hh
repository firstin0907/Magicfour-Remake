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

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

	using XMMATRIX = DirectX::XMMATRIX;

public:
	UserInterfaceClass(class D2DClass* direct2D, ID3D11Device* device, int screenWidth, int screenHeight,
		const wchar_t* monsterHpFrameFilename, const wchar_t* monsterHpGaugeFilename);
	~UserInterfaceClass();

	void InitializeBuffers(ID3D11Device* device);

	void Render(class D2DClass* direct2D,
		class TextureShaderClass* textureShader,
		ID3D11DeviceContext* deviceContext, class CharacterClass* character,
		const XMMATRIX& vp_matrix, const XMMATRIX& orthoMatrix, time_t curr_time);

	void CalculateScreenPos(const XMMATRIX& mvp_matrix,
		const XMMATRIX& ortho_inv, float& x, float& y);

	void DrawMonsterHp(class D2DClass* direct2D,
		int center_x, int top, float hp_ratio, float hp_white_ratio);
	void DrawScoreAndCombo(class D2DClass* direct2D,
		class CharacterClass* character, time_t curr_time);
	void DrawPauseMark(class D2DClass* direct2D);
	void DrawGameoverScreen(class D2DClass* direct2D,
		time_t gameover_elapsed_time);

	void Begin2dDraw(class D2DClass* direct2D);
	void End2dDraw(class D2DClass* direct2D);

private:
	int screen_width_, screen_height_;

	unique_ptr<class SkillGaugeClass> skillGauge_;
	unique_ptr<class BitmapClass> monster_hp_gauge_bitmap_;

	ComPtr<struct IDWriteTextFormat> score_text_format_;
	ComPtr<struct IDWriteTextFormat> pause_text_format_;
	ComPtr<struct IDWriteTextFormat> pause_description_format_;
	ComPtr<struct IDWriteTextFormat> gameover_text_format_;
};