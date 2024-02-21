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

	inline INT64 GetElapsedTime()
	{
		return (curr_ticks_ - start_ticks_) * 1'000 / frequency_ -
			(prev_ticks_ - start_ticks_) * 1'000 / frequency_;
	}

	inline INT64 GetTime()
	{
		return (curr_ticks_ - start_ticks_) * 1'000 / frequency_;
	};

private:
	INT64 frequency_;

	INT64 start_ticks_;

	INT64 prev_ticks_;
	INT64 curr_ticks_;

	bool is_paused_, wait_to_resume_;

};