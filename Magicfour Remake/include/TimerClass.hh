#pragma once

#include <windows.h>

class TimerClass
{
public:
	TimerClass();
	TimerClass(TimerClass&) = delete;
	~TimerClass();

	void Frame();
	inline INT64 GetElapsedTime() { return frameTime_; }
	inline INT64 GetTime() { return currTime_; };

private:
	INT64 frequency_;

	INT64 startTicks_;

	INT64 prevTime_;
	INT64 currTime_;

	INT64 frameTime_;

};