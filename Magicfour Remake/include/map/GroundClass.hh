#pragma once

#include "core/global.hh"

#define COLLIDE_FROM_LEFT	0b0001
#define COLLIDE_FROM_RIGHT	0b0010
#define COLLIDE_FROM_ABOVE	0b0100
#define COLLIDE_FROM_BELOW	0b1000
#define COLLIDE_FROM_ALL	0b1111

#define ISBETWEEN(X1, X, X2) ((X1) <= (X) && (X) <= (X2))

class GroundClass
{
public:
	GroundClass(const rect_t& range) : range_(range) {};

	inline int IsCollided(int x1, int x2, int from_bottom_coord, int to_bottom_coord) const
	{
		if ((ISBETWEEN(range_.x1, x1, range_.x2) || ISBETWEEN(range_.x1, x2, range_.x2)) &&
			to_bottom_coord <= range_.y2 && range_.y2 <= from_bottom_coord)
		{
			return range_.y2;
		}
		else return to_bottom_coord;
	}

	inline rect_t GetRange() const
	{
		return range_;
	}

	//time_t GetCollisionTime(class RigidBodyClass* rigid);

private:
	rect_t range_;

};