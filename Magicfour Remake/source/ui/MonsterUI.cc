#include "ui/MonsterUI.hh"

#include <DirectXMath.h>

#include "game-object/MonsterClass.hh"
#include "graphics/BitmapClass.hh"
#include "core/D2DClass.hh"
#include "ui/UserInterfaceClass.hh"

using namespace DirectX;

void MonsterUI::DrawUI(D2DClass* direct2D, UserInterfaceClass* ui,
	class IGameObject* obj, time_t curr_time)
{
	MonsterClass* monster = static_cast<MonsterClass*>(obj);

	float screen_x = 0, screen_y = 0;
	ui->CalculateScreenPos(monster->GetLocalWorldMatrix(), screen_x, screen_y);

	if (monster->GetState() == MonsterState::kStopEmbryo)
	{
		DrawWarningVerticalRect(direct2D, ui, screen_x, 50,
			monster->GetStateTime(curr_time) / 700.0f);
	}
	else
	{
		DrawMonsterHp(direct2D, ui, screen_x, screen_y - 23,
			monster->GetHpRatio(), monster->GetPrevHpRatio());
	}
}

void MonsterUI::DrawMonsterHp(class D2DClass* direct2D, UserInterfaceClass* ui,
	int center_x, int top, float hp_ratio, float hp_white_ratio)
{
	const auto hp_gauge_bitmap = ui->GetContext().bitmaps_.get("monster_hp_gauge");

	const float left = center_x - hp_gauge_bitmap->GetWidth() / 2;
	const float right = left + hp_gauge_bitmap->GetWidth();
	const float bottom = top + hp_gauge_bitmap->GetHeight();

	// Draw White Portion
	if (hp_white_ratio >= 0.0f)
	{
		direct2D->SetBrushColor(D2D1::ColorF(D2D1::ColorF::White));
		direct2D->RenderRect(left, (float)top,
			left + hp_gauge_bitmap->GetWidth() * hp_white_ratio,
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
			left + hp_gauge_bitmap->GetWidth() * hp_ratio,
			bottom);
	}

	// Draw HP Frame
	direct2D->RenderBitmap(hp_gauge_bitmap.get(), left, (float)top);
}


void MonsterUI::DrawWarningVerticalRect(D2DClass* direct2D, UserInterfaceClass* ui,
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
		center_x + width / 2, ui->GetContext().f_screen_height_);
}