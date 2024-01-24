#include "../include/CharacterClass.hh"

#include <Windows.h>

#include "../include/global.hh"

#include "../include/InputClass.hh"
#include "../include/AnimatedObjectClass.hh"
#include "../include/SkillObjects.hh"
#include "../include/GroundClass.hh"

constexpr int SKILL_COOLTIME = 500;
constexpr int INVINCIBLE_TIME = 2000;
constexpr int WALK_SPD = 700, RUN_SPD = 1300;

CharacterClass::CharacterClass(int pos_x, int pos_y)
	: pos_x(pos_x), pos_y(pos_y), pos_yv(0), jump_cnt(0)
{
	m_JumpAnimationData = make_unique<AnimatedObjectClass>("data\\motion\\jump_motion.txt");
	m_FallAnimationData = make_unique<AnimatedObjectClass>("data\\motion\\fall_motion.bvh");
	m_WalkAnimationData = make_unique<AnimatedObjectClass>("data\\motion\\walk_motion.txt");
	m_RunAnimationData = make_unique<AnimatedObjectClass>("data\\motion\\run_motion.txt");
	m_SkillAnimationData = make_unique<AnimatedObjectClass>("data\\motion\\skill_motion.bvh");

	SetState(CHARACTER_STATE_NORMAL, 0);

	m_Skill[0] = 4;
	m_Skill[1] = 3;
	m_Skill[2] = 1;
	m_Skill[3] = 2;

	m_TimeInvincibleEnd = 0;
}

bool CharacterClass::Frame(time_t time_delta, time_t curr_time, InputClass* input,
	vector<unique_ptr<class SkillObjectClass> >& skill_objs,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	bool is_walk = false;

	if (input->IsKeyPressed(DIK_LEFT))
	{
		if (m_State == CHARACTER_STATE_STOP && m_Direction != LEFT_FORWARD)
		{
			SetState(CHARACTER_STATE_NORMAL, curr_time);
		}
		m_Direction = LEFT_FORWARD;

		is_walk = !is_walk;
	}

	if (input->IsKeyPressed(DIK_RIGHT))
	{
		if (m_State == CHARACTER_STATE_STOP && m_Direction != RIGHT_FORWARD)
		{
			SetState(CHARACTER_STATE_NORMAL, curr_time);
		}
		m_Direction = RIGHT_FORWARD;

		is_walk = !is_walk;
	}

	if (input->IsKeyDown(DIK_Z)) UseSkill(curr_time, skill_objs);


	if (input->IsKeyDown(DIK_X))
	{
		for (int i = 3; i >= 0; i--)
		{
			if (m_Skill[i])
			{
				m_Skill[i] = 0;
				break;
			}
		}
	}

	// jump attempt
	if (jump_cnt <= 1 && input->IsKeyDown(DIK_UP) && m_State != CHARACTER_STATE_SPELL && m_State != CHARACTER_STATE_HIT)
	{
		pos_yv = 2'800, jump_cnt++;
		if (m_State == CHARACTER_STATE_RUN || m_State == CHARACTER_STATE_RUNJUMP) SetState(CHARACTER_STATE_RUNJUMP, curr_time);
		else SetState(CHARACTER_STATE_JUMP, curr_time);
	}
	else
	{
		const int start_y = pos_y;
		const int target_y = pos_y + time_delta * (pos_yv - (GRAVITY / 2) * time_delta);
		pos_yv -= GRAVITY * time_delta;

		pos_y = target_y;
		for (auto& ground_obj : ground)
		{
			pos_y = max(pos_y,
				ground_obj->IsColiided(range.x1 + pos_x, range.x2 + pos_x, start_y, target_y));
		}

		if (pos_y != target_y)
		{
			pos_yv = jump_cnt = 0;
		}
	}



	switch (m_State)
	{
	case CHARACTER_STATE_JUMP:
		if(is_walk)
		{
			pos_x += DIR_WEIGHT(m_Direction, WALK_SPD) * (int)time_delta;
			pos_x = SATURATE(LEFT_X, pos_x, RIGHT_X);
		}

		if (curr_time - m_StateStartTime >= 1000) SetState(CHARACTER_STATE_NORMAL, curr_time);
		else if (jump_cnt == 0 && is_walk) SetState(CHARACTER_STATE_WALK, curr_time);
		break;

	case CHARACTER_STATE_RUNJUMP:
		if (is_walk)
		{
			pos_x += DIR_WEIGHT(m_Direction, RUN_SPD) * (int)time_delta;
			pos_x = SATURATE(LEFT_X, pos_x, RIGHT_X);
		}

		if (jump_cnt == 0)
		{
			if (is_walk) SetState(CHARACTER_STATE_RUN, curr_time);
			else m_State = CHARACTER_STATE_JUMP;
		}
		break;

	case CHARACTER_STATE_NORMAL:
		if (is_walk)
		{
			SetState(CHARACTER_STATE_WALK, curr_time);
			pos_x += DIR_WEIGHT(m_Direction, WALK_SPD) * (int)time_delta;
			pos_x = SATURATE(LEFT_X, pos_x, RIGHT_X);
		}
		break;

	case CHARACTER_STATE_WALK:
		if (!is_walk) SetState(CHARACTER_STATE_STOP, curr_time);
		else
		{
			pos_x += DIR_WEIGHT(m_Direction, WALK_SPD) * (int)time_delta;
			pos_x = SATURATE(LEFT_X, pos_x, RIGHT_X);
		}
		break;

	case CHARACTER_STATE_RUN:
		if (!is_walk) SetState(CHARACTER_STATE_STOP, curr_time);
		else
		{
			pos_x += DIR_WEIGHT(m_Direction, RUN_SPD) * (int)time_delta;
			pos_x = SATURATE(LEFT_X, pos_x, RIGHT_X);
		}
		break;

	case CHARACTER_STATE_STOP:
		if (curr_time - m_StateStartTime >= 150)
			SetState(CHARACTER_STATE_NORMAL, m_StateStartTime + 150);
		else if(is_walk) SetState(CHARACTER_STATE_RUN, curr_time);
		break;


	case CHARACTER_STATE_SPELL:
		OnSkill(curr_time, skill_objs);
		break;


	case CHARACTER_STATE_HIT:
		pos_x += (int)time_delta * pos_xv;
		if (curr_time - m_StateStartTime >= 500)
		{
			SetState(CHARACTER_STATE_SLIP, m_StateStartTime);
		}
		break;

	case CHARACTER_STATE_SLIP:
		if (curr_time - m_StateStartTime >= 1000)
		{
			SetState(CHARACTER_STATE_NORMAL, m_StateStartTime + 1000);
		}
	}

	return true;
}

