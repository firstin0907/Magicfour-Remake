#include "../include/Monsters.hh"

#include "../include/global.hh"
#include "../include/GroundClass.hh"
#include "../include/RandomClass.hh"

using namespace std;

MonsterDuck::MonsterDuck(direction_t direction, time_t created_time)
	: MonsterClass(
		Point2d(DIR_WEIGHT(direction_, SPAWN_RIGHT_X), GROUND_Y),
		direction, 1, 70, {-70000, 0, 70000, 300000})
{
	next_jump_time_ = created_time + 5000;
	SetState(MonsterState::kEmbryo, created_time);
}

void MonsterDuck::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	constexpr int spd = 1'000;
	constexpr int kKnockBackTime = 1'000;

	if (curr_time - time_delta < state_start_time_)
		time_delta = curr_time - state_start_time_;

	switch (state_)
	{

	case MonsterState::kEmbryo:
		position_.x += DIR_WEIGHT(direction_, spd * time_delta);
		if (LEFT_X <= position_.x && position_.x <= RIGHT_X) SetState(MonsterState::kNormal, curr_time);
		break;

	case MonsterState::kDuckJump:
	case MonsterState::kNormal:
		// MOVE first
		position_.x += DIR_WEIGHT(direction_, spd * time_delta);

		if (position_.x >= RIGHT_X)
		{
			position_.x = 2 * RIGHT_X - position_.x;
			direction_ = LEFT_FORWARD;
		}

		if (position_.x <= -RIGHT_X)
		{
			position_.x = -2 * RIGHT_X - position_.x;
			direction_ = RIGHT_FORWARD;
		}

	case MonsterState::kDuckJumpReady:
		{
			const int before_vy = velocity_.y;
			const int after_vy = velocity_.y - GRAVITY * time_delta;

			velocity_.y = after_vy;

			if (after_vy >= 0)
			{
				position_.y += (before_vy + after_vy) / 2 * time_delta;
			}
			else if (before_vy >= 0) // up and down
			{
				const int max_y = position_.y + before_vy / 2 * before_vy / GRAVITY;
				const int target = position_.y + (before_vy + after_vy) / 2 * time_delta;
				position_.y = target;

				for (auto& ground_obj : ground)
				{
					position_.y = max(position_.y,
						ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, max_y, position_.y));
				}

				if (position_.y != target)
				{
					if(state_ == MonsterState::kDuckJump)
						SetState(MonsterState::kNormal, curr_time);
					velocity_.y = 0;
				}
			}
			else
			{
				const int max_y = position_.y;
				const int target = position_.y + (before_vy + after_vy) / 2 * time_delta;
				position_.y = target;

				for (auto& ground_obj : ground)
				{
					position_.y = max(position_.y, ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, max_y, position_.y));

				}

				if (position_.y != target)
				{
					if (state_ == MonsterState::kDuckJump) SetState(MonsterState::kNormal, curr_time);
					velocity_.y = 0;
				}

			}
		}		


		break;
	case MonsterState::kHit:
	case MonsterState::kDie:
		{
			const time_t avg_time = (curr_time - time_delta / 2) - state_start_time_;

			position_.x += (hit_vx_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;

			const int start_y = position_.y;
			const int target_y = position_.y + (hit_vy_ * (kKnockBackTime - avg_time) / kKnockBackTime
				- GRAVITY * avg_time) * time_delta;

			position_.y += (hit_vy_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;
			position_.y -= (GRAVITY * avg_time) * time_delta;

			if (position_.x > RIGHT_X) position_.x = RIGHT_X;
			else if (position_.x < LEFT_X) position_.x = LEFT_X;

			position_.y = target_y;
			for (auto& ground_obj : ground)
			{
				position_.y = max(position_.y,
					ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, start_y, target_y));
			}


			break;
		}
	}
}

bool MonsterDuck::Frame(time_t curr_time, time_t time_delta)
{
	MonsterClass::Frame(curr_time, time_delta);
	switch (state_)
	{
	case MonsterState::kNormal:
		if (next_jump_time_ < curr_time)
		{
			SetState(MonsterState::kDuckJumpReady, curr_time);
			next_jump_time_ = state_start_time_ + RandomClass::rand(2000, 7000);
		}
		break;

	case MonsterState::kHit:
		if (curr_time - state_start_time_ >= 1000)
			SetState(MonsterState::kNormal, state_start_time_ + 1000);
		
		next_jump_time_ = state_start_time_ + RandomClass::rand(2000, 7000);

		break;

	case MonsterState::kDuckJumpReady:
		if (curr_time - state_start_time_ >= 400)
		{
			velocity_.y = 4'100;
			SetState(MonsterState::kDuckJump, state_start_time_ + 400);
		}

		break;


	case MonsterState::kDie:
		if (curr_time - state_start_time_ >= 1000) return false;
		else return true;
	}
	return true;
}

int MonsterDuck::GetVx()
{
	return DIR_WEIGHT(direction_, 1000);
}

MonsterOctopus::MonsterOctopus(direction_t direction, time_t created_time)
	: MonsterClass(
		Point2d(DIRECTION_T(direction, SPAWN_RIGHT_X), GROUND_Y),
		direction, 3, 100, { -200000, 0, 200000, 300000 })
{
	SetState(MonsterState::kEmbryo, created_time);
}

void MonsterOctopus::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	constexpr int spd = 500;
	constexpr int kKnockBackTime = 1'000;

	if (curr_time - time_delta < state_start_time_)
		time_delta = curr_time - state_start_time_;

	switch (state_)
	{
	case MonsterState::kEmbryo:
		position_.x += spd * time_delta * ((direction_ == LEFT_FORWARD) ? -1 : 1);
		if (LEFT_X <= position_.x && position_.x <= RIGHT_X) SetState(MonsterState::kNormal, curr_time);
		break;

	case MonsterState::kNormal:
		position_.x += spd * time_delta * ((direction_ == LEFT_FORWARD) ? -1 : 1);

		if (position_.x >= RIGHT_X)
		{
			position_.x = 2 * RIGHT_X - position_.x;
			direction_ = LEFT_FORWARD;
		}

		if (position_.x <= -RIGHT_X)
		{
			position_.x = -2 * RIGHT_X - position_.x;
			direction_ = RIGHT_FORWARD;
		}
		break;

	case MonsterState::kHit:
	case MonsterState::kDie:
	{
		const time_t avg_time = (curr_time - time_delta / 2) - state_start_time_;

		position_.x += (hit_vx_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;
		position_.y += (hit_vy_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;
		position_.y -= (GRAVITY * avg_time) * time_delta;

		if (position_.x > RIGHT_X) position_.x = RIGHT_X;
		else if (position_.x < LEFT_X) position_.x = LEFT_X;

		if (position_.y < GROUND_Y) position_.y = GROUND_Y;

		break;
	}
	}
}

bool MonsterOctopus::Frame(time_t curr_time, time_t time_delta)
{
	MonsterClass::Frame(curr_time, time_delta);
	switch (state_)
	{
	case MonsterState::kNormal:
		break;

	case MonsterState::kHit:
		SetStateIfTimeOver(MonsterState::kNormal, curr_time, 1'000);
		break;

	case MonsterState::kDie:
		if (curr_time - state_start_time_ >= 1000) return false;
		else return true;
	}

	return true;
}

int MonsterOctopus::GetVx()
{	
	return DIR_WEIGHT(direction_, 500);
}

MonsterBird::MonsterBird(direction_t direction, time_t created_time)
	: MonsterClass(
		Point2d(
			DIR_WEIGHT(direction, SPAWN_RIGHT_X),
			max(7, RandomClass::rand(-2, 8)) * 150'000 + 200'000
		), direction, 2, 55, { -105000, 0, 105000, 140000 })
{
	SetState(MonsterState::kEmbryo, created_time);
	next_relocation_time_ = created_time + RandomClass::rand(1000, 4000);
	target_y_pos_ = position_.y;
}

void MonsterBird::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass>>& ground)
{
	constexpr int X_SPEED = 1500, Y_SPEED = 400;
	constexpr int kKnockBackTime = 1'000;

	// set targetYPosition
	if (curr_time >= next_relocation_time_)
	{
		target_y_pos_ = min(7, RandomClass::rand(-2, 8)) * 150'000 + 200'000;

		if (target_y_pos_ != position_.y) SetState(MonsterState::kBirdMove, next_relocation_time_);

		next_relocation_time_ += RandomClass::rand(3000, 10000);
	}


	switch (state_)
	{

	case MonsterState::kEmbryo:
		position_.x += X_SPEED * time_delta * ((direction_ == LEFT_FORWARD) ? -1 : 1);
		if (LEFT_X <= position_.x && position_.x <= RIGHT_X) SetState(MonsterState::kNormal, curr_time);
		break;

	case MonsterState::kBirdMove:
		if (target_y_pos_ < position_.y)
		{
			position_.y -= (int)time_delta * Y_SPEED;
			if (target_y_pos_ >= position_.y)
			{
				SetState(MonsterState::kNormal, curr_time - (target_y_pos_ - position_.y) / Y_SPEED);
				position_.y = target_y_pos_;
			}
		}
		else
		{
			position_.y += (int)time_delta * Y_SPEED;
			if (target_y_pos_ <= position_.y)
			{
				SetState(MonsterState::kNormal, curr_time - (position_.y - target_y_pos_) / Y_SPEED);
				position_.y = target_y_pos_;
			}
		}


	case MonsterState::kNormal:
		position_.x += X_SPEED * time_delta * ((direction_ == LEFT_FORWARD) ? -1 : 1);

		if (position_.x >= RIGHT_X)
		{
			position_.x = 2 * RIGHT_X - position_.x;
			direction_ = LEFT_FORWARD;
		}

		if (position_.x <= -RIGHT_X)
		{
			position_.x = -2 * RIGHT_X - position_.x;
			direction_ = RIGHT_FORWARD;
		}
		break;

	case MonsterState::kHit:
	case MonsterState::kDie:
	{
		const time_t avg_time = (curr_time - time_delta / 2) - state_start_time_;

		position_.x += (hit_vx_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;

		const int start_y = position_.y;
		const int target_y = position_.y + (hit_vy_ * (kKnockBackTime - avg_time) / kKnockBackTime
			- GRAVITY * avg_time) * time_delta;

		position_.y += (hit_vy_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;
		position_.y -= (GRAVITY * avg_time) * time_delta;

		if (position_.x > RIGHT_X) position_.x = RIGHT_X;
		else if (position_.x < LEFT_X) position_.x = LEFT_X;

		position_.y = target_y;
		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y,
				ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, start_y, target_y));
		}
		break;
	}
	}
}

bool MonsterBird::Frame(time_t curr_time, time_t time_delta)
{
	MonsterClass::Frame(curr_time, time_delta);
	switch (state_)
	{
	case MonsterState::kHit:

		if (curr_time - state_start_time_ >= 1000)
		{
			target_y_pos_ = min(7, RandomClass::rand(-2, 8)) * 150'000 + 200'000;

			SetState(MonsterState::kBirdMove, state_start_time_ + 1000);

			next_relocation_time_ = state_start_time_ + RandomClass::rand(3000, 10000);
		}
		break;

	case MonsterState::kDie:
		if (curr_time - state_start_time_ >= 1000) return false;
		else return true;
	}

	return true;
}

int MonsterBird::GetVx()
{
	return DIR_WEIGHT(direction_, 1500);
}

MonsterStop::MonsterStop(time_t created_time)
	: MonsterClass(
		Point2d(RandomClass::rand(LEFT_X, RIGHT_X), 1'000'000),
		LEFT_FORWARD, 4, 100, { -50000, 0, 50000, 400000 })
{
	SetState(MonsterState::kNormal, created_time);
}

void MonsterStop::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass>>& ground)
{
	constexpr int kKnockBackTime = 1'000;
	switch (state_)
	{
	case MonsterState::kNormal:
	{
		const int start_y = position_.y;
		const int target_y = position_.y + (velocity_.y - GRAVITY * time_delta / 2) * time_delta;

		position_.y = target_y;
		velocity_.y -= GRAVITY * time_delta;

		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y,				
				ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, start_y, target_y));
		}

		if (position_.y > target_y)
		{
			SetState(MonsterState::kStopOnGround, curr_time);
			velocity_.y = 0;
		}
		break;
	}		

	case MonsterState::kHit:
	case MonsterState::kDie:
	{
		const time_t avg_time = (curr_time - time_delta / 2) - state_start_time_;

		position_.x += (hit_vx_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;

		const int start_y = position_.y;
		const int target_y = position_.y + (hit_vy_ * (kKnockBackTime - avg_time) / kKnockBackTime
			- GRAVITY * avg_time) * time_delta;

		position_.y += (hit_vy_ * (kKnockBackTime - avg_time) / kKnockBackTime) * time_delta;
		position_.y -= (GRAVITY * avg_time) * time_delta;

		if (position_.x > RIGHT_X) position_.x = RIGHT_X;
		else if (position_.x < LEFT_X) position_.x = LEFT_X;

		position_.y = target_y;
		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y,
				ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, start_y, target_y));
		}
		break;
	}
	}
}

bool MonsterStop::Frame(time_t curr_time, time_t time_delta)
{
	MonsterClass::Frame(curr_time, time_delta);

	switch (state_)
	{
	case MonsterState::kHit:
		SetStateIfTimeOver(MonsterState::kNormal, curr_time, 1'000);
		break;

	case MonsterState::kDie:
		if (curr_time - state_start_time_ >= 1000) return false;
		else return true;
		break;
	}

	return true;
}

int MonsterStop::GetVx()
{
	return 0;
}