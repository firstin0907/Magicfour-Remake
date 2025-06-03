#include "core/MonsterSpawnerClass.hh"

#include <algorithm>

#include "game-object/Monsters.hh"
#include "util/RandomClass.hh"

constexpr time_t kSpawnBeginTime = 5'000;
constexpr time_t kLevelupTerm = 20'000;

MonsterSpawnerClass::MonsterSpawnerClass() :
	game_level_(0),
	base_total_spawn_rate_(6),
	individual_spawn_rate_{25, 25, 25, 25}
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

			for (int i = 0; i < base_total_spawn_rate_ + game_level_; i++)
			{
				int			 monster_type = -1;
				const int    monster_type_rv = RandomClass::rand(100);
				const time_t creation_time = levelup_time + RandomClass::rand(100LL, kLevelupTerm - 100);

				for (int j = 3; j >= 0; j--)
				{
					if (monster_type_rv < individual_spawn_rate_[j]) monster_type = j;
				}

				if (monster_type > 0)
				{
					monster_spawn_schedule_.emplace_back(creation_time, monster_type);
				}
			}
			sort(monster_spawn_schedule_.begin(), monster_spawn_schedule_.end());

			schedule_iterator_ = monster_spawn_schedule_.begin();
		}
	} while (game_level_ < curr_game_level);
}

void MonsterSpawnerClass::SetBaseTotalSpawnRate(uint32_t monsters_cnt_for_wave)
{
	base_total_spawn_rate_ = monsters_cnt_for_wave;
}

void MonsterSpawnerClass::SetIndividualSpawnRate(uint32_t p_octopus, uint32_t p_duck, uint32_t p_bird, uint32_t p_stop)
{
	individual_spawn_rate_[0] = p_octopus;
	individual_spawn_rate_[1] = p_octopus + p_duck;
	individual_spawn_rate_[2] = p_octopus + p_duck + p_bird;
	individual_spawn_rate_[3] = p_octopus + p_duck + p_bird + p_stop;
}
