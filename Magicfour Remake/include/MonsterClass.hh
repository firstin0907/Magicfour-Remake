#pragma once

#include <time.h>
#include <DirectXMath.h>

#include <vector>
#include <memory>

#include "global.hh"
#include "RigidbodyClass.hh"

enum class MonsterState
{
	kEmbryo, kNormal, kHit, kDie,
	kDuckJump, kDuckJumpReady,
	kBirdMove
};

class MonsterClass : public RigidbodyClass<MonsterState>
{
private:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

private:
	// It is used to determine the ID(id_) of newly create MonsterClass instance.
	// The monster's id is set ++monster_count_.
	static int monster_count_;

protected:
	// The unique ID this monster instance has.
	// It is used for SKillObjectClass to identify which monsters have been hit already
	// and prevent double-damage to the same monster,
	// which is caused because skill object class can penetrate monster. 
	const int id_;

	// Health point of this monster instance.
	// The monster whose hp is below then zero is to die.
	int hp_, max_hp_;

	// Previous HP. This decrease toward current hp(hp_)
	// This is used to print white portion of monster hp bar. 
	int prev_hp_;

	// Which species this mosnter instance is.
	int type_;

	// Knock-back speed which is set if some skill objects hit this monster.
	int hit_vx_, hit_vy_;


public:
	MonsterClass(Point2d position, direction_t direction,
		int type, int hp, rect_t range);
	~MonsterClass() = default;

	inline int GetId() { return id_; }

	inline int GetType() { return type_; }

	inline float GetPrevHpRatio() { return prev_hp_ / (float)max_hp_; }
	inline float GetHpRatio() { return hp_ / (float)max_hp_; }

	virtual int GetVx() = 0;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground) = 0;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) = 0;

	bool Damage(const int amount, time_t damaged_time, int vx, int vy);
};

