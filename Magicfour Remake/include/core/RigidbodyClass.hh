#pragma once

#include "core/global.hh"
#include "IGameObject.hh"

template <typename STATE_TYPE>
class RigidbodyClass : public IGameObject
{
public:
	struct Point2d
	{
		int x, y;
		Point2d() : x(0), y(0) {};
		Point2d(int x, int y) : x(x), y(y) {};

		Point2d operator+(const Point2d& rhs) const { return Point2d(x + rhs.x, y + rhs.y); }
		Point2d operator-(const Point2d& rhs) const { return Point2d(x - rhs.x, y - rhs.y); }
		Point2d operator*(int scalar) const { return Point2d(x * scalar, y * scalar); }
		Point2d operator/(int scalar) const { return Point2d(x / scalar, y / scalar); }

		Point2d& operator+=(const Point2d& rhs)
		{
			x += rhs.x, y += rhs.y; return *this;
		}
	};
	using Vector2d = Point2d;

	using XMMATRIX = DirectX::XMMATRIX;

public:
	RigidbodyClass(Point2d position, rect_t range,
		direction_t direction, Vector2d velocity = {0, 0}, Vector2d accel = {0, -kGravity})
		: position_(position), range_(range),
		direction_(direction), velocity_(velocity), accel_(accel) {};

	inline XMMATRIX GetLocalWorldMatrix() const
	{
		return DirectX::XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0);
	}

	virtual rect_t GetGlobalRange() const override final
	{
		return range_.add(position_.x, position_.y);
	}

	inline XMMATRIX GetRangeRepresentMatrix() const
	{
		return range_.add(position_.x, position_.y).toMatrix();
	}

	// Returns position_ field.
	inline Point2d GetPosition() const { return position_; }
	inline Point2d GetVelocity() const { return velocity_; }
	inline Point2d GetAccel() const { return accel_; }

	inline STATE_TYPE GetState() const
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
	inline time_t GetStateTime(time_t curr_time) const
	{
		return curr_time - state_start_time_;
	}

	// Returns which position this instance locates after 'time_delta' milliseconds goes by
	// based on current position and velocity.
	inline Point2d GetPositionAfterMove(time_t time_delta) const
	{
		return position_ + (velocity_ - accel_ * time_delta / 2) * time_delta;
	}

protected:
	Point2d			position_;
	Vector2d		velocity_;
	Vector2d		accel_;

	rect_t			range_;
	direction_t		direction_;

	STATE_TYPE		state_;
	time_t			state_start_time_;
};