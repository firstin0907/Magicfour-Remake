#pragma once

#include <ctime>

#include "core/global.hh"
#include "util/ResourceMap.hh"

struct UIContext
{
	int		screen_width_, screen_height_;
	float	f_screen_width_, f_screen_height_;

	ResourceMap<class BitmapClass>		bitmaps_;
	ResourceMap<class FontClass>		fonts_;

	struct SystemUIContext
	{
		int prev_frame_cnt = 0, frame_cnt = 0;
		time_t prev_frame_time = 0;

		GameState game_state = GameState::kGameRun;
		time_t state_start_time = 0;
	} system_context_;

	UIContext(int screen_width, int screen_height)
		: screen_height_(screen_height), screen_width_(screen_width),
		f_screen_height_(static_cast<float>(screen_height)),
		f_screen_width_(static_cast<float>(screen_width)),
		system_context_{}
	{
	
	}
};