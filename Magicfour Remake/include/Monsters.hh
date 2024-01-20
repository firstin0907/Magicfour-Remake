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
	~MonsterDuck();

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

private:
	static constexpr int STATE_JUMP_READY = 4;
	static constexpr int STATE_JUMP = 5;

	int vy;
	time_t m_NextJumpTime;
};

class MonsterOctopus : public MonsterClass
{
public: 
	MonsterOctopus(direction_t direction, time_t created_time);
	~MonsterOctopus();

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);
};


class MonsterBird : public MonsterClass
{
public:
	MonsterBird(direction_t direction, time_t created_time);
	~MonsterBird();

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

public:
	// is the state 
	static constexpr int STATE_MOVE = 4;

	int m_TargetYPosition;
	int m_NextMoveTime;

};