#pragma once

#include "MonsterClass.hh"

#include <vector>
#include <memory>

#include "global.hh"

class MonsterDuck : public MonsterClass
{
private:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;


public:
	MonsterDuck(direction_t direction, time_t created_time);
	~MonsterDuck() = default;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	// Return character's knock-back speed.
	virtual int GetVx();

private:
	time_t next_jump_time_;
};

class MonsterOctopus : public MonsterClass
{
private:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public: 
	MonsterOctopus(direction_t direction, time_t created_time);
	~MonsterOctopus() = default;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	// Return character's knock-back speed.
	virtual int GetVx();
};


class MonsterBird : public MonsterClass
{
private:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public:
	MonsterBird(direction_t direction, time_t created_time);
	~MonsterBird() = default;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	// Return character's knock-back speed.
	virtual int GetVx();

public:
	int target_y_pos_;
	time_t next_relocation_time_;
};


class MonsterStop : public MonsterClass
{
private:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public:
	MonsterStop(time_t created_time);
	~MonsterStop() = default;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	// Return character's knock-back speed.
	virtual int GetVx();
};