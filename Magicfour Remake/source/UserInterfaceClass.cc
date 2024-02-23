#include "../include/UserInterfaceClass.hh"

#include "../include/TextureClass.hh"
#include "../include/ModelClass.hh"
#include "../include/MonsterClass.hh"
#include "../include/GameException.hh"
#include "../include/TextureShaderClass.hh"
#include "../include/CharacterClass.hh"
#include "../include/D2DClass.hh"

#include "../include/BitmapClass.hh"

using namespace std;
using namespace DirectX;

UserInterfaceClass::UserInterfaceClass(class D2DClass* direct2D,
	ID3D11Device* device, int screen_width, int screen_height)
	: screen_height_(screen_height), screen_width_(screen_width),
	f_screen_height_((float)screen_height), f_screen_width_((float)screen_width)
{
	score_text_format_ = direct2D->CreateTextFormat(L"Arial", 40,
		DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	fps_text_format_ = direct2D->CreateTextFormat(L"Arial", 20,
		DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	pause_text_format_ = direct2D->CreateTextFormat(L"Cambria", 35,
		DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	pause_description_format_ = direct2D->CreateTextFormat(L"Cambria", 20,
		DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	gameover_text_format_ = direct2D->CreateTextFormat(L"Cambria", 70,
		DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	monster_hp_gauge_bitmap_ = make_unique<BitmapClass>(
		direct2D, L"data/texture/user_interface/monster_hp_frame.png"
	);
	skill_gauge_gray_bitmap_ = make_unique<BitmapClass>(
		direct2D, L"data/texture/user_interface/skill_gauge_gray.png"
	);
	invincible_gauge_bitmap_ = make_unique<BitmapClass>(
		direct2D, L"data/texture/user_interface/skill_gauge_rainbow.png"
	);
}

UserInterfaceClass::~UserInterfaceClass()
{
}

void UserInterfaceClass::Render()
{
}

void UserInterfaceClass::CalculateScreenPos(const XMMATRIX& mvp_matrix,
	const XMMATRIX& ortho_inv, float& x, float& y)
{
	XMVECTOR point = { 0, 0, 0, 1 };
	point =	XMVector4Transform(point, mvp_matrix);
	point /= point.m128_f32[3];
	point.m128_f32[2] = 0;
	point = XMVector4Transform(point, ortho_inv);
	
	x = point.m128_f32[0] + screen_width_ / 2.0f;
	y = screen_height_ / 2.0f - point.m128_f32[1];
}

void UserInterfaceClass::DrawWarningVerticalRect(D2DClass* direct2D,
	float center_x, float width, float progress)
{
	if (progress < 0.3f)
	{
		width *= progress / 0.3f;
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Red, 0.3f));
	}
	else if (progress < 0.5f)
	{
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Red, progress));
	}
	else
	{
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Red, 0.5f));
	}

	direct2D->RenderRect(center_x - width / 2, 0,
		center_x + width / 2, (float)screen_height_);
}

void UserInterfaceClass::DrawMonsterHp(D2DClass* direct2D,
	int center_x, int top, float hp_ratio, float hp_white_ratio)
{
	const float left = center_x - monster_hp_gauge_bitmap_->GetWidth() / 2;
	const float right = left + monster_hp_gauge_bitmap_->GetWidth();
	const float bottom = top + monster_hp_gauge_bitmap_->GetHeight();

	// Draw White Portion
	if (hp_white_ratio >= 0.0f)
	{
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White));
		direct2D->RenderRect(left, (float)top,
			left + monster_hp_gauge_bitmap_->GetWidth() * hp_white_ratio,
			bottom);
	}

	// Draw Real HP Portion
	if (hp_ratio >= 0.0f)
	{
		if (hp_ratio >= 0.5f)
			direct2D->SetBrushColor(D2D1::ColorF(0.18f, 1.0f, 0.05f, 1.0f));
		else if (hp_ratio >= 0.2f)
			direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Yellow));
		else
			direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Red));

		direct2D->RenderRect(left, (float)top,
			left + monster_hp_gauge_bitmap_->GetWidth() * hp_ratio,
			bottom);
	}

	// Draw HP Frame
	direct2D->RenderBitmap(monster_hp_gauge_bitmap_.get(), left, (float)top);
}

void UserInterfaceClass::DrawScoreAndCombo(D2DClass* direct2D,
	CharacterClass* character, time_t curr_time)
{
	// Draw Score
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black));
	direct2D->RenderText(score_text_format_.Get(), std::to_wstring(character->GetTotalScore(curr_time)).c_str(),
		0, 30.0f, (float)(screen_width_ - 30), 200.0f);

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
			0, (float)(screen_height_ / 2), (float)(screen_width_ - 190 - font_offset), (float)(screen_height_ / 2));

		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(L"Arial", font_size_2,
				DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_FAR), L"Combo",
			0, (float)(screen_height_ / 2), (float)(screen_width_ - 30), (float)(screen_height_ / 2));
	}
}

void UserInterfaceClass::DrawSkillGauge(D2DClass* direct2D,
	float char_screen_x, float char_screen_y, float skill_charge_ratio)
{
	if (skill_charge_ratio < -0.1f) return;

	const float left = char_screen_x - 40;
	const float top = char_screen_y - 196;

	const float right = left + skill_gauge_gray_bitmap_->GetWidth();
	const float bottom = top + skill_gauge_gray_bitmap_->GetHeight();

	if (skill_charge_ratio > 0.0f)
	{
		const float height = skill_gauge_gray_bitmap_->GetHeight() * skill_charge_ratio;

		auto dest = D2D1::RectF(left, bottom - height, right, bottom);
		auto source = D2D1::RectF(0,
			skill_gauge_gray_bitmap_->GetHeight() - height,
			right, skill_gauge_gray_bitmap_->GetHeight());

		direct2D->RenderBitmap(skill_gauge_gray_bitmap_.get(), dest, source);
	}
	else
	{
		const float alpha = 1.0f - skill_charge_ratio / -0.1f;
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White, alpha));

		direct2D->RenderRect(left, top, right, bottom);
	}
}
void UserInterfaceClass::DrawInvincibleGauge(D2DClass* direct2D,
	float char_screen_x, float char_screen_y, float invincible_ratio)
{
	if (invincible_ratio < -0.1f) return;

	const float left = char_screen_x + 40;
	const float top = char_screen_y - 196;

	const float right = left + invincible_gauge_bitmap_->GetWidth();
	const float bottom = top + invincible_gauge_bitmap_->GetHeight();

	if (invincible_ratio > 0.0f)
	{
		const float height = invincible_gauge_bitmap_->GetHeight() * invincible_ratio;

		auto dest = D2D1::RectF(left, bottom - height, right, bottom);
		auto source = D2D1::RectF(0,
			invincible_gauge_bitmap_->GetHeight() - height,
			right, invincible_gauge_bitmap_->GetHeight());

		direct2D->RenderBitmap(invincible_gauge_bitmap_.get(), dest, source);
	}
	else
	{
		const float alpha = 1.0f - invincible_ratio / -0.1f;
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black, alpha));

		direct2D->RenderRect(left, top, right, bottom);
	}
}

void UserInterfaceClass::DrawFps(D2DClass* direct2D,
	time_t actual_curr_time, time_t actual_time_delta)
{
	static int prev_frame_cnt = 0, frame_cnt = 0;
	if ((actual_curr_time - actual_time_delta) / 1000 < actual_curr_time / 1000)
	{
		prev_frame_cnt = frame_cnt;
		frame_cnt = 0;
	}
	frame_cnt++;

	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black));
	direct2D->RenderText(fps_text_format_.Get(),
		(L"fps: " + std::to_wstring(prev_frame_cnt)).c_str(),
		0, 10.0f, (float)(screen_width_ - 30), 45.0f);
}

void UserInterfaceClass::DrawSkillPower(D2DClass* direct2D,
	int skill_type, int skill_power, float skill_stone_screen_x,
	float skill_stone_screen_y)
{
	const int render_x = skill_stone_screen_x - 22;
	const int render_y = skill_stone_screen_y;

	// for debug
	//direct2D->RenderRect(render_x - 5, render_y - 5, render_x + 5, render_y + 5);

	constexpr float skill_color[5][3] =
	{
		{0, 0, 0},
		{0.45f, 0.05f, 0.15f},
		{0.10f, 0.04f, 0.05f},
		{0.05f, 0.15f, 0.45f},
		{0.01f, 0.05f, 0.05f}
	};

	direct2D->SetBrushColor(D2D1::ColorF(
		skill_color[skill_type][0], skill_color[skill_type][1],
		skill_color[skill_type][2]));

	if(skill_power == 10)
	{

		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(L"Arial", 15,
				DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_FAR),
			L"10", 0, 0, render_x, render_y);
	}
	else
	{
		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(L"Arial", 15,
				DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_FAR),
			(L"0" + std::to_wstring(skill_power)).c_str(),
			0, 0, render_x, render_y);
	}

}

void UserInterfaceClass::DrawPauseMark(class D2DClass* direct2D)
{
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::DarkRed));
	direct2D->RenderText(pause_text_format_.Get(), L"<< Paused >>",
		0, 30.0f, f_screen_width_, 70.0f);
	direct2D->RenderText(pause_description_format_.Get(), L"To resume, press R key.",
		0, 70.0f, f_screen_width_, 100.0f);
}

void UserInterfaceClass::DrawGameoverScreen(D2DClass* direct2D, time_t gameover_elapsed_time)
{
	const float blackout_alpha = SATURATE(0.0f, (gameover_elapsed_time - 2000) / 3000.0f, 1.0f);
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black, blackout_alpha));
	direct2D->RenderRect(0, 0, f_screen_width_, f_screen_height_);

	const float text_alpha = SATURATE(0.0f, (gameover_elapsed_time - 5000) / 3000.0f, 1.0f);
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White, text_alpha));
	direct2D->RenderText(gameover_text_format_.Get(), L"GAME OVER",
		0, 0, f_screen_width_, f_screen_height_);
}


void UserInterfaceClass::Begin2dDraw(D2DClass* direct2D)
{
	direct2D->BeginDraw();
}

void UserInterfaceClass::End2dDraw(D2DClass* direct2D)
{
	direct2D->EndDraw();
}