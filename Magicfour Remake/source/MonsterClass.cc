#include "../include/MonsterClass.hh"

#include "../include/global.hh"

int MonsterClass::MonsterCount = 0;

MonsterClass::MonsterClass(int hp, direction_t direction, rect_t range)
	: m_MaxHp(hp), m_Hp(hp), m_Direction(direction), m_Id(++MonsterCount), m_State(MONSTER_STATE_NORMAL),
	m_Range(range)
{
	m_HitVx = m_HitVy = 0;
}

MonsterClass::~MonsterClass()
{
}

bool MonsterClass::Frame(time_t curr_time, time_t time_delta)
{
	return true;
}

void MonsterClass::SetState(const int state, time_t time)
{
	m_State = state;
	m_StateStartTime = time;
}

bool MonsterClass::Damage(const int amount, time_t time, int vx, int vy)
{
	m_Hp -= amount;

	m_StateStartTime = time;
	m_State = (m_Hp > 0) ? MONSTER_STATE_HIT : MONSTER_STATE_DIE;

	m_HitVx = vx, m_HitVy = vy;
	if (m_HitVx > 0) m_Direction = LEFT_FORWARD;
	else if (m_HitVx < 0) m_Direction = RIGHT_FORWARD;

	return m_Hp > 0;
}

XMMATRIX MonsterClass::GetLocalWorldMatrix()
{
	return XMMatrixTranslation(m_PosX * SCOPE, m_PosY * SCOPE, 0);
}
