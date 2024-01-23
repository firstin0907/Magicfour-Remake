#pragma once

#include <map>
#include <utility>
#include <memory>
#include <vector>

#include "global.hh"

using namespace std;

class SkillObjectClass
{
public:
	SkillObjectClass(int pos_x, int pos_y, rect_t range);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground) = 0;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) = 0;

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time) = 0;

	inline rect_t GetGlobalRange() { return m_Range.add(pos_x, pos_y); }
	inline XMMATRIX GetRangeRepresentMatrix() { return m_Range.add(pos_x, pos_y).toMatrix(); }

	virtual class ModelClass* GetModel() = 0;

protected:
	int pos_x, pos_y;	// Position of Instance	
	rect_t m_Range;		// Range of Instance

	// Monster who has been collided with this SkillObjectClass instance.
	// Key = id of monster instance.
	// Value = the time when collided with that monster.
	map<int, time_t>					m_hitMonsters;
};