#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <stdint.h>


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
		vector<unique_ptr<class IGameObject> >& monsters_);

	void SetBaseTotalSpawnRate(uint32_t monsters_cnt_for_wave);

	/// @brief Sets the base spawn rates(probability) for individual monster types.
	///		   The sum of each probability should be 100.
	///
	/// @param p_octopus	An integer representing the spawn rate for Monster Type 0.
	/// @param p_duck		An integer representing the spawn rate for Monster Type 1.
	/// @param p_bird		An integer representing the spawn rate for Monster Type 2.
	/// @param p_stop		An integer representing the spawn rate for Monster Type 3.
	/// 
	void SetIndividualSpawnRate(uint32_t p_octopus,
		uint32_t p_duck, uint32_t p_bird, uint32_t p_stop);

private:
	int game_level_;
	vector<std::pair<time_t, int> > monster_spawn_schedule_;
	vector<std::pair<time_t, int> >::iterator schedule_iterator_;

	int base_total_spawn_rate_;
	int individual_spawn_rate_[4];
};