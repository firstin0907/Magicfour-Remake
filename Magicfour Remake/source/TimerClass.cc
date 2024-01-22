#include "../include/TimerClass.hh"

#include "../include/GameException.hh"


TimerClass::TimerClass()
{
    // Get the cycles per second speed for this system.
    QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
    if (m_frequency == 0) throw GAME_EXCEPTION(
        L"Failed to get performance frequency, needed to initialize TimerClass.");

    m_frameTime = 0;

    // Get the initial start time.
    QueryPerformanceCounter((LARGE_INTEGER*)&m_startTicks);
    m_currTime = 0;
}

TimerClass::~TimerClass()
{

}

void TimerClass::Frame()
{
    m_prevTime = m_currTime;

    // Query the current time.
    QueryPerformanceCounter((LARGE_INTEGER*)&m_currTime);

    m_currTime = (m_currTime - m_startTicks) * 1000 / m_frequency;

    // Calculate the difference in time since the last time we queried for the current time.
    m_frameTime = m_currTime - m_prevTime;

    return;
}