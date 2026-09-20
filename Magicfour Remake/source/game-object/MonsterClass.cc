#include "game-object/MonsterClass.hh"

#include "core/global.hh"
#include "ui/MonsterUI.hh"
#include "ui/UserInterfaceClass.hh"

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

bool MonsterClass::Frame(time_t curr_time, time_t time_delta, SoundClass* sound_manager)
{
	prev_hp_ = (prev_hp_ * 30 + hp_) / 31;
	return true;
}

void MonsterClass::DrawUI(time_t curr_time, time_t time_delta, UserInterfaceClass* ui) const
{
	if (state_ == MonsterState::kEmbryo || state_ == MonsterState::kDie) return;

	float screen_x = 0, screen_y = 0;
	ui->CalculateScreenPos(GetLocalWorldMatrix(), screen_x, screen_y);

	if (state_ == MonsterState::kStopEmbryo)
	{
		MonsterUI::DrawWarningVerticalRect(
			ui->GetContext().direct2d_, ui, screen_x, 50,
			GetStateTime(curr_time) / 700.0f);
	}
	else
	{
		MonsterUI::DrawMonsterHp(
			ui->GetContext().direct2d_, ui, screen_x, screen_y - 23,
			GetHpRatio(), GetPrevHpRatio());
	}
}


bool MonsterClass::Damage(const int amount, time_t damaged_time, int vx, int vy)
{
	hp_ -= amount;

	SetState((hp_ > 0) ? MonsterState::kHit : MonsterState::kDie, damaged_time);

	hit_vx_ = vx, hit_vy_ = vy;
	velocity_ = Vector2d(vx, vy);
	accel_ = Vector2d(-vx / 1000, -kGravity);

	if (hit_vx_ > 0) direction_ = kLeftForword;
	else if (hit_vx_ < 0) direction_ = kRightForward;

	return hp_ > 0;
}