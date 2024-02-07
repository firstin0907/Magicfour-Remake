#pragma once

#include "MonsterClass.hh"

#include <vector>
#include <memory>

#include "global.hh"

using namespace std;

class MonsterDuck : public MonsterClass
{
public:
	MonsterDuck(direction_t direction, time_t created_time);
	~MonsterDuck() = default;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	// Return character's knock-back speed.
	virtual int GetVx();

private:
	time_t m_NextJumpTime;
};

class MonsterOctopus : public MonsterClass
{
public: 
	MonsterOctopus(direction_t direction, time_t created_time);
	~MonsterOctopus() = default;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	// Return character's knock-back speed.
	virtual int GetVx();
};


class MonsterBird : public MonsterClass
{
public:
	MonsterBird(direction_t direction, time_t created_time);
	~MonsterBird() = default;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	// Return character's knock-back speed.
	virtual int GetVx();

public:
	int m_TargetYPosition;
	int m_NextMoveTime;

};