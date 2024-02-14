#pragma once

#include <vector>
#include <utility>
#include <memory>


class MonsterSpawnerClass
{
private:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public:
	MonsterSpawnerClass();

	void Frame(time_t curr_time, time_t delta_time,
		vector<unique_ptr<class MonsterClass> >& monsters_);

private:
	int game_level_;
	vector<std::pair<time_t, int> > monster_spawn_schedule_;
	vector<std::pair<time_t, int> >::iterator schedule_iterator_;
};