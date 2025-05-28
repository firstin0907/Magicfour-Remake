#include "util/TimerClass.hh"

#include "core/GameException.hh"

TimerClass::TimerClass()
{
    // Get the cycles per second speed for this system.
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency_);
    if (frequency_ == 0) throw GAME_EXCEPTION(
        L"Failed to get performance frequency, needed to initialize TimerClass.");

    // Get the initial start time.
    QueryPerformanceCounter((LARGE_INTEGER*)&curr_ticks_);
    prev_ticks_ = set_speed_timestamp_ticks_ = curr_ticks_;
    set_speed_timestamp_game_time_ = prev_game_time_ = curr_game_time_ = 0;

    game_speed_ = 1000;
}

void TimerClass::Frame()
{
    prev_ticks_ = curr_ticks_;

    // Query the current time.
    QueryPerformanceCounter((LARGE_INTEGER*)&curr_ticks_);

    prev_game_time_ = curr_game_time_;

    const INT64 elapsed_ticks = curr_ticks_ - set_speed_timestamp_ticks_;

    curr_game_time_ = set_speed_timestamp_game_time_ + elapsed_ticks * game_speed_ / frequency_;

    return;
}

void TimerClass::Pause()
{
    SetGameSpeed(0);
}

void TimerClass::Resume()
{
    SetGameSpeed(1000);
}
