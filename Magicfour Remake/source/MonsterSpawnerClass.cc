#include "../include/MonsterSpawnerClass.hh"

#include <algorithm>

#include "../include/Monsters.hh"
#include "../include/RandomClass.hh"

constexpr time_t SPAWN_BEGIN_TIME = 5'000;
constexpr time_t LEVELUP_TERM = 20'000;

MonsterSpawnerClass::MonsterSpawnerClass() : gameLevel_(0)
{
	scheduleIterator_ = monsterSpawnSchedule_.begin();
}

void MonsterSpawnerClass::Frame(time_t curr_time, time_t delta_time,
	vector<unique_ptr<class MonsterClass> >& monsters_)
{
	// When Gamelevel is up, plan which and when the monster will be spawned.
	const unsigned __int64 curr_game_level = (curr_time + (LEVELUP_TERM - SPAWN_BEGIN_TIME)) / LEVELUP_TERM;

	for (; scheduleIterator_ != monsterSpawnSchedule_.end()
		&& curr_time <= scheduleIterator_->first; scheduleIterator_++)
	{
		direction_t direction = RandomClass::rand(2) ? LEFT_FORWARD : RIGHT_FORWARD;

		switch (scheduleIterator_->second)
		{
		case 0:
			monsters_.emplace_back(new MonsterOctopus(direction, scheduleIterator_->second));
			break;
		case 1:
			monsters_.emplace_back(new MonsterDuck(direction, scheduleIterator_->second));
			break;
		case 2:
			monsters_.emplace_back(new MonsterBird(direction, scheduleIterator_->second));
			break;
		}
	}

	// TODO : the case where leveling is pended due to low frame rate. (over 10second)
	if (gameLevel_ < curr_game_level)
	{
		const time_t levelup_time = gameLevel_++ * LEVELUP_TERM + SPAWN_BEGIN_TIME;

		monsterSpawnSchedule_.clear();		
		
		for (int i = 0; i < 3 + gameLevel_; i++)
		{
			monsterSpawnSchedule_.emplace_back(
				RandomClass::rand(levelup_time, levelup_time + LEVELUP_TERM), RandomClass::rand(3));
		}
		sort(monsterSpawnSchedule_.begin(), monsterSpawnSchedule_.end());

		scheduleIterator_ = monsterSpawnSchedule_.begin();
	}
}
