#include "scenes/TitleScene.hh"

#include "core/InputClass.hh"

#include "shader/ShaderManager.hh"

#include "core/common/GraphicResources.hh"
#include "ui/UserInterfaceClass.hh"
#include "graphics/BitmapClass.hh"
#include "core/D2DClass.hh"
#include "core/configuration/ConfigManager.hh"

namespace
{
	constexpr const wchar_t* kMenuItems[] = {
		L"Game Start",
		L"Options",
		L"Statstics(Not Implemented)",
		L"Terminate"
	};
	constexpr size_t kMenuItemCount = sizeof(kMenuItems) / sizeof(const wchar_t*);
}


TitleScene::TitleScene(ConfigManager* config_manager, InputClass* input) :
	config_manager_(config_manager), input_(input), next_scene_name_()
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Frame(time_t scene_time, time_t time_delta, class SoundClass* sound_manager)
{
    next_scene_name_ = "";
	if (input_->IsKeyDown(DIK_RETURN)) // Check if the Enter key is pressed.
	{
		switch (selected_menu_index_)
		{
		case 0: // Start Game
			next_scene_name_ = "GameplayScene"; break;
		case 1: // Settings
			next_scene_name_ = "SettingScene"; break;
		case 2: // Statistics
			break;
		case 3: // Exit Game
			next_scene_name_ = "Exit"; break;
		}
	}

	if (input_->IsKeyDown(DIK_DOWN)) // Check if the Down arrow key is pressed.
	{
		selected_menu_index_ = (selected_menu_index_ + 1) % kMenuItemCount; // Move to the next menu item.
	}
	if (input_->IsKeyDown(DIK_UP)) // Check if the Up arrow key is pressed.
	{
		selected_menu_index_ = (selected_menu_index_ + kMenuItemCount - 1) % kMenuItemCount; // Move to the previous menu item.
	}
}

void TitleScene::Render(time_t scene_time, time_t time_delta, ShaderManager* shader_manager, GraphicResources* graphic_resources)
{


}

void TitleScene::RenderUI(time_t scene_time, time_t time_delta, UserInterfaceClass* user_interface)
{
    user_interface->Begin2dDraw();

    auto* direct2D = user_interface->GetContext().direct2d_;
	auto& ui = user_interface;

	// Draw the background
	{
		auto background_bitmap = ui->GetContext().bitmaps_.get("title_background");
		auto source_rect = background_bitmap->GetEntireRect();
		auto dest_rect = D2D1::RectF(
			0,
			0,
			ui->GetContext().f_screen_width_,
			ui->GetContext().f_screen_height_
		);

		direct2D->RenderBitmap(background_bitmap.get(), dest_rect, source_rect);
	}

	// Draw the title logo
	{
		auto logo_bitmap = ui->GetContext().bitmaps_.get("logo");
		auto source_rect = logo_bitmap->GetEntireRect();

		float dest_height = ui->GetContext().f_screen_height_ * 0.4;
		float dest_width = logo_bitmap->GetWidth() * (dest_height / logo_bitmap->GetHeight());
		auto dest_rect = D2D1::RectF(
			ui->GetContext().f_screen_width_ / 2 - dest_width / 2,
			0,
			ui->GetContext().f_screen_width_ / 2 + dest_width / 2,
			dest_height
		);

		direct2D->RenderBitmap(logo_bitmap.get(), dest_rect, source_rect);
	}

	// Draw Menu
	{
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White));

		const float font_size = ui->GetContext().f_screen_height_ * 0.05f;
		const float font_step = ui->GetContext().f_screen_height_ * 0.12f;

		for (int i = 0; i < kMenuItemCount; ++i)
		{
			float text_width = ui->GetContext().f_screen_width_ * 0.3f;
			float text_left = ui->GetContext().f_screen_width_ / 2 - text_width / 2;
			float text_top = ui->GetContext().f_screen_height_ * 0.9f - (kMenuItemCount - i - 1) * font_step;
			float text_right = text_left + text_width;

			direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black));
			for (int dx : {-2, 0, 2})
			{
				for (int dy : {-2, 0, 2})
				{
					if (dx == 0 && dy == 0) continue;
					direct2D->RenderTextWithInstantFormat(
						direct2D->CreateTextFormat(L"Arial", font_size,
							DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
						kMenuItems[i],
						0 + dx, text_top + dy, ui->GetContext().f_screen_width_ + dx, text_top + dy);
				}
			}

			if(i == selected_menu_index_)
				direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::LightSteelBlue));
			else
				direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White));

			direct2D->RenderTextWithInstantFormat(
				direct2D->CreateTextFormat(L"Arial", font_size,
					DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
				kMenuItems[i],
				0, text_top, ui->GetContext().f_screen_width_, text_top);

		}
	}


    user_interface->End2dDraw();
}

void TitleScene::OnEnter()
{
    // Initialize the title scene when it is entered.
    next_scene_name_.clear();
	selected_menu_index_ = 0;
}

void TitleScene::OnExit()
{
    // Clean up the title scene when it is exited.
    next_scene_name_.clear();
}

const std::string& TitleScene::NextScene() const
{
    return next_scene_name_;
}
