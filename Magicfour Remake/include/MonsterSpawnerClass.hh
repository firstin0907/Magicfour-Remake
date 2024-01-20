#pragma once

#include <vector>
#include <utility>
#include <memory>

using namespace std;

class MonsterSpawnerClass
{
public:
	MonsterSpawnerClass();

	void Frame(time_t curr_time, time_t delta_time,
		vector<unique_ptr<class MonsterClass> >& m_Monsters);

private:
	int m_GameLevel;
	vector<pair<time_t, int> > m_MonsterSpawnSchedule;
	vector<pair<time_t, int> >::iterator m_ScheduleIterator;
};