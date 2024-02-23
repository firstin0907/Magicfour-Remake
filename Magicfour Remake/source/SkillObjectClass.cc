#include "../include/SkillObjectClass.hh"

#include "../include/MonsterClass.hh"

SkillObjectClass::SkillObjectClass(int pos_x, int pos_y,
	rect_t range, int vx, int vy, int skill_level, time_t created_time)
	: RigidbodyClass<SkillObjectState>(
		Point2d(pos_x, pos_y), rect_t(range), LEFT_FORWARD, Vector2d(vx, vy), Vector2d(0, 0))
{
	SetState(SkillObjectState::kNormal, created_time);	
	skill_level_ = skill_level;
}

bool SkillObjectClass::OnCollided(
	MonsterClass* monster, time_t collided_time)
{
	auto entry = hitMonsters_.find(monster->GetId());

	// If that monster is hit with this instance first time in lastest 1000ms,
	// this collision is valid.
	if (entry == hitMonsters_.end() || collided_time - entry->second >= 1000 )
	{
		// Add entry
		hitMonsters_[monster->GetId()] = collided_time;
		return true;
	}
	return false;

	// Processing for monster instance or this instance is conducted
	// in the overrided child class.
}
