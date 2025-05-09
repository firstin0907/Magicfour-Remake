#pragma once

#include <vector>
#include <queue>
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
	UserInterfaceClass(class D2DClass* direct2D,
		ID3D11Device* device, int screen_width, int screen_height);
	~UserInterfaceClass();

	void CalculateScreenPos(const XMMATRIX& mvp_matrix,
		const XMMATRIX& ortho_inv, float& x, float& y);

	void DrawCharacterInfo(class D2DClass* direct2D,
		class CharacterClass* character,
		float char_screen_x, float char_screen_y,
		time_t curr_time);


	void Begin2dDraw(class D2DClass* direct2D);
	void End2dDraw(class D2DClass* direct2D);


	void DrawWarningVerticalRect(
		D2DClass* direct2D, float center_x, float width, float progress);

	void DrawMonsterHp(class D2DClass* direct2D,
		int center_x, int top, float hp_ratio, float hp_white_ratio);
	void DrawSkillPower(D2DClass* direct2D,
		int skill_type, int skill_power, time_t skill_learned_elapsed_time,
		float skill_stone_screen_x,
		float skill_stone_screen_y);

	void DrawFps(D2DClass* direct2D,
		time_t actual_curr_time, time_t actual_time_delta);


	void DrawPauseMark(class D2DClass* direct2D);
	void DrawGameoverScreen(class D2DClass* direct2D,
		time_t gameover_elapsed_time);

private:
	void DrawScoreAndCombo(class D2DClass* direct2D,
		class CharacterClass* character, time_t curr_time);
	void DrawSkillGauge(D2DClass* direct2D,
		float char_screen_x, float char_screen_y,
		float skill_charge_ratio);
	void DrawInvincibleGauge(D2DClass* direct2D,
		float char_screen_x, float char_screen_y,
		float invincible_ratio);
	void DrawSkillBonus(D2DClass* direct2D, unsigned int skill_bonus,
		time_t learn_elapsed_time);

private:
	int screen_width_, screen_height_;
	float f_screen_width_, f_screen_height_;

	std::pair<unsigned int, time_t> bonus_effect_;

	unique_ptr<class BitmapClass> monster_hp_gauge_bitmap_;
	unique_ptr<class BitmapClass> skill_gauge_gray_bitmap_;
	unique_ptr<class BitmapClass> invincible_gauge_bitmap_;

	ComPtr<struct IDWriteTextFormat> score_text_format_;
	ComPtr<struct IDWriteTextFormat> fps_text_format_;
	ComPtr<struct IDWriteTextFormat> pause_text_format_;
	ComPtr<struct IDWriteTextFormat> pause_description_format_;
	ComPtr<struct IDWriteTextFormat> gameover_text_format_;
};