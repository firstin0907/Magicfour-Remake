#include "MonsterSpawnerClass.hh"

#include <algorithm>

#include "Monsters.hh"
#include "RandomClass.hh"

constexpr time_t SPAWN_BEGIN_TIME = 5'000;
constexpr time_t LEVELUP_TERM = 20'000;

MonsterSpawnerClass::MonsterSpawnerClass() : m_GameLevel(0)
{
	m_ScheduleIterator = m_MonsterSpawnSchedule.begin();
}

void MonsterSpawnerClass::Frame(time_t curr_time, time_t delta_time,
	vector<unique_ptr<class MonsterClass> >& m_Monsters)
{
	// When Gamelevel is up, plan which and when the monster will be spawned.
	const unsigned __int64 curr_game_level = (curr_time + (LEVELUP_TERM - SPAWN_BEGIN_TIME)) / LEVELUP_TERM;

	for (; m_ScheduleIterator != m_MonsterSpawnSchedule.end()
		&& curr_time <= m_ScheduleIterator->first; m_ScheduleIterator++)
	{
		direction_t direction = RandomClass::rand(2) ? LEFT_FORWARD : RIGHT_FORWARD;

		switch (m_ScheduleIterator->second)
		{
		case 0:
			m_Monsters.emplace_back(new MonsterOctopus(direction, m_ScheduleIterator->second));
			break;
		case 1:
			m_Monsters.emplace_back(new MonsterDuck(direction, m_ScheduleIterator->second));
			break;
		case 2:
			m_Monsters.emplace_back(new MonsterBird(direction, m_ScheduleIterator->second));
			break;
		}
	}

	// TODO : the case where leveling is pended due to low frame rate. (over 10second)
	if (m_GameLevel < curr_game_level)
	{
		const time_t levelup_time = m_GameLevel++ * LEVELUP_TERM + SPAWN_BEGIN_TIME;

		m_MonsterSpawnSchedule.clear();		
		
		for (int i = 0; i < 3 + m_GameLevel; i++)
		{
			m_MonsterSpawnSchedule.emplace_back(
				RandomClass::rand(levelup_time, levelup_time + LEVELUP_TERM), RandomClass::rand(3));
		}
		sort(m_MonsterSpawnSchedule.begin(), m_MonsterSpawnSchedule.end());

		m_ScheduleIterator = m_MonsterSpawnSchedule.begin();
	}
}
