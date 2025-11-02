#pragma once

#include "util/ResourceMap.hh"

class MonsterUI
{
public:
	static void DrawUI(class D2DClass* direct2D, class UserInterfaceClass* ui,
		class IGameObject* obj, time_t curr_time);

private:
	static void DrawWarningVerticalRect(
		D2DClass* direct2D, UserInterfaceClass* ui, float center_x, float width, float progress);

	static void DrawMonsterHp(class D2DClass* direct2D, UserInterfaceClass* ui,
		int center_x, int top, float hp_ratio, float hp_white_ratio);
};