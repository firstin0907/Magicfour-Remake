#include "../include/CharacterClass.hh"

#include <Windows.h>

#include "../include/global.hh"

#include "../include/InputClass.hh"
#include "../include/AnimatedObjectClass.hh"
#include "../include/SkillObjects.hh"
#include "../include/GroundClass.hh"

using namespace DirectX;
using namespace std;

constexpr int SKILL_COOLTIME = 1'500;
constexpr int COMBO_DURATION = 5'000;
constexpr int INVINCIBLE_TIME = 5'000;
constexpr int WALK_SPD = 700, RUN_SPD = 1300;

CharacterClass::CharacterClass(int pos_x, int pos_y)
	: RigidbodyClass(
		Point2d(pos_x, pos_y),
		rect_t{ -50000, 0, 50000, 400000 }, LEFT_FORWARD
	), jump_cnt(0), score_(0)
{
	jump_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\jump_motion.txt");
	fall_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\fall_motion.bvh");
	walk_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\walk_motion.txt");
	run_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\run_motion.txt");
	skill_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\skill_motion.bvh");

	SetState(CharacterState::kNormal, 0);

	skill_[0] = 4;
	skill_[1] = 3;
	skill_[2] = 1;
	skill_[3] = 2;

	time_invincible_end_ = 0;
}

bool CharacterClass::Frame(time_t time_delta, time_t curr_time, InputClass* input,
	vector<unique_ptr<class SkillObjectClass> >& skill_objs,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	bool is_walk = false;

	if (state_ != CharacterState::kHit && state_ != CharacterState::kSlip && state_ != CharacterState::kDie)
	{
		if (input->IsKeyPressed(DIK_LEFT))
		{
			if (state_ == CharacterState::kStop && direction_ != LEFT_FORWARD)
			{
				SetState(CharacterState::kNormal, curr_time);
			}
			direction_ = LEFT_FORWARD;

			is_walk = !is_walk;
		}

		if (input->IsKeyPressed(DIK_RIGHT))
		{
			if (state_ == CharacterState::kStop && direction_ != RIGHT_FORWARD)
			{
				SetState(CharacterState::kNormal, curr_time);
			}
			direction_ = RIGHT_FORWARD;

			is_walk = !is_walk;
		}
		if (input->IsKeyDown(DIK_Z)) UseSkill(curr_time, skill_objs);
	}


	if (input->IsKeyDown(DIK_X))
	{
		for (int i = 3; i >= 0; i--)
		{
			if (skill_[i])
			{
				skill_[i] = 0;
				break;
			}
		}
	}

	if (time_combo_end_ < curr_time)
	{
		combo_ = 0;
	}

	// jump attempt
	if (jump_cnt <= 1 && input->IsKeyDown(DIK_UP)
		&& state_ != CharacterState::kSpell && state_ != CharacterState::kHit
		&& state_ != CharacterState::kSlip && state_ != CharacterState::kDie)
	{
		velocity_.y = 2'800, jump_cnt++;
		if (state_ == CharacterState::kRun || state_ == CharacterState::kRunJump)
			SetState(CharacterState::kRunJump, curr_time);
		else SetState(CharacterState::kJump, curr_time);
	}
	else
	{
		const int start_y = position_.y;
		const int target_y = position_.y + time_delta * (velocity_.y - (GRAVITY / 2) * time_delta);
		velocity_.y -= GRAVITY * (int)time_delta;

		position_.y = target_y;
		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y,
				ground_obj->IsColiided(range_.x1 + position_.x, range_.x2 + position_.x, start_y, target_y));
		}

		if (position_.y != target_y)
		{
			velocity_.y = jump_cnt = 0;
		}
	}



	switch (state_)
	{
	case CharacterState::kJump:
		if(is_walk)
		{
			position_.x += DIR_WEIGHT(direction_, WALK_SPD) * (int)time_delta;
			position_.x = SATURATE(LEFT_X, position_.x, RIGHT_X);
		}

		if (GetStateTime(curr_time) >= 1000) SetState(CharacterState::kNormal, curr_time);
		else if (jump_cnt == 0 && is_walk) SetState(CharacterState::kWalk, curr_time);
		break;

	case CharacterState::kRunJump:
		if (is_walk)
		{
			position_.x += DIR_WEIGHT(direction_, RUN_SPD) * (int)time_delta;
			position_.x = SATURATE(LEFT_X, position_.x, RIGHT_X);
		}

		if (jump_cnt == 0)
		{
			if (is_walk) SetState(CharacterState::kRun, curr_time);
			else state_ = CharacterState::kJump;
		}
		break;

	case CharacterState::kNormal:
		if (is_walk)
		{
			SetState(CharacterState::kWalk, curr_time);
			position_.x += DIR_WEIGHT(direction_, WALK_SPD) * (int)time_delta;
			position_.x = SATURATE(LEFT_X, position_.x, RIGHT_X);
		}
		break;

	case CharacterState::kWalk:
		if (!is_walk) SetState(CharacterState::kStop, curr_time);
		else
		{
			position_.x += DIR_WEIGHT(direction_, WALK_SPD) * (int)time_delta;
			position_.x = SATURATE(LEFT_X, position_.x, RIGHT_X);
		}
		break;

	case CharacterState::kRun:
		if (!is_walk) SetState(CharacterState::kStop, curr_time);
		else
		{
			position_.x += DIR_WEIGHT(direction_, RUN_SPD) * (int)time_delta;
			position_.x = SATURATE(LEFT_X, position_.x, RIGHT_X);
		}
		break;

	case CharacterState::kStop:
		if (GetStateTime(curr_time) >= 150)
			SetState(CharacterState::kNormal, state_start_time_ + 150);
		else if(is_walk) SetState(CharacterState::kRun, curr_time);
		break;


	case CharacterState::kSpell:
		OnSkill(curr_time, skill_objs);
		break;


	case CharacterState::kHit:
		position_.x += (int)time_delta * velocity_.x;
		if (GetStateTime(curr_time) >= 500)
		{
			SetState(CharacterState::kSlip, state_start_time_);
		}
		break;

	case CharacterState::kSlip:
		SetStateIfTimeOver(CharacterState::kNormal, curr_time, 1'000);
		break;

	case CharacterState::kDie:
		
		if (GetStateTime(curr_time) < 1000)
		{
			position_.x = SATURATE(LEFT_X, position_.x + (int)time_delta * velocity_.x, RIGHT_X);
		}

		break;
	}

	return true;
}

