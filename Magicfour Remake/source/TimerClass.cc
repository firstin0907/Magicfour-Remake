#include "../include/TimerClass.hh"

#include "../include/GameException.hh"


TimerClass::TimerClass()
{
    // Get the cycles per second speed for this system.
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency_);
    if (frequency_ == 0) throw GAME_EXCEPTION(
        L"Failed to get performance frequency, needed to initialize TimerClass.");

    frameTime_ = 0;

    // Get the initial start time.
    QueryPerformanceCounter((LARGE_INTEGER*)&startTicks_);
    currTime_ = 0;
}

TimerClass::~TimerClass()
{

}

void TimerClass::Frame()
{
    prevTime_ = currTime_;

    // Query the current time.
    QueryPerformanceCounter((LARGE_INTEGER*)&currTime_);

    currTime_ = (currTime_ - startTicks_) * 1000 / frequency_;

    // Calculate the difference in time since the last time we queried for the current time.
    frameTime_ = currTime_ - prevTime_;

    return;
}