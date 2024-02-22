#include "../include/ItemClass.hh"

#include <algorithm>

#include "../include/GroundClass.hh"

using namespace std;
using namespace DirectX;

constexpr rect_t ITEM_RANGE = { -30000, -10000, 30000, 60000 };
constexpr time_t ITEM_LIFETIME = 10'000;

ItemClass::ItemClass(time_t create_time, int x_pos, int y_pos, int type)
	: RigidbodyClass<ItemState>(
		Point2d {x_pos, y_pos}, rect_t(ITEM_RANGE),
		LEFT_FORWARD, Vector2d(0, 1000)
	), createTime_(create_time), type_(type)
{
	SetState(ItemState::kNormal, create_time);
}

void ItemClass::FrameMove(time_t curr_time, time_t time_delta,
	const GroundVector& ground)
{
	const int before_vy = velocity_.y, after_vy = velocity_.y - kGravity * time_delta;

	if (after_vy >= 0) position_.y += (before_vy + after_vy) / 2 * time_delta;	
	else if (before_vy > 0)
	{
		const int max_y = position_.y + before_vy / 2 * before_vy / kGravity - ITEM_RANGE.y1;
		const int target = position_.y + (before_vy + after_vy) / 2 * time_delta - ITEM_RANGE.y1;

		position_.y = target;
		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y, ground_obj.IsCollided(ITEM_RANGE.x1 + position_.x,
					ITEM_RANGE.x2 + position_.x, max_y, position_.y));
		}

		// For the case item is collided with the ground, it should stop.
		if (position_.y != target)
		{
			velocity_.y = 0; // it should stop.
		}
		position_.y += ITEM_RANGE.y1;
	}
	else
	{
		const int max_y = position_.y - ITEM_RANGE.y1;;
		const int target = position_.y + (before_vy + after_vy) / 2 * time_delta - ITEM_RANGE.y1;;
		position_.y = target;

		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y, ground_obj.IsCollided(ITEM_RANGE.x1 + position_.x,
				ITEM_RANGE.x2 + position_.x, max_y, position_.y));
		}

		// For the case item is collided with the ground, it should stop.
		if (position_.y != target)
		{
			velocity_.y = 0; 
		}
		position_.y += ITEM_RANGE.y1;
	}
	
	velocity_.y = after_vy;
}

bool ItemClass::Frame(time_t curr_time, time_t time_delta)
{
	return curr_time <= createTime_ + ITEM_LIFETIME && state_ == ItemState::kNormal;
}

XMMATRIX ItemClass::GetShapeMatrix(time_t curr_time)
{
	constexpr float box_size = 0.3f;
	const time_t age = curr_time - createTime_;

	return XMMatrixTranslation(0, sin(age * 0.001) * 30000 * kScope, 0) *
		XMMatrixRotationY(age * 0.001f) * XMMatrixScaling(box_size, box_size * 1.2f, box_size);
}