#include "scenes/SettingScene.hh"

#include "core/InputClass.hh"

#include "shader/ShaderManager.hh"

#include "core/common/GraphicResources.hh"
#include "ui/UserInterfaceClass.hh"
#include "graphics/BitmapClass.hh"
#include "core/D2DClass.hh"
#include "core/configuration/ConfigDefinitions.hh"
#include "core/configuration/ConfigManager.hh"

#include <initializer_list>

using Config::ConfigMap;
using Config::MenuOption;
using Config::kSettingOption;
using Config::kSettingCount;

namespace
{
	constexpr int kBackToTitleIndex = Config::kSettingCount;
	constexpr int kSettingsRow = Config::kSettingCount + 1;
}


SettingScene::SettingScene(ConfigManager* config_manager, InputClass* input) :
	config_manager_(config_manager), input_(input), next_scene_name_()
{
	setting_values_ = std::vector<size_t>(kSettingsRow, 0);
}

SettingScene::~SettingScene()
{
}

void SettingScene::Frame(time_t scene_time, time_t time_delta, class SoundClass* sound_manager)
{
	next_scene_name_ = "";

	if (input_->IsKeyDown(DIK_RETURN)) // Check if the Enter key is pressed.
	{
		if (selected_menu_index_ == kBackToTitleIndex) // If the "Back" option is selected.
		{
			next_scene_name_ = "TitleScene"; // Transition to InitScene after rendering the title scene.
			return;
		}
	}

	if (input_->IsKeyDown(DIK_DOWN)) // Check if the Down arrow key is pressed.
	{
		selected_menu_index_ = (selected_menu_index_ + 1) % kSettingsRow; // Move to the next menu item.
	}
	if (input_->IsKeyDown(DIK_UP)) // Check if the Up arrow key is pressed.
	{
		selected_menu_index_ = (selected_menu_index_ + kSettingsRow - 1) % kSettingsRow; // Move to the previous menu item.
	}

	if (input_->IsKeyDown(DIK_LEFT) || input_->IsKeyDown(DIK_RIGHT))
	{
		if (!kSettingOption[selected_menu_index_].second.empty())
		{
			if (input_->IsKeyDown(DIK_LEFT))
				setting_values_[selected_menu_index_] =
				(setting_values_[selected_menu_index_] + kSettingOption[selected_menu_index_].second.size() - 1)
				% kSettingOption[selected_menu_index_].second.size();

			if (input_->IsKeyDown(DIK_RIGHT))
				setting_values_[selected_menu_index_] =
				(setting_values_[selected_menu_index_] + 1) % kSettingOption[selected_menu_index_].second.size();

			setting_changed_ = true;
		}
	}
}

void SettingScene::Render(time_t scene_time, time_t time_delta, ShaderManager* shader_manager, GraphicResources* graphic_resources)
{

}