void CharacterClass::GetShapeMatrices(time_t curr_time, std::vector<XMMATRIX>& shape_matrices)
{
	XMMATRIX root_transform = XMMatrixRotationY(DIR_WEIGHT(direction_, XM_PI * 0.65f));
	//XMMATRIX root_transform = XMMatrixIdentity();

	float state_elapsed_seconds = (GetStateTime(curr_time)) / 1000.0f;

	switch (state_)
	{
	case CharacterState::kNormal:
	case CharacterState::kStop:
		run_animation_data_->UpdateGlobalMatrices(0, root_transform, shape_matrices);
		break;

	case CharacterState::kWalk:
		walk_animation_data_->UpdateGlobalMatrices(
			state_elapsed_seconds / 0.00333333, root_transform, shape_matrices);
		break;

	case CharacterState::kRun:
		run_animation_data_->UpdateGlobalMatrices(
			state_elapsed_seconds / 0.00333333, root_transform, shape_matrices);
		break;

	case CharacterState::kJump:
	case CharacterState::kRunJump:
		if(GetStateTime(curr_time) > 90)
			jump_animation_data_->UpdateGlobalMatrices(
				20 + state_elapsed_seconds / 0.00333333, root_transform, shape_matrices);
		else
			jump_animation_data_->UpdateGlobalMatrices(
				43 + state_elapsed_seconds / 0.00833333, root_transform, shape_matrices);
		break;
		
	case CharacterState::kSpell:
		root_transform *= XMMatrixRotationY(XM_PI * 0.5f);
		skill_animation_data_->UpdateGlobalMatrices(
			state_elapsed_seconds / 0.00133333, root_transform, shape_matrices);
		break;

	case CharacterState::kHit:
	case CharacterState::kSlip:
		fall_animation_data_->UpdateGlobalMatrices(
			50 + state_elapsed_seconds / 0.00433333, root_transform, shape_matrices);
		break;

	case CharacterState::kDie:
		fall_animation_data_->UpdateGlobalMatrices(
			min(394, 50 + state_elapsed_seconds / 0.01433333), root_transform, shape_matrices);
		break;

	}
}