void CharacterClass::GetShapeMatrices(time_t curr_time, std::vector<XMMATRIX>& shape_matrices)
{
	XMMATRIX root_transform = XMMatrixRotationY(XM_PI * 0.65f * ((m_Direction == LEFT_FORWARD) ? -1 : 1));
	//XMMATRIX root_transform = XMMatrixIdentity();

	float state_elapsed_seconds = (curr_time - m_StateStartTime) / 1000.0f;

	switch (m_State)
	{
	case CHARACTER_STATE_NORMAL:
	case CHARACTER_STATE_STOP:
		m_RunAnimationData->UpdateGlobalMatrices(0, root_transform, shape_matrices);
		break;

	case CHARACTER_STATE_WALK:
		m_WalkAnimationData->UpdateGlobalMatrices(
			state_elapsed_seconds / 0.00333333, root_transform, shape_matrices);
		break;

	case CHARACTER_STATE_RUN:
		m_RunAnimationData->UpdateGlobalMatrices(
			state_elapsed_seconds / 0.00333333, root_transform, shape_matrices);
		break;

	case CHARACTER_STATE_JUMP:
	case CHARACTER_STATE_RUNJUMP:
		if(curr_time - m_StateStartTime > 90)
			m_JumpAnimationData->UpdateGlobalMatrices(
				20 + state_elapsed_seconds / 0.00333333, root_transform, shape_matrices);
		else
			m_JumpAnimationData->UpdateGlobalMatrices(
				43 + state_elapsed_seconds / 0.00833333, root_transform, shape_matrices);
		break;

	case CHARACTER_STATE_SPELL:
		root_transform *= XMMatrixRotationY(XM_PI * 0.5f);
		m_SkillAnimationData->UpdateGlobalMatrices(
			state_elapsed_seconds / 0.00133333, root_transform, shape_matrices);
		break;

	case CHARACTER_STATE_HIT:
	case CHARACTER_STATE_SLIP:
		m_FallAnimationData->UpdateGlobalMatrices(
			50 + state_elapsed_seconds / 0.00433333, root_transform, shape_matrices);
		break;

	}
}

XMMATRIX CharacterClass::GetLocalWorldMatrix()
{
	return XMMatrixTranslation(pos_x * SCOPE, pos_y * SCOPE, 0);
}

void CharacterClass::OnCollided(time_t curr_time, int vx)
{
	if (m_TimeInvincibleEnd < curr_time)
	{
		SetState(CHARACTER_STATE_HIT, curr_time);
		m_Direction = (vx > 0) ? LEFT_FORWARD : RIGHT_FORWARD;
		m_HitVx = pos_xv = vx / 3;

		pos_yv = 1500;
		m_TimeInvincibleEnd = m_StateStartTime + INVINCIBLE_TIME;
	}
}

void CharacterClass::OnSkill(time_t curr_time,
	vector<unique_ptr<class SkillObjectClass> >& skill_objs)
{
	const time_t state_elapsed_time = curr_time - m_StateStartTime;

	switch (m_SkillUsed)
	{
	case 0:
		// When Skill Ended.
		if (curr_time >= m_TimeSkillEnded)
		{
			SetState(CHARACTER_STATE_NORMAL, m_TimeSkillEnded);
		}

		if (m_SkillState == 0 && state_elapsed_time >= 100)
		{
			skill_objs.emplace_back(new SkillObjectBasic(pos_x + DIR_WEIGHT(m_Direction, 185000), pos_y,
				DIR_WEIGHT(m_Direction, 100), m_StateStartTime + 100));
			m_SkillState = 1;
		}

		break;

	case 1:
		
		if (m_SkillState == 0 && state_elapsed_time >= 100)
		{
			skill_objs.emplace_back(new SkillObjectSpear(pos_x, pos_y, 4000, -4000, m_StateStartTime + 100));
			skill_objs.emplace_back(new SkillObjectSpear(pos_x, pos_y, 2000, -4000, m_StateStartTime + 100));
			skill_objs.emplace_back(new SkillObjectSpear(pos_x, pos_y, 0, -4000, m_StateStartTime + 100));
			skill_objs.emplace_back(new SkillObjectSpear(pos_x, pos_y, -2000, -4000, m_StateStartTime + 100));
			skill_objs.emplace_back(new SkillObjectSpear(pos_x, pos_y, -4000, -4000, m_StateStartTime + 100));

			m_SkillState = 1;
		}

		// When Skill Ended.
		if (state_elapsed_time >= 300)
		{
			SetState(CHARACTER_STATE_JUMP, m_StateStartTime - 200);
		}
			
		break;

	case 2:

		for (; m_SkillState < 3 && state_elapsed_time >= m_SkillState * 100 + 100; m_SkillState++)
		{
			skill_objs.emplace_back(new SkillObjectBead(
				pos_x, pos_y + 300000, DIR_WEIGHT(m_Direction, 1200),
				m_SkillState * 200, m_StateStartTime + m_SkillState * 100 + 100));
		}

		// When Skill Ended.
		if (state_elapsed_time >= 300)
		{
			SetState(CHARACTER_STATE_NORMAL, m_TimeSkillEnded);
		}

		break;

	case 3:
		if (m_SkillState == 0 && state_elapsed_time >= 100)
		{
			skill_objs.emplace_back(new SkillObjectLeg(
				pos_x + DIR_WEIGHT(m_Direction, 3'000), m_StateStartTime + 100));
			m_SkillState = 1;
		}


		// When Skill Ended.
		if (state_elapsed_time >= 300)
		{
			SetState(CHARACTER_STATE_NORMAL, m_TimeSkillEnded);
		}
		break;
	}
}

bool CharacterClass::UseSkill(time_t curr_time,
	vector<unique_ptr<class SkillObjectClass> >& skill_objs)
{
	if (curr_time < m_TimeSkillAvailable)
	{
		// TODO: message print
		return false;
	}

	m_SkillUsed = 0;
	for (int i = 3; i >= 0; i--)
	{
		if (m_Skill[i])
		{
			m_SkillUsed = m_Skill[i];
			break;
		}
	}

	m_SkillState = 0;
	SetState(CHARACTER_STATE_SPELL, curr_time);

	switch (m_SkillUsed)
	{
	case 0:
		m_TimeSkillEnded = m_StateStartTime + 300;
		break;

	case 1:
		if (pos_yv == 0 && jump_cnt == 0) pos_yv = 3'600;

		m_TimeSkillEnded = m_StateStartTime + 300;
		break;

	case 2:
		m_TimeSkillEnded = m_StateStartTime + 300;
		break;

	case 3:
		m_TimeSkillEnded = m_StateStartTime + 300;
		break;
	}

	m_TimeSkillAvailable = m_TimeSkillEnded + SKILL_COOLTIME;

	return true;
}