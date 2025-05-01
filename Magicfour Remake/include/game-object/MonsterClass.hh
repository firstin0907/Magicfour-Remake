#pragma once

#include <time.h>
#include <DirectXMath.h>

#include <vector>
#include <memory>

#include "core/global.hh"
#include "core/RigidbodyClass.hh"

enum class MonsterState
{
	kEmbryo, kNormal, kHit, kDie,
	kDuckJump, kDuckJumpReady,
	kBirdMove,
	kStopEmbryo, kStopOnGround
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
		int type, int hp, rect_t range, time_t created_time);
	~MonsterClass() = default;

	inline int GetId() { return id_; }

	inline int GetType() { return type_; }

	inline float GetPrevHpRatio() { return prev_hp_ / (float)max_hp_; }
	inline float GetHpRatio() { return hp_ / (float)max_hp_; }

	virtual int GetVx() = 0;

	// Change the location for one frame.
	virtual void FrameMove(time_t curr_time, time_t time_delta, const std::vector<class GroundClass>& ground) = 0;

	// Proceed the logic for one frame, and return this is still alive.
	virtual bool Frame(time_t curr_time, time_t time_delta) = 0;

	// Check if this instance is on collidable state.
	virtual bool IsColliable() const override final { return state_ != MonsterState::kEmbryo && state_ != MonsterState::kDie;  }

	bool Damage(const int amount, time_t damaged_time, int vx, int vy);
	inline bool DamageWithNoKnockBack(const int amount, time_t damaged_time)
	{
		hp_ -= amount;
		if(hp_ <= 0) SetState(MonsterState::kDie, damaged_time);
		return hp_ > 0;
	};
};

