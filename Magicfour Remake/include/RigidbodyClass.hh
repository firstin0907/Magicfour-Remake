#pragma once

#include "global.hh"

template <typename STATE_TYPE>
class RigidbodyClass
{
public:
	struct Point2d
	{
		int x, y;
		Point2d() : x(0), y(0) {};
		Point2d(int x, int y) : x(x), y(y) {};
	};
	using Vector2d = Point2d;

	using XMMATRIX = DirectX::XMMATRIX;

public:
	RigidbodyClass(Point2d position, rect_t range,
		direction_t direction, Vector2d velocity = {0, 0})
		: position_(position), range_(range),
		direction_(direction), velocity_(velocity) {};

	inline XMMATRIX GetLocalWorldMatrix()
	{
		return XMMatrixTranslation(position_.x * SCOPE, position_.y * SCOPE, 0);
	}

	inline rect_t GetGlobalRange()
	{
		return range_.add(position_.x, position_.y);
	}

	inline XMMATRIX GetRangeRepresentMatrix()
	{
		return range_.add(position_.x, position_.y).toMatrix();
	}

	// Returns position_ field.
	inline Point2d GetPosition()
	{
		return position_;
	}

	inline STATE_TYPE GetState()
	{
		return state_;
	}

	// Sets variable state_ and state_start_time_.
	// NOTE: NEVER set state_start_time as future.
	inline void SetState(STATE_TYPE state, time_t start_time)
	{
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
			state_ = state;
			state_start_time_ += state_elapsed_time;
		}
	}

	// Returns how long this instance is on this state.
	inline time_t GetStateTime(time_t curr_time)
	{
		return curr_time - state_start_time_;
	}

protected:
	Point2d			position_;
	Vector2d		velocity_;
	rect_t			range_;
	direction_t		direction_;

	STATE_TYPE		state_;
	time_t			state_start_time_;
};