void SettingScene::RenderUI(time_t scene_time, time_t time_delta,
	UserInterfaceClass* user_interface)
{
	user_interface->Begin2dDraw();

	auto& ctx = user_interface->GetContext();
	auto* direct2D = ctx.direct2d_;

	const float screen_width = ctx.f_screen_width_;
	const float screen_height = ctx.f_screen_height_;

	//
	// Background
	//
	{
		auto background = ctx.bitmaps_.get("title_background");

		direct2D->RenderBitmap(
			background.get(),
			D2D1::RectF(
				0,
				0,
				screen_width,
				screen_height),
			background->GetEntireRect());
	}


	//
	// Title
	//
	{
		direct2D->SetBrushColor(
			D2D1::ColorF(D2D1::ColorF::White));

		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(
				L"Arial",
				screen_height * 0.07f,
				DWRITE_TEXT_ALIGNMENT_CENTER,
				DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
			L"Settings",
			0,
			screen_height * 0.05f,
			screen_width,
			screen_height * 0.05f);
	}


	const float start_y = screen_height * 0.28f;
	const float line_height = screen_height * 0.1f;

	const float name_left = screen_width * 0.25f;
	const float value_left = screen_width * 0.6f;

	const float font_size = screen_height * 0.045f;


	auto DrawText =
		[&](const wchar_t* text,
			float left,
			float top,
			D2D1_COLOR_F color)
		{
			for(int dx : {-1, 1, 0})
				for(int dy : {-1, 1, 0})
				{
					if (dx == 0 && dy == 0) direct2D->SetBrushColor(color);
					else direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::Black));

					direct2D->RenderTextWithInstantFormat(
						direct2D->CreateTextFormat(
							L"Arial",
							font_size,
							DWRITE_TEXT_ALIGNMENT_CENTER,
							DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
						text,
						left - screen_width * 0.25f + dx,
						top + dy,
						left + screen_width * 0.25f + dx,
						top + dy);
				}
		};


	//
	// Settings
	//
	for (int i = 0; i < kBackToTitleIndex; ++i)
	{
		float y = start_y + line_height * i;

		D2D1_COLOR_F color = (i == selected_menu_index_) ?
			D2D1::ColorF(D2D1::ColorF::LightSteelBlue) :
			D2D1::ColorF(D2D1::ColorF::White);
		
			DrawText(
				kSettingOption[i].first,
				screen_width * 0.3f,
				y,
				color);

			DrawText(
				kSettingOption[i].second[setting_values_[i]],
				screen_width * 0.7f,
				y,
				color);
	}

	DrawText(
		L"Back to Title",
		screen_width * 0.5f,
		start_y + line_height * kBackToTitleIndex,
		(kBackToTitleIndex == selected_menu_index_) ?
			D2D1::ColorF(D2D1::ColorF::LightSteelBlue) :
			D2D1::ColorF(D2D1::ColorF::White));


	//
	// Restart message
	//
	if (setting_changed_)
	{
		direct2D->SetBrushColor(
			D2D1::ColorF(D2D1::ColorF::Gold));


		direct2D->RenderTextWithInstantFormat(
			direct2D->CreateTextFormat(
				L"Arial",
				screen_height * 0.032f,
				DWRITE_TEXT_ALIGNMENT_CENTER,
				DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
			L"You need to restart the game to apply modified settings.",
			0,
			screen_height * 0.82f,
			screen_width,
			screen_height * 0.82f);
	}


	//
	// Control guide
	//
	direct2D->SetBrushColor(
		D2D1::ColorF(D2D1::ColorF::LightGray));


	direct2D->RenderTextWithInstantFormat(
		direct2D->CreateTextFormat(
			L"Arial",
			screen_height * 0.025f,
			DWRITE_TEXT_ALIGNMENT_CENTER,
			DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
		L"Control: [Left Arrow], [Right Arrow], [Up Arrow], [Down Arrow], [Enter]",
		0,
		screen_height * 0.92f,
		screen_width,
		screen_height * 0.92f);


	user_interface->End2dDraw();
}

void SettingScene::OnEnter()
{
	// Initialize the title scene when it is entered.
	next_scene_name_.clear();

	for (size_t i = 0; i < kBackToTitleIndex; i++)
	{
		std::wstring value = config_manager_->GetConfigValue<std::wstring>(kSettingOption[i].first);
		for (size_t j = 0; j < kSettingOption[i].second.size(); j++)
		{
			if (value == kSettingOption[i].second[j]) setting_values_[i] = j;
		}
	}
}

void SettingScene::OnExit()
{
	// Clean up the title scene when it is exited.
	next_scene_name_.clear();

	ConfigMap config_map;
	for (size_t i = 0; i < kBackToTitleIndex; i++)
	{
		config_map[kSettingOption[i].first] = kSettingOption[i].second[setting_values_[i]];
	}

	config_manager_->SaveConfigToFile(config_map);

}

const std::string& SettingScene::NextScene() const
{
	return next_scene_name_;
}