bool CharacterClass::OnCollided(time_t curr_time, int vx)
{
	if (time_invincible_end_ < curr_time && time_skill_ended_ < curr_time)
	{
		combo_ = 0;


		SetState(CharacterState::kHit, curr_time);
		direction_ = (vx > 0) ? LEFT_FORWARD : RIGHT_FORWARD;

		// lost skill
		if (skill_[0] == 0)
		{
			SetState(CharacterState::kDie, curr_time);
			hit_vx_ = velocity_.x = vx / 2;

			velocity_.y = 1500;
			time_invincible_end_ = 1LL << 59;
			return false;
		}
		else
		{
			for (int i = 3; i >= 0; i--)
			{
				if (skill_[i])
				{
					skill_[i] = 0;
					break;
				}
			}
			hit_vx_ = velocity_.x = vx / 3;
			velocity_.y = 1500;
			time_invincible_end_ = state_start_time_ + INVINCIBLE_TIME;
			return true;
		}
	}
	return true;
}

float CharacterClass::GetCooltimeGaugeRatio(time_t curr_time)
{
	return SATURATE(-0.3f, (time_skill_available_ - (long long)curr_time) / (float)SKILL_COOLTIME, 1.0f);
}

void CharacterClass::LearnSkill(int skill_id)
{
	for (auto& skill : skill_)
	{
		if (!skill) { skill = skill_id; return; }
	}
}

void CharacterClass::AddCombo(time_t curr_time)
{
	++combo_;
	time_combo_end_ = curr_time + COMBO_DURATION;
}

void CharacterClass::OnSkill(time_t curr_time,
	vector<unique_ptr<class SkillObjectClass> >& skill_objs)
{
	const time_t state_elapsed_time = GetStateTime(curr_time);

	switch (skillUsed_)
	{
	case 0:
		if (skillState_ == 0 && state_elapsed_time >= 100)
		{
			skill_objs.emplace_back(new SkillObjectBasic(position_.x + DIR_WEIGHT(direction_, 185000), position_.y,
				DIR_WEIGHT(direction_, 100), state_start_time_ + 100));
			skillState_ = 1;
		}
		break;

	case 1:	
		if (skillState_ == 0 && state_elapsed_time >= 100)
		{
			skill_objs.emplace_back(new SkillObjectSpear(position_.x, position_.y, 4000, -4000, state_start_time_ + 100));
			skill_objs.emplace_back(new SkillObjectSpear(position_.x, position_.y, 2000, -4000, state_start_time_ + 100));
			skill_objs.emplace_back(new SkillObjectSpear(position_.x, position_.y, 0, -4000, state_start_time_ + 100));
			skill_objs.emplace_back(new SkillObjectSpear(position_.x, position_.y, -2000, -4000, state_start_time_ + 100));
			skill_objs.emplace_back(new SkillObjectSpear(position_.x, position_.y, -4000, -4000, state_start_time_ + 100));

			skillState_ = 1;
		}			
		break;

	case 2:
		for (; skillState_ < 4 && state_elapsed_time >= skillState_ * 70 + 70; skillState_++)
		{
			skill_objs.emplace_back(new SkillObjectBead(
				position_.x, position_.y + 300000, DIR_WEIGHT(direction_, 1200),
				(skillState_ - 1) * 200, state_start_time_ + skillState_ * 70 + 70));
		}
		break;

	case 3:
		if (skillState_ == 0 && state_elapsed_time >= 50)
		{
			skill_objs.emplace_back(new SkillObjectLeg(
				position_.x + DIR_WEIGHT(direction_, 300'000), state_start_time_ + 50));
			skillState_ = 1;
		}
		break;
	}

	// When Skill Ended, return to normal state_.
	SetStateIfTimeOver(CharacterState::kNormal, curr_time, 300);
}

bool CharacterClass::UseSkill(time_t curr_time,
	vector<unique_ptr<class SkillObjectClass> >& skill_objs)
{
	if (curr_time < time_skill_available_)
	{
		// TODO: message print
		return false;
	}

	skillUsed_ = 0;
	for (int i = 3; i >= 0; i--)
	{
		if (skill_[i])
		{
			skillUsed_ = skill_[i];
			break;
		}
	}

	skillState_ = 0;
	SetState(CharacterState::kSpell, curr_time);

	switch (skillUsed_)
	{
	case 0:
		time_skill_ended_ = state_start_time_ + 300;
		break;

	case 1:
		if (velocity_.y == 0 && jump_cnt == 0) velocity_.y = 3'600;

		time_skill_ended_ = state_start_time_ + 300;
		break;

	case 2:
		time_skill_ended_ = state_start_time_ + 300;
		break;

	case 3:
		time_skill_ended_ = state_start_time_ + 300;
		break;
	}

	time_skill_available_ = time_skill_ended_ + SKILL_COOLTIME;

	return true;
}