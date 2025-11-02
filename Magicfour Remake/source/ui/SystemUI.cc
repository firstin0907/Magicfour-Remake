#pragma once

#include "ui/SystemUI.hh"

#include "ui/UserInterfaceClass.hh"
#include "core/D2DClass.hh"

void SystemUI::DrawUI(class D2DClass* direct2D,
	UserInterfaceClass* ui, time_t actual_curr_time)
{
	DrawFps(direct2D, ui->GetContext());
	switch (ui->GetContext().system_context_.game_state)
	{
	case GameState::kGamePause:
		DrawPauseMark(direct2D, ui->GetContext());
		break;
	case GameState::kGameOver:
		DrawGameoverScreen(direct2D, ui->GetContext(), actual_curr_time - ui->GetContext().system_context_.state_start_time);
		break;
	}
}

void SystemUI::DrawFps(D2DClass* direct2D, const UIContext& context)
{
	const auto font = context.fonts_.get("fps_text_format").get();
	const std::wstring text = L"fps: " + std::to_wstring(context.system_context_.prev_frame_cnt);

	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black));
	direct2D->RenderText(
		font,
		text.c_str(),
		0, 10.0f, (float)(context.screen_width_ - 30), 45.0f);
}

void SystemUI::DrawPauseMark(D2DClass* direct2D, const UIContext& context)
{
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::DarkRed));
	direct2D->RenderText(context.fonts_.get("pause_text_format").get(), L"<< Paused >>",
		0, 30.0f, context.f_screen_width_, 70.0f);
	direct2D->RenderText(context.fonts_.get("pause_description_format").get(), L"To resume, press R key.",
		0, 70.0f, context.f_screen_width_, 100.0f);
}

void SystemUI::DrawGameoverScreen(D2DClass* direct2D, const UIContext& context, time_t gameover_elapsed_time)
{
	const float blackout_alpha = SATURATE(0.0f, (gameover_elapsed_time - 1000) / 1500.f, 1.0f);
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black, blackout_alpha));
	direct2D->RenderRect(0, 0, context.f_screen_width_, context.f_screen_height_);

	const float text_alpha = SATURATE(0.0f, (gameover_elapsed_time - 2500) / 1500.f, 1.0f);
	direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White, text_alpha));
	direct2D->RenderText(context.fonts_.get("gameover_text_format").get(), L"GAME OVER",
		0, 0, context.f_screen_width_, context.f_screen_height_);
}