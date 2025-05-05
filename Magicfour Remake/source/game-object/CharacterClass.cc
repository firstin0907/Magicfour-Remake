#include "game-object/CharacterClass.hh"

#include <Windows.h>

#include "core/global.hh"

#include "core/InputClass.hh"
#include "core/AnimatedObjectClass.hh"
#include "game-object/SkillObjects.hh"
#include "map/GroundClass.hh"
#include "core/SoundClass.hh"
#include "util/RandomClass.hh"

using namespace DirectX;
using namespace std;

constexpr int kSkillCooltime = 800;
constexpr int kComboDuration = 5'000;
constexpr int kInvincibleDuration = 5'000;
constexpr int kWalkSpd = 700, kRunSpd = 1300;

CharacterClass::CharacterClass(int pos_x, int pos_y,
	class InputClass* input, class SoundClass* sound,
	vector<unique_ptr<class IGameObject> >& skill_objs)
	: RigidbodyClass(
		Point2d(pos_x, pos_y),
		rect_t{ -50000, 0, 50000, 400000 }, LEFT_FORWARD
	), jump_cnt(0), score_(0), input(input), sound(sound), skill_objs(skill_objs)
{
	jump_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\jump_motion.txt");
	fall_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\fall_motion.bvh");
	walk_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\walk_motion.txt");
	run_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\run_motion.txt");
	skill_animation_data_ = make_unique<AnimatedObjectClass>("data\\motion\\skill_motion.bvh");

	SetState(CharacterState::kNormal, 0);

	skill_[0] = { 1, 1 };
	skill_[1] = { 2, 1 };
	skill_[2] = { 3, 1 };
	skill_[3] = { 4, 1 };

	time_invincible_end_ = 0;

	skill_bonus_ = SkillBonus::BONUS_NONE;
	time_skill_bonus_get_ = 0;

	guardians_[0] = make_unique<SkillObjectGuardian>();
	guardians_[1] = make_unique<SkillObjectGuardian>();
}

void CharacterClass::FrameMove(time_t curr_time, time_t time_delta, const vector<class GroundClass>& ground)
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
		if (input->IsKeyDown(DIK_Z)) UseSkill(curr_time, skill_objs, sound);
	}


	if (input->IsKeyDown(DIK_X))
	{
		for (int i = 3; i >= 0; i--)
		{
			if (skill_[i].skill_type)
			{
				skill_[i].skill_type = 0;
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
		const int target_y = position_.y + time_delta * (velocity_.y - (kGravity / 2) * time_delta);
		velocity_.y -= kGravity * (int)time_delta;

		position_.y = target_y;
		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y,
				ground_obj.IsCollided(range_.x1 + position_.x, range_.x2 + position_.x, start_y, target_y));
		}

		if (position_.y != target_y)
		{
			velocity_.y = jump_cnt = 0;
		}
	}



	switch (state_)
	{
	case CharacterState::kJump:
		if (is_walk)
		{
			position_.x += DIR_WEIGHT(direction_, kWalkSpd) * (int)time_delta;
			position_.x = SATURATE(kFieldLeftX, position_.x, kFieldRightX);
		}

		if (GetStateTime(curr_time) >= 1000) SetState(CharacterState::kNormal, curr_time);
		else if (jump_cnt == 0 && is_walk) SetState(CharacterState::kWalk, curr_time);
		break;

	case CharacterState::kRunJump:
		if (is_walk)
		{
			position_.x += DIR_WEIGHT(direction_, kRunSpd) * (int)time_delta;
			position_.x = SATURATE(kFieldLeftX, position_.x, kFieldRightX);
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
			position_.x += DIR_WEIGHT(direction_, kWalkSpd) * (int)time_delta;
			position_.x = SATURATE(kFieldLeftX, position_.x, kFieldRightX);
		}
		break;

	case CharacterState::kWalk:
		if (!is_walk) SetState(CharacterState::kStop, curr_time);
		else
		{
			position_.x += DIR_WEIGHT(direction_, kWalkSpd) * (int)time_delta;
			position_.x = SATURATE(kFieldLeftX, position_.x, kFieldRightX);
		}
		break;

	case CharacterState::kRun:
		if (!is_walk) SetState(CharacterState::kStop, curr_time);
		else
		{
			position_.x += DIR_WEIGHT(direction_, kRunSpd) * (int)time_delta;
			position_.x = SATURATE(kFieldLeftX, position_.x, kFieldRightX);
		}
		break;

	case CharacterState::kStop:
		if (GetStateTime(curr_time) >= 150)
			SetState(CharacterState::kNormal, state_start_time_ + 150);
		else if (is_walk) SetState(CharacterState::kRun, curr_time);
		break;


	case CharacterState::kSpell:
	{
		if (skill_bonus_ == SkillBonus::BONUS_FOUR_CARDS)
		{
			// if the character BONUS_FOUR_CARDS bounus, use all skills he has.
			int used[5] = { 0 };
			for (int i = 0; i <= 3 && skill_[i].skill_type; i++)
			{
				if (used[skill_[i].skill_type]) continue;
				used[skill_[i].skill_type] = 1;

				skill_currently_used_ = skill_[i];
				OnSkill(curr_time, time_delta, skill_objs);
			}
		}
		else OnSkill(curr_time, time_delta, skill_objs);

		break;
	}

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
			position_.x = SATURATE(kFieldLeftX, position_.x + (int)time_delta * velocity_.x, kFieldRightX);
		}

		break;
	}


	if (skill_bonus_ == SkillBonus::BONUS_ONE_PAIR || skill_bonus_ == SkillBonus::BONUS_TWO_PAIR)
	{
		constexpr float radius = 500'000.0f;
		const float offset_x = static_cast<int>(radius * cos(curr_time * 0.003f));
		const float offset_y = static_cast<int>(radius * sin(curr_time * 0.003f));

		guardians_[0]->SetPosition(position_.x + offset_x, position_.y + 200000 + offset_y);
		if (skill_bonus_ == SkillBonus::BONUS_TWO_PAIR)
		{
			guardians_[1]->SetPosition(position_.x - offset_x, position_.y + 200000 - offset_y);
		}
	}
}

bool CharacterClass::Frame(time_t time_delta, time_t curr_time)
{
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
		if (skill_[0].skill_type == 0)
		{
			SetState(CharacterState::kDie, curr_time);
			velocity_.x = vx / 2;

			velocity_.y = 1500;
			time_invincible_end_ = 1LL << 59;
		}
		else
		{
			for (int i = 3; i >= 0; i--)
			{
				if (skill_[i].skill_type)
				{
					skill_[i].skill_type = 0;
					break;
				}
			}
			velocity_.x = vx / 3;
			velocity_.y = 1500;
			time_invincible_end_ = state_start_time_ + kInvincibleDuration;
		}
		return true;
	}
	return false;
}

float CharacterClass::GetCooltimeGaugeRatio(time_t curr_time)
{
	return SATURATE(-0.3f, (time_skill_available_ - (long long)curr_time) / (float)kSkillCooltime, 1.0f);
}

float CharacterClass::GetInvincibleGaugeRatio(time_t curr_time)
{
	return SATURATE(-0.3f, (time_invincible_end_ - (long long)curr_time) / (float)kInvincibleDuration, 1.0f);
}



CharacterClass::SkillBonus CharacterClass::LearnSkill(
	int skill_id, time_t curr_time)
{
	int skill_power = RandomClass::rand(1, 10);
	score_ += skill_power;

	for (auto& skill : skill_)
	{
		// If an empty skill slot exists,
		if (skill.skill_type == 0)
		{
			// fill the skill with random power.
			skill.skill_type = skill_id;
			skill.skill_power = skill_power;
			skill.learned_time = curr_time;

			// If all skills is loaded,
			if (skill_[3].skill_type)
			{
				// check bonus and set it.
				time_skill_bonus_get_ = curr_time;
				return skill_bonus_ = CalculateSkillBonus();
			}
			// Otherwise, don't need to do anything.
			else return SkillBonus::BONUS_NONE;
		}
	}
	return SkillBonus::BONUS_NONE;
}

void CharacterClass::AddCombo(time_t curr_time)
{
	++combo_;
	time_combo_end_ = curr_time + kComboDuration;
}

void CharacterClass::OnSkill(time_t curr_time, time_t time_delta,
	vector<unique_ptr<class IGameObject> >& skill_objs)
{
	const time_t state_time = GetStateTime(curr_time);
	const time_t prev_state_time =
		(state_time >= time_delta) ? state_time - time_delta : 0;
		
	switch (skill_currently_used_.skill_type)
	{
	case 0:
		if (prev_state_time < 100 && 100 <= state_time)
		{
			if (skill_bonus_ == SkillBonus::BONUS_NO_PAIR)
			{
				skill_objs.emplace_back(new SkillObjectBasic(position_.x + DIR_WEIGHT(direction_, 185000), position_.y,
					DIR_WEIGHT(direction_, 100), 1, state_start_time_ + 100));
			}
			else
			{
				skill_objs.emplace_back(new SkillObjectBasic(position_.x + DIR_WEIGHT(direction_, 185000), position_.y,
					DIR_WEIGHT(direction_, 100), 0, state_start_time_ + 100));
			}
			
		}
		break;

	case 1:
		if (prev_state_time < 100 && 100 <= state_time)
		{
			constexpr int object_vx[9] = { 6000, 6000, 4000, 2000, 0, -2000, -4000, -6000, -6000 };
			constexpr int object_vy[9] = { 0, -2000, -3000, -4000, -4000, -4000, -3000, -2000, 0 };


			if (skill_bonus_ == SkillBonus::BONUS_FLUSH || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
			{
				for (int i = 0; i < 9; i++)
				{
					skill_objs.emplace_back(
						new SkillObjectSpear(position_.x, position_.y,
							object_vx[i], object_vy[i],
							skill_currently_used_.skill_power, state_start_time_ + 100));
				}
			}
			else
			{
				for (int i = 1; i < 8; i++)
				{
					skill_objs.emplace_back(
						new SkillObjectSpear(position_.x, position_.y,
							object_vx[i], object_vy[i],
							skill_currently_used_.skill_power, state_start_time_ + 100));
				}
			}

			
		}			
		break;

	case 2:
		if (skill_bonus_ == SkillBonus::BONUS_FLUSH || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
		{
			for (time_t i = min(1, state_time / 40); i <= 6; i++)
			{
				if (prev_state_time < i * 40 && i * 40 <= state_time)
				{
					skill_objs.emplace_back(new SkillObjectBead(
						position_.x, position_.y + 300000, DIR_WEIGHT(direction_, 1200),
						(i - 1) * 200, skill_currently_used_.skill_power,
						state_start_time_ + i * 40));
				}
			}
		}
		else
		{
			for (time_t i = min(1, state_time / 70); i <= 4; i++)
			{
				if (prev_state_time < i * 70 && i * 70 <= state_time)
				{
					skill_objs.emplace_back(new SkillObjectBead(
						position_.x, position_.y + 300000, DIR_WEIGHT(direction_, 1200),
						(i - 1) * 200, skill_currently_used_.skill_power,
						state_start_time_ + i * 70));
				}
			}
		}
		break;

	case 3:
		if (prev_state_time < 50 && 50 <= state_time)
		{
			if (skill_bonus_ == SkillBonus::BONUS_FLUSH || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
			{

				skill_objs.emplace_back(new SkillObjectLeg(
					position_.x - 300'000,
					skill_currently_used_.skill_power, state_start_time_ + 50));
				skill_objs.emplace_back(new SkillObjectLeg(
					position_.x + 300'000,
					skill_currently_used_.skill_power, state_start_time_ + 50));
			}
			else
			{
				skill_objs.emplace_back(new SkillObjectLeg(
					position_.x + DIR_WEIGHT(direction_, 300'000),
					skill_currently_used_.skill_power, state_start_time_ + 50));
			}
		}
		break;

	case 4:
		break;

	}


	// When Skill Ended, return to normal state_.
	SetStateIfTimeOver(CharacterState::kNormal, curr_time, 300);
}

bool CharacterClass::UseSkill(time_t curr_time,
	vector<unique_ptr<class IGameObject> >& skill_objs,
	SoundClass* sound)
{
	if (curr_time < time_skill_available_)
	{
		// TODO: message print
		return false;
	}

	SetState(CharacterState::kSpell, curr_time);

	skill_currently_used_ = { 0, 0 };
	for (int i = 3; i >= 0; i--)
	{
		if (skill_[i].skill_type || i == 0) skill_currently_used_ = skill_[i];
		else continue;

		switch (skill_currently_used_.skill_type)
		{
		case 0:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect(EffectSound::kSoundSpell3);
			break;

		case 1:
			if (velocity_.y == 0 && jump_cnt == 0) velocity_.y = 3'600;

			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect(EffectSound::kSoundSpell1);
			break;

		case 2:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect(EffectSound::kSoundSpell2);
			break;

		case 3:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect(EffectSound::kSoundSpell2);
			break;

		case 4:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect(EffectSound::kSoundSpell2);


			skill_objs.emplace_back(new SkillObjectShield(
				position_.x, position_.y + 200000, -1600, 0,
				skill_currently_used_.skill_power, state_start_time_));
			skill_objs.emplace_back(new SkillObjectShield(
				position_.x, position_.y + 200000, 1600, 0,
				skill_currently_used_.skill_power, state_start_time_));
			skill_objs.emplace_back(new SkillObjectShield(
				position_.x, position_.y + 200000, 0, 1600,
				skill_currently_used_.skill_power, state_start_time_));

			if (skill_bonus_ == SkillBonus::BONUS_FLUSH || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
			{
				skill_objs.emplace_back(new SkillObjectShield(
					position_.x, position_.y + 200000, 0, -1600,
					skill_currently_used_.skill_power, state_start_time_));
			}

			break;
		}

		if (skill_bonus_ != SkillBonus::BONUS_FOUR_CARDS)
		{
			break;
		}
	}

	
	if (skill_bonus_ == SkillBonus::BONUS_TRIPLE)
		skill_currently_used_.skill_power += 3;

	


	if (skill_bonus_ == SkillBonus::BONUS_STRAIGHT || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
	{
		time_skill_available_ = time_skill_ended_ + kSkillCooltime / 2;
	}
	else time_skill_available_ = time_skill_ended_ + kSkillCooltime;

	return true;
}

CharacterClass::SkillBonus CharacterClass::CalculateSkillBonus()
{
	bool is_flush = true, is_straight = true;
	// check is it flush
	for (int i = 1; i <= 3; i++)
	{
		if (skill_[i].skill_type != skill_[i - 1].skill_type)
		{
			is_flush = false;
		}
	}

	// check is it straight
	int powers[4] = { 0 };
	for (int i = 0; i <= 3; i++) powers[i] = skill_[i].skill_power;
	sort(powers, powers + 4);
	for (int i = 1; i <= 3; i++)
	{
		if (powers[i] != powers[i - 1] + 1)
		{
			is_straight = false;
		}
	}
		
	if (is_flush && is_straight)
		return SkillBonus::BONUS_STRAIGHT_FLUSH;

	if (powers[1] == powers[2] && powers[0] == powers[1] && powers[2] == powers[3])
		return SkillBonus::BONUS_FOUR_CARDS;

	if (is_flush)
		return SkillBonus::BONUS_FLUSH;

	if (is_straight)
		return SkillBonus::BONUS_STRAIGHT;

	if (powers[1] == powers[2] && (powers[0] == powers[1] || powers[2] == powers[3]))
		return SkillBonus::BONUS_TRIPLE;

	if (powers[0] == powers[1] && powers[2] == powers[3])
		return SkillBonus::BONUS_TWO_PAIR;

	if (powers[1] == powers[2] || powers[0] == powers[1] || powers[2] == powers[3])
		return SkillBonus::BONUS_ONE_PAIR;


	return SkillBonus::BONUS_NO_PAIR;
}