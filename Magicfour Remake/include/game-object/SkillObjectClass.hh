#pragma once

#include <map>
#include <utility>
#include <memory>
#include <vector>

#include "core/global.hh"
#include "core/RigidbodyClass.hh"

enum class SkillObjectState
{
	kEmbryo, kNormal, kDie,
	kSpearOnGround,
	kBeadOneHit
};

class SkillObjectClass : public RigidbodyClass<SkillObjectState>
{
public:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

	SkillObjectClass(int pos_x, int pos_y, rect_t range,
		int vx, int vy, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground) = 0;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) = 0;

	// Check if this instance is on collidable state.
	virtual bool IsColliable() const override final
	{
		return !(state_ == SkillObjectState::kEmbryo || state_ == SkillObjectState::kDie);
	};

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time) = 0;

	virtual class ModelClass* GetModel() = 0;

protected:

	int skill_level_;

	time_t created_time_;

	// Monster who has been collided with this SkillObjectClass instance.
	// Key = id of monster instance.
	// Value = the time when collided with that monster.
	std::map<int, time_t> hitMonsters_;
};