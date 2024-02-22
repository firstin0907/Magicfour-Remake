#include "../include/MonsterClass.hh"

#include "../include/global.hh"

int MonsterClass::monster_count_ = 0;

MonsterClass::MonsterClass(Point2d position, direction_t direction,
	int type, int hp,  rect_t range, time_t created_time)
	: RigidbodyClass<MonsterState>(position, range, direction),
	id_(++monster_count_), type_(type)
{
	hp_ = max_hp_ = prev_hp_ = hp;

	SetState(MonsterState::kEmbryo, created_time);
	hit_vx_ = hit_vy_ = 0;
}

bool MonsterClass::Frame(time_t curr_time, time_t time_delta)
{
	prev_hp_ = (prev_hp_ * 30 + hp_) / 31;
	return true;
}


bool MonsterClass::Damage(const int amount, time_t damaged_time, int vx, int vy)
{
	hp_ -= amount;

	SetState((hp_ > 0) ? MonsterState::kHit : MonsterState::kDie, damaged_time);

	hit_vx_ = vx, hit_vy_ = vy;
	velocity_ = Vector2d(vx, vy);
	accel_ = Vector2d(-vx / 1000, -kGravity);

	if (hit_vx_ > 0) direction_ = LEFT_FORWARD;
	else if (hit_vx_ < 0) direction_ = RIGHT_FORWARD;

	return hp_ > 0;
}