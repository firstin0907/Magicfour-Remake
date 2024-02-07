#include "../include/MonsterClass.hh"

#include "../include/global.hh"

int MonsterClass::MonsterCount = 0;

MonsterClass::MonsterClass(Point2d position, direction_t direction,
	int type, int hp,  rect_t range)
	: RigidbodyClass<MonsterState>(position, range, direction),
	m_MaxHp(hp), m_Hp(hp), m_prevHp(hp), m_Id(++MonsterCount), m_Type(type)
{
	SetState(MonsterState::kEmbryo, 0);
	m_HitVx = m_HitVy = 0;
}

bool MonsterClass::Frame(time_t curr_time, time_t time_delta)
{
	m_prevHp = (m_prevHp * 30 + m_Hp) / 31;
	return true;
}


bool MonsterClass::Damage(const int amount, time_t damaged_time, int vx, int vy)
{
	m_Hp -= amount;

	SetState((m_Hp > 0) ? MonsterState::kHit : MonsterState::kDie, damaged_time);

	m_HitVx = vx, m_HitVy = vy;
	if (m_HitVx > 0) direction_ = LEFT_FORWARD;
	else if (m_HitVx < 0) direction_ = RIGHT_FORWARD;

	return m_Hp > 0;
}