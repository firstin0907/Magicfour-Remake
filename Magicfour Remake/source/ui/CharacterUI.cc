#include "ui/CharacterUI.hh"

#include <DirectXMath.h>

#include "game-object/CharacterClass.hh"
#include "graphics/BitmapClass.hh"
#include "core/D2DClass.hh"
#include "ui/UserInterfaceClass.hh"

using namespace DirectX;

void CharacterUI::DrawUI(class D2DClass* direct2D, UserInterfaceClass* ui,
	IGameObject* obj, time_t curr_time)
{
	CharacterClass* character = static_cast<CharacterClass*>(obj);

	float screen_x, screen_y;
	ui->CalculateScreenPos(character->GetLocalWorldMatrix(), screen_x, screen_y);

	DrawScoreAndCombo(direct2D, ui->GetContext(), character, curr_time);
	DrawSkillGauge(direct2D, ui->GetContext(), screen_x, screen_y, character->GetCooltimeGaugeRatio(curr_time));
	DrawInvincibleGauge(direct2D, ui->GetContext(), screen_x, screen_y, character->GetInvincibleGaugeRatio(curr_time));
	DrawSkillBonus(direct2D, ui->GetContext(),
		static_cast<unsigned int>(character->GetSkillBonus()),
		character->GetSkillBonusElapsedTime(curr_time));

	XMMATRIX skill_stone_pos = character->GetSkillStonePos(curr_time);
	// Get the coordinate of stone with respect to screen coordinate.
	for (int i = 0; i < 4; i++)
	{
		ui->CalculateScreenPos(skill_stone_pos * XMMatrixTranslation(0, -0.6f * i, 0), screen_x, screen_y);
		if (character->GetSkill(i).skill_type)
		{
			DrawSkillPower(direct2D,
				character->GetSkill(i).skill_type,
				character->GetSkill(i).skill_power,
				curr_time - character->GetSkill(i).learned_time,
				screen_x, screen_y);
		}
	}

}

void CharacterUI::DrawScoreAndCombo(D2DClass* direct2D, const UIContext& context,
	CharacterClass* character, time_t curr_time)
{
	// Draw Score
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black));
	direct2D->RenderText(context.fonts_.get("score_text_format").get(), std::to_wstring(character->GetScore()).c_str(),
		0, 30.0f, (float)(context.screen_width_ - 30), 200.0f);

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
			0, (float)(context.screen_height_ / 2), (float)(context.screen_width_ - 190 - font_offset), (float)(context.screen_height_ / 2));

		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(L"Arial", font_size_2,
				DWRITE_TEXT_ALIGNMENT_TRAILING, DWRITE_PARAGRAPH_ALIGNMENT_FAR), L"Combo",
			0, (float)(context.screen_height_ / 2), (float)(context.screen_width_ - 30), (float)(context.screen_height_ / 2));
	}
}

void CharacterUI::DrawSkillGauge(D2DClass* direct2D, const UIContext& context,
	float char_screen_x, float char_screen_y, float skill_charge_ratio)
{
	if (skill_charge_ratio < -0.1f) return;

	const float left = char_screen_x - 40;
	const float top = char_screen_y - 196;

	const float right = left + context.bitmaps_.get("skill_gauge_gray")->GetWidth();
	const float bottom = top + context.bitmaps_.get("skill_gauge_gray")->GetHeight();

	if (skill_charge_ratio > 0.0f)
	{
		const float height = context.bitmaps_.get("skill_gauge_gray")->GetHeight() * skill_charge_ratio;

		auto dest = D2D1::RectF(left, bottom - height, right, bottom);
		auto source = D2D1::RectF(0,
			context.bitmaps_.get("skill_gauge_gray")->GetHeight() - height,
			right, context.bitmaps_.get("skill_gauge_gray")->GetHeight());

		direct2D->RenderBitmap(context.bitmaps_.get("skill_gauge_gray").get(), dest, source);
	}
	else
	{
		const float alpha = 1.0f - skill_charge_ratio / -0.1f;
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White, alpha));

		direct2D->RenderRect(left, top, right, bottom);
	}
}

void CharacterUI::DrawInvincibleGauge(D2DClass* direct2D, const UIContext& context,
	float char_screen_x, float char_screen_y, float invincible_ratio)
{
	if (invincible_ratio < -0.1f) return;

	const float left = char_screen_x + 40;
	const float top = char_screen_y - 196;

	const float right = left + context.bitmaps_.get("skill_gauge_rainbow")->GetWidth();
	const float bottom = top + context.bitmaps_.get("skill_gauge_rainbow")->GetHeight();

	if (invincible_ratio > 0.0f)
	{
		const float height = context.bitmaps_.get("skill_gauge_rainbow")->GetHeight() * invincible_ratio;

		auto dest = D2D1::RectF(left, bottom - height, right, bottom);
		auto source = D2D1::RectF(0,
			context.bitmaps_.get("skill_gauge_rainbow")->GetHeight() - height,
			right, context.bitmaps_.get("skill_gauge_rainbow")->GetHeight());

		direct2D->RenderBitmap(context.bitmaps_.get("skill_gauge_rainbow").get(), dest, source);
	}
	else
	{
		const float alpha = 1.0f - invincible_ratio / -0.1f;
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black, alpha));

		direct2D->RenderRect(left, top, right, bottom);
	}
}

void CharacterUI::DrawSkillBonus(D2DClass* direct2D, const UIContext& context,
	unsigned int skill_bonus, time_t learn_elapsed_time)
{
	const wchar_t* skill_text = SkillBonusText(static_cast<SkillBonus>(skill_bonus));

	if (learn_elapsed_time <= 5'000)
	{
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black, (5000 - learn_elapsed_time) / 5000.0f));

		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(L"Arial", 40,
				DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
			skill_text, 0, 0, context.f_screen_width_,
			context.f_screen_height_ - context.f_screen_height_ * (learn_elapsed_time / 8000.0f) / 3);
	}

	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black, 0.5f));
	direct2D->RenderRect(context.f_screen_width_ - 300, context.f_screen_height_ - 27,
		context.f_screen_width_, context.f_screen_height_);

	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White));
	direct2D->RenderTextWithInstantFormat(
		direct2D->CreateTextFormat(L"Arial", 18,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
		skill_text, context.f_screen_width_ - 293, context.f_screen_height_ - 27,
		context.f_screen_width_, context.f_screen_height_);
}

void CharacterUI::DrawSkillPower(D2DClass* direct2D,
	int skill_type, int skill_power, time_t skill_learned_elapsed_time,
	float skill_stone_screen_x, float skill_stone_screen_y)
{
	const int render_x = skill_stone_screen_x - 22;
	const int render_y = skill_stone_screen_y;

	constexpr float kSkillColor[5][3] =
	{
		{0, 0, 0},
		{0.45f, 0.05f, 0.15f},
		{0.10f, 0.40f, 0.05f},
		{0.05f, 0.15f, 0.45f},
		{0.01f, 0.05f, 0.05f}
	};

	direct2D->SetBrushColor(D2D1::ColorF(
		kSkillColor[skill_type][0], kSkillColor[skill_type][1],
		kSkillColor[skill_type][2]));

	if (skill_power == 10)
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

	skill_power = 9;
	constexpr time_t kEffectTime1 = 200;
	if (skill_learned_elapsed_time <= kEffectTime1)
	{
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White, 0.7f));

		const int y_offset = 16 * skill_learned_elapsed_time / kEffectTime1;
		direct2D->RenderRect(render_x - 19, render_y - 20 + y_offset, render_x + 38, render_y - 13 + y_offset);
	}
	else if (skill_power < 9)
	{
		constexpr time_t kEffectTime2 = 350;
		if (skill_learned_elapsed_time <= kEffectTime1 + kEffectTime2)
		{
			skill_learned_elapsed_time -= kEffectTime1;
			direct2D->SetBrushColor(D2D1::ColorF(
				D2D1::ColorF::White, 1.0f - skill_learned_elapsed_time / (float)kEffectTime2));

			direct2D->RenderRect(render_x - 19, render_y - 20, render_x + 38, render_y + 3);
		}
	}
	else
	{
		constexpr time_t kEffectTime2 = 350;
		if (skill_learned_elapsed_time <= kEffectTime1 + kEffectTime2)
		{
			direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White));
			direct2D->RenderRect(render_x - 19, render_y - 20, render_x + 38, render_y + 3);
		}
		else if (skill_learned_elapsed_time <= kEffectTime1 + kEffectTime2 * 2)
		{
			skill_learned_elapsed_time -= kEffectTime1 + kEffectTime2;
			direct2D->SetBrushColor(D2D1::ColorF(
				D2D1::ColorF::White, 1.0f - skill_learned_elapsed_time / (float)kEffectTime2));
			direct2D->RenderRect(render_x - 19, render_y - 20, render_x + 38, render_y + 3);
		}

	}

}
