#pragma once

#include "util/ResourceMap.hh"

class SystemUI
{
public:
	static void DrawUI(class D2DClass* direct2D,
		class UserInterfaceClass* ui, time_t actual_curr_time);

private:
	static void DrawFps(D2DClass* direct2D, const struct UIContext& context);
	static void DrawPauseMark(class D2DClass* direct2D, const struct UIContext& context);
	static void DrawGameoverScreen(class D2DClass* direct2D, const struct UIContext& context,
		time_t gameover_elapsed_time);
};
