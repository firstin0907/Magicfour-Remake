#include "../include/SkillObjectClass.hh"

#include "../include/MonsterClass.hh"

SkillObjectClass::SkillObjectClass(int pos_x, int pos_y, rect_t range)
	: pos_x(pos_x), pos_y(pos_y), m_Range(range)
{
	
}

bool SkillObjectClass::OnCollided(
	MonsterClass* monster, time_t collided_time)
{
	auto entry = m_hitMonsters.find(monster->GetId());

	// If that monster is hit with this instance first time in lastest 1000ms,
	// this collision is valid.
	if (entry == m_hitMonsters.end() || collided_time - entry->second >= 1000 )
	{
		// Add entry
		m_hitMonsters[monster->GetId()] = collided_time;
		return true;
	}
	return false;

	// Processing for monster instance or this instance is conducted
	// in the overrided child class.
}
