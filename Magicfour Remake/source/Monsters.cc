#include "../include/Monsters.hh"

#include "../include/global.hh"
#include "../include/GroundClass.hh"
#include "../include/RandomClass.hh"

MonsterDuck::MonsterDuck(direction_t direction, time_t created_time)
	: MonsterClass(1, 70, direction, {-70000, 0, 70000, 300000}), m_NextJumpTime(created_time + 5000)
{
	SetState(MONSTER_STATE_EMBRACE, created_time);

	m_PosX = DIR_WEIGHT(m_Direction, SPAWN_LEFT_X);
	m_PosY = GROUND_Y;
}

MonsterDuck::~MonsterDuck()
{
}

void MonsterDuck::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	constexpr int spd = 1'000;
	constexpr int knock_back_time = 1'000;

	if (curr_time - time_delta < m_StateStartTime)
		time_delta = curr_time - m_StateStartTime;

	switch (m_State)
	{

	case MONSTER_STATE_EMBRACE:
		m_PosX += spd * time_delta * ((m_Direction == LEFT_FORWARD) ? -1 : 1);
		if (LEFT_X <= m_PosX && m_PosX <= RIGHT_X) SetState(MONSTER_STATE_NORMAL, curr_time);
		break;

	case STATE_JUMP:
	case MONSTER_STATE_NORMAL:
		// MOVE first
		m_PosX += spd * time_delta * ((m_Direction == LEFT_FORWARD) ? -1 : 1);

		if (m_PosX >= RIGHT_X)
		{
			m_PosX = 2 * RIGHT_X - m_PosX;
			m_Direction = LEFT_FORWARD;
		}

		if (m_PosX <= -RIGHT_X)
		{
			m_PosX = -2 * RIGHT_X - m_PosX;
			m_Direction = RIGHT_FORWARD;
		}

	case STATE_JUMP_READY:
		{
			const int before_vy = vy;
			const int after_vy = vy - GRAVITY * time_delta;

			vy = after_vy;

			if (after_vy >= 0)
			{
				m_PosY += (before_vy + after_vy) / 2 * time_delta;
			}
			else if (before_vy >= 0) // up and down
			{
				const int max_y = m_PosY + before_vy / 2 * before_vy / GRAVITY;
				const int target = m_PosY + (before_vy + after_vy) / 2 * time_delta;
				m_PosY = target;

				for (auto& ground_obj : ground)
				{
					m_PosY = max(m_PosY,
						ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, max_y, m_PosY));
				}

				if (m_PosY != target)
				{
					if(m_State == STATE_JUMP) SetState(MONSTER_STATE_NORMAL, curr_time);
					vy = 0;
				}
			}
			else
			{
				const int max_y = m_PosY;
				const int target = m_PosY + (before_vy + after_vy) / 2 * time_delta;
				m_PosY = target;

				for (auto& ground_obj : ground)
				{
					m_PosY = max(m_PosY, ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, max_y, m_PosY));

				}

				if (m_PosY != target)
				{
					if (m_State == STATE_JUMP) SetState(MONSTER_STATE_NORMAL, curr_time);
					vy = 0;
				}

			}
		}		


		break;
	case MONSTER_STATE_HIT:
		{
			const time_t avg_time = (curr_time - time_delta / 2) - m_StateStartTime;

			m_PosX += (m_HitVx * (knock_back_time - avg_time) / knock_back_time) * time_delta;

			const int start_y = m_PosY;
			const int target_y = m_PosY + (m_HitVy * (knock_back_time - avg_time) / knock_back_time
				- GRAVITY * avg_time) * time_delta;

			m_PosY += (m_HitVy * (knock_back_time - avg_time) / knock_back_time) * time_delta;
			m_PosY -= (GRAVITY * avg_time) * time_delta;

			if (m_PosX > RIGHT_X) m_PosX = RIGHT_X;
			else if (m_PosX < LEFT_X) m_PosX = LEFT_X;

			m_PosY = target_y;
			for (auto& ground_obj : ground)
			{
				m_PosY = max(m_PosY,
					ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, start_y, target_y));
			}


			break;
		}
	}
}

bool MonsterDuck::Frame(time_t curr_time, time_t time_delta)
{
	MonsterClass::Frame(curr_time, time_delta);
	switch (m_State)
	{
	case MONSTER_STATE_NORMAL:
		if (m_NextJumpTime < curr_time)
		{
			SetState(STATE_JUMP_READY, curr_time);
			m_NextJumpTime = m_StateStartTime + RandomClass::rand(2000, 7000);
		}
		break;

	case MONSTER_STATE_HIT:
		if (curr_time - m_StateStartTime >= 1000)
			SetState(MONSTER_STATE_NORMAL, m_StateStartTime + 1000);
		
		m_NextJumpTime = m_StateStartTime + RandomClass::rand(2000, 7000);

		break;

	case STATE_JUMP_READY:
		if (curr_time - m_StateStartTime >= 400)
		{
			vy = 4'100;
			SetState(STATE_JUMP, m_StateStartTime + 400);
		}

		break;


	case MONSTER_STATE_DIE:
		if (curr_time - m_StateStartTime >= 1000) return false;
		else return true;
	}
	return true;
}

int MonsterDuck::GetVx()
{
	return DIR_WEIGHT(m_Direction, 1000);
}

MonsterOctopus::MonsterOctopus(direction_t direction, time_t created_time)
	: MonsterClass(3, 100, direction, { -200000, 0, 200000, 300000 })
{
	SetState(MONSTER_STATE_EMBRACE, created_time);

	m_PosX = RIGHT_X * ((m_Direction == LEFT_FORWARD) ? 1 : -1);
	m_PosY = GROUND_Y;
}

MonsterOctopus::~MonsterOctopus()
{
}

void MonsterOctopus::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	constexpr int spd = 500;
	constexpr int knock_back_time = 1'000;

	if (curr_time - time_delta < m_StateStartTime)
		time_delta = curr_time - m_StateStartTime;

	switch (m_State)
	{
	case MONSTER_STATE_EMBRACE:
		m_PosX += spd * time_delta * ((m_Direction == LEFT_FORWARD) ? -1 : 1);
		if (LEFT_X <= m_PosX && m_PosX <= RIGHT_X) SetState(MONSTER_STATE_NORMAL, curr_time);
		break;

	case MONSTER_STATE_NORMAL:
		m_PosX += spd * time_delta * ((m_Direction == LEFT_FORWARD) ? -1 : 1);

		if (m_PosX >= RIGHT_X)
		{
			m_PosX = 2 * RIGHT_X - m_PosX;
			m_Direction = LEFT_FORWARD;
		}

		if (m_PosX <= -RIGHT_X)
		{
			m_PosX = -2 * RIGHT_X - m_PosX;
			m_Direction = RIGHT_FORWARD;
		}
		break;

	case MONSTER_STATE_HIT:
	{
		const time_t avg_time = (curr_time - time_delta / 2) - m_StateStartTime;

		m_PosX += (m_HitVx * (knock_back_time - avg_time) / knock_back_time) * time_delta;
		m_PosY += (m_HitVy * (knock_back_time - avg_time) / knock_back_time) * time_delta;
		m_PosY -= (GRAVITY * avg_time) * time_delta;

		if (m_PosX > RIGHT_X) m_PosX = RIGHT_X;
		else if (m_PosX < LEFT_X) m_PosX = LEFT_X;

		if (m_PosY < GROUND_Y) m_PosY = GROUND_Y;

		break;
	}
	}
}

bool MonsterOctopus::Frame(time_t curr_time, time_t time_delta)
{
	MonsterClass::Frame(curr_time, time_delta);
	switch (m_State)
	{
	case MONSTER_STATE_NORMAL:
		break;

	case MONSTER_STATE_HIT:
		if (curr_time - m_StateStartTime >= 1000)
			SetState(MONSTER_STATE_NORMAL, m_StateStartTime + 1000);
		break;

	case MONSTER_STATE_DIE:
		return false;
	}

	return true;
}

int MonsterOctopus::GetVx()
{
	return DIR_WEIGHT(m_Direction, 500);
}

MonsterBird::MonsterBird(direction_t direction, time_t created_time)
	: MonsterClass(2, 55, direction, { -105000, 0, 105000, 140000 })
{
	SetState(MONSTER_STATE_EMBRACE, created_time);
	m_NextMoveTime = created_time + RandomClass::rand(1000, 4000);

	m_PosX = RIGHT_X * ((m_Direction == LEFT_FORWARD) ? 1 : -1);
	m_PosY = max(7, RandomClass::rand(-2, 8)) * 150'000 + 200'000;
}

MonsterBird::~MonsterBird()
{
}

void MonsterBird::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass>>& ground)
{
	constexpr int X_SPEED = 1500, Y_SPEED = 400;
	constexpr int knock_back_time = 1'000;

	// set targetYPosition
	if (curr_time >= m_NextMoveTime)
	{
		m_TargetYPosition = min(7, RandomClass::rand(-2, 8)) * 150'000 + 200'000;

		if (m_TargetYPosition != m_PosY) SetState(STATE_MOVE, m_NextMoveTime);

		m_NextMoveTime += RandomClass::rand(3000, 10000);
	}


	switch (m_State)
	{

	case MONSTER_STATE_EMBRACE:
		m_PosX += X_SPEED * time_delta * ((m_Direction == LEFT_FORWARD) ? -1 : 1);
		if (LEFT_X <= m_PosX && m_PosX <= RIGHT_X) SetState(MONSTER_STATE_NORMAL, curr_time);
		break;

	case STATE_MOVE:
		if (m_TargetYPosition < m_PosY)
		{
			m_PosY -= (int)time_delta * Y_SPEED;
			if (m_TargetYPosition >= m_PosY)
			{
				SetState(MONSTER_STATE_NORMAL, curr_time - (m_TargetYPosition - m_PosY) / Y_SPEED);
				m_PosY = m_TargetYPosition;
			}
		}
		else
		{
			m_PosY += (int)time_delta * Y_SPEED;
			if (m_TargetYPosition <= m_PosY)
			{
				SetState(MONSTER_STATE_NORMAL, curr_time - (m_PosY - m_TargetYPosition) / Y_SPEED);
				m_PosY = m_TargetYPosition;
			}
		}


	case MONSTER_STATE_NORMAL:
		m_PosX += X_SPEED * time_delta * ((m_Direction == LEFT_FORWARD) ? -1 : 1);

		if (m_PosX >= RIGHT_X)
		{
			m_PosX = 2 * RIGHT_X - m_PosX;
			m_Direction = LEFT_FORWARD;
		}

		if (m_PosX <= -RIGHT_X)
		{
			m_PosX = -2 * RIGHT_X - m_PosX;
			m_Direction = RIGHT_FORWARD;
		}
		break;

	case MONSTER_STATE_HIT:
	{
		const time_t avg_time = (curr_time - time_delta / 2) - m_StateStartTime;

		m_PosX += (m_HitVx * (knock_back_time - avg_time) / knock_back_time) * time_delta;

		const int start_y = m_PosY;
		const int target_y = m_PosY + (m_HitVy * (knock_back_time - avg_time) / knock_back_time
			- GRAVITY * avg_time) * time_delta;

		m_PosY += (m_HitVy * (knock_back_time - avg_time) / knock_back_time) * time_delta;
		m_PosY -= (GRAVITY * avg_time) * time_delta;

		if (m_PosX > RIGHT_X) m_PosX = RIGHT_X;
		else if (m_PosX < LEFT_X) m_PosX = LEFT_X;

		m_PosY = target_y;
		for (auto& ground_obj : ground)
		{
			m_PosY = max(m_PosY,
				ground_obj->IsColiided(GetGlobalRange().x1, GetGlobalRange().x2, start_y, target_y));
		}
		break;
	}
	}
}

bool MonsterBird::Frame(time_t curr_time, time_t time_delta)
{
	MonsterClass::Frame(curr_time, time_delta);
	switch (m_State)
	{
	case MONSTER_STATE_HIT:

		if (curr_time - m_StateStartTime >= 1000)
		{
			m_TargetYPosition = min(7, RandomClass::rand(-2, 8)) * 150'000 + 200'000;

			SetState(STATE_MOVE, m_StateStartTime + 1000);

			m_NextMoveTime = m_StateStartTime + RandomClass::rand(3000, 10000);
		}
		break;

	case MONSTER_STATE_DIE:
		return false;
	}

	return true;
}

int MonsterBird::GetVx()
{
	return DIR_WEIGHT(m_Direction, 1500);
}
