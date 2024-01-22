#pragma once

#include <windows.h>

class TimerClass
{
public:
	TimerClass();
	TimerClass(TimerClass&) = delete;
	~TimerClass();

	void Frame();
	inline INT64 GetElapsedTime() { return m_frameTime; }
	inline INT64 GetTime() { return m_currTime; };

private:
	INT64 m_frequency;

	INT64 m_startTicks;

	INT64 m_prevTime;
	INT64 m_currTime;

	INT64 m_frameTime;

};