#include "../include/TimerClass.hh"

#include "../include/GameException.hh"

TimerClass::TimerClass()
{
    // Get the cycles per second speed for this system.
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency_);
    if (frequency_ == 0) throw GAME_EXCEPTION(
        L"Failed to get performance frequency, needed to initialize TimerClass.");

    // Get the initial start time.
    QueryPerformanceCounter((LARGE_INTEGER*)&start_ticks_);
    prev_ticks_ = curr_ticks_ = 0;

    is_paused_ = wait_to_resume_ = false;
}

void TimerClass::Frame()
{
    if (!is_paused_)
    {
        prev_ticks_ = curr_ticks_;

        // Query the current time.
        QueryPerformanceCounter((LARGE_INTEGER*)&curr_ticks_);
    }
    else if (wait_to_resume_)
    {
        // Query the current time.
        QueryPerformanceCounter((LARGE_INTEGER*)&curr_ticks_);

        start_ticks_ += curr_ticks_ - prev_ticks_;
        prev_ticks_ = curr_ticks_;

        wait_to_resume_ = is_paused_ = false;
    }
    return;
}

void TimerClass::Pause()
{
    if (!is_paused_)
    {
        prev_ticks_ = curr_ticks_;
        is_paused_ = true;
    }
}

void TimerClass::Resume()
{
    wait_to_resume_ = true;
}
