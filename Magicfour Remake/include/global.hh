#pragma once

#include <DirectXMath.h>

using namespace DirectX;

#ifndef DIRECTION_T
#define DIRECTION_T
enum direction_t
{
	LEFT_FORWARD,
	RIGHT_FORWARD
};

#define DIR_WEIGHT(DIR, VAL) ((DIR == LEFT_FORWARD) ? -(VAL) : (VAL))

#endif

#ifndef RECT_T
#define RECT_T
struct rect_t
{
	int x1, y1; // left top
	int x2, y2; // right bottom

	int get_w() const { return x2 - x1; }
	int get_h() const { return y2 - y1; }

	XMMATRIX toMatrix() const
	{
		return XMMatrixScaling(get_w() / 2.0f * 0.00001f, get_h() / 2.0f * 0.00001f, 1.0f) *
			XMMatrixTranslation((x1 + get_w() / 2.0f) * 0.00001f, (y1 + get_h() / 2.0f) * 0.00001f, 0);
	}

	bool collide(const rect_t& rhs) const
	{
		if ((rhs.x1 < x1 && rhs.x2 < x1) || (x2 < rhs.x1 && x2 < rhs.x2)) return false;
		if ((rhs.y1 < y1 && rhs.y2 < y1) || (y2 < rhs.y1 && y2 < rhs.y2)) return false;
		return true;
	}

	rect_t add(int x, int y) const
	{
		return { x1 + x , y1 + y, x2 + x, y2 + y };
	}
};
#endif

constexpr int GROUND_Y = -600'000;

constexpr int RIGHT_X = 2'500'000;
constexpr int LEFT_X = -2'500'000;

constexpr int SPAWN_RIGHT_X = RIGHT_X + 1'000'000;
constexpr int SPAWN_LEFT_X = LEFT_X - 1'000'000;

constexpr float SCOPE = 0.00001f;
constexpr int GRAVITY = 10;

#include <algorithm>

#define SATURATE(MINIMUM, VAL, MAXIMUM) min(max(VAL, MINIMUM), MAXIMUM)