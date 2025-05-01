#include "core/MonsterSpawnerClass.hh"

#include <algorithm>

#include "game-object/Monsters.hh"
#include "util/RandomClass.hh"

constexpr time_t kSpawnBeginTime = 5'000;
constexpr time_t kLevelupTerm = 20'000;

MonsterSpawnerClass::MonsterSpawnerClass() : game_level_(0)
{
	schedule_iterator_ = monster_spawn_schedule_.begin();
}

void MonsterSpawnerClass::Frame(time_t curr_time, time_t delta_time,
	vector<unique_ptr<class IGameObject> >& monsters_)
{
	// When Gamelevel is up, plan which and when the monster will be spawned.
	const unsigned __int64 curr_game_level = (curr_time + (kLevelupTerm - kSpawnBeginTime)) / kLevelupTerm;

	do
	{
		for (; schedule_iterator_ != monster_spawn_schedule_.end()
			&& curr_time >= schedule_iterator_->first; schedule_iterator_++)
		{
			direction_t direction = RandomClass::rand(2) ? LEFT_FORWARD : RIGHT_FORWARD;

			switch (schedule_iterator_->second)
			{
			case 0:
				monsters_.emplace_back(new MonsterOctopus(direction, schedule_iterator_->first));
				break;
			case 1:
				monsters_.emplace_back(new MonsterDuck(direction, schedule_iterator_->first));
				break;
			case 2:
				monsters_.emplace_back(new MonsterBird(direction, schedule_iterator_->first));
				break;
			case 3:
				monsters_.emplace_back(new MonsterStop(schedule_iterator_->first));
				break;
			}
		}

		if (game_level_ < curr_game_level)
		{
			const time_t levelup_time = (game_level_++) * kLevelupTerm + kSpawnBeginTime;

			monster_spawn_schedule_.clear();

			for (int i = 0; i < 6 + game_level_; i++)
			{
				monster_spawn_schedule_.emplace_back(
					RandomClass::rand(levelup_time + 100, levelup_time + kLevelupTerm - 100), RandomClass::rand(4));
			}
			sort(monster_spawn_schedule_.begin(), monster_spawn_schedule_.end());

			schedule_iterator_ = monster_spawn_schedule_.begin();
		}
	} while (game_level_ < curr_game_level);
}
