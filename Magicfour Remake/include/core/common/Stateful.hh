#pragma once

template <typename STATE_TYPE>
class Stateful
{
protected:
    STATE_TYPE prev_state_;
    STATE_TYPE state_;
    time_t state_start_time_;

public:
    inline STATE_TYPE GetState() const
    {
        return state_;
    }

    // Sets variable state_ and state_start_time_.
    // NOTE: NEVER set state_start_time as future. Instead, consider using SetStateIfTimeOver function.
    inline void SetState(STATE_TYPE state, time_t start_time)
    {
        prev_state_ = state_;
        state_ = state;
        state_start_time_ = start_time;
    }

    // Sets variable state_ and state_start_time_
    // if state is sustained as long as 'state_elapsed_time' parameter.
    // Otherwise, does nothing.
    inline void SetStateIfTimeOver(STATE_TYPE state,
        time_t curr_time, time_t state_elapsed_time)
    {
        if (curr_time - state_start_time_ >= state_elapsed_time)
        {
            prev_state_ = state_;
            state_ = state;
            state_start_time_ += state_elapsed_time;
        }
    }

    // Returns how long this instance is on this state.
    inline time_t GetStateTime(time_t curr_time) const
    {
        return curr_time - state_start_time_;
    }
};