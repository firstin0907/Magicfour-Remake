#pragma once

#include "global.hh"

#define COLLIDE_FROM_LEFT	0b0001
#define COLLIDE_FROM_RIGHT	0b0010
#define COLLIDE_FROM_ABOVE	0b0100
#define COLLIDE_FROM_BELOW	0b1000
#define COLLIDE_FROM_ALL	0b1111

#define ISBETWEEN(X1, X, X2) ((X1) <= (X) && (X) <= (X2))

class GroundClass
{
public:
	GroundClass(const rect_t& range) : m_Range(range) {};

	inline int IsColiided(int x1, int x2, int from_bottom_coord, int to_bottom_coord)
	{
		if ((ISBETWEEN(m_Range.x1, x1, m_Range.x2) || ISBETWEEN(m_Range.x1, x2, m_Range.x2)) &&
			to_bottom_coord <= m_Range.y2 && m_Range.y2 <= from_bottom_coord)
		{
			return m_Range.y2;
		}
		else return to_bottom_coord;
	}

	inline rect_t GetRange()
	{
		return m_Range;
	}

private:
	rect_t m_Range;

};