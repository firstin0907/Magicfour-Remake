#pragma once

#include <windows.h>

class TimerClass
{
public:
	TimerClass();
	TimerClass(TimerClass&) = delete;
	~TimerClass() = default;

	void Frame();
	void Pause();
	void Resume();

	inline INT64 GetActualTime()
	{
		return curr_ticks_ * 1'000 / frequency_;
	}

	inline INT64 GetActualElapsedTime()
	{
		return curr_ticks_ * 1'000 / frequency_ -
			prev_ticks_ * 1'000 / frequency_;
	}

	inline void SetGameSpeed(INT64 game_speed)
	{
		game_speed_ = game_speed;
		set_speed_timestamp_game_time_ = curr_game_time_;
		set_speed_timestamp_ticks_ = curr_ticks_;
	}

	inline INT64 GetElapsedTime()
	{
		return curr_game_time_ - prev_game_time_;
	}

	inline INT64 GetTime()
	{
		return curr_game_time_;
	};

private:
	INT64 frequency_;

	INT64 game_speed_;
	INT64 curr_game_time_;
	INT64 prev_game_time_;
	INT64 set_speed_timestamp_game_time_;

	INT64 prev_ticks_;
	INT64 curr_ticks_;
	INT64 set_speed_timestamp_ticks_;
};