#include "../include/MonsterSpawnerClass.hh"

#include <algorithm>

#include "../include/Monsters.hh"
#include "../include/RandomClass.hh"

constexpr time_t kSpawnBeginTime = 5'000;
constexpr time_t kLevelupTerm = 20'000;

MonsterSpawnerClass::MonsterSpawnerClass() : game_level_(0)
{
	schedule_iterator_ = monster_spawn_schedule_.begin();
}

void MonsterSpawnerClass::Frame(time_t curr_time, time_t delta_time,
	vector<unique_ptr<class MonsterClass> >& monsters_)
{
	// When Gamelevel is up, plan which and when the monster will be spawned.
	const unsigned __int64 curr_game_level = (curr_time + (kLevelupTerm - kSpawnBeginTime)) / kLevelupTerm;

	for (; schedule_iterator_ != monster_spawn_schedule_.end()
		&& curr_time >= schedule_iterator_->first; schedule_iterator_++)
	{
		direction_t direction = RandomClass::rand(2) ? LEFT_FORWARD : RIGHT_FORWARD;

		switch (schedule_iterator_->second)
		{
		case 0:
			monsters_.emplace_back(new MonsterOctopus(direction, schedule_iterator_->second));
			break;
		case 1:
			monsters_.emplace_back(new MonsterDuck(direction, schedule_iterator_->second));
			break;
		case 2:
			monsters_.emplace_back(new MonsterBird(direction, schedule_iterator_->second));
			break;
		case 3:
			monsters_.emplace_back(new MonsterStop(schedule_iterator_->second));
			break;
		}
	}

	// TODO : the case where leveling is pended due to low frame rate. (over 10second)
	if (game_level_ < curr_game_level)
	{
		const time_t levelup_time = game_level_++ * kLevelupTerm + kSpawnBeginTime;

		monster_spawn_schedule_.clear();		
		
		for (int i = 0; i < 3 + game_level_; i++)
		{
			monster_spawn_schedule_.emplace_back(
				RandomClass::rand(levelup_time, levelup_time + kLevelupTerm), RandomClass::rand(4));
		}
		sort(monster_spawn_schedule_.begin(), monster_spawn_schedule_.end());

		schedule_iterator_ = monster_spawn_schedule_.begin();
	}
}
