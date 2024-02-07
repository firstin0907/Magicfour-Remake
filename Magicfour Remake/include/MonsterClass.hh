#pragma once

#include <time.h>
#include <DirectXMath.h>

#include <vector>
#include <memory>

#include "global.hh"
#include "RigidbodyClass.hh"

using namespace DirectX;
using namespace std;

enum class MonsterState
{
	kEmbryo, kNormal, kHit, kDie,
	kDuckJump, kDuckJumpReady,
	kBirdMove
};

class MonsterClass : public RigidbodyClass<MonsterState>
{
private:
	// It is used to determine the ID(m_Id) of newly create MonsterClass instance.
	// The monster's id is set ++MonsterCount.
	static int MonsterCount;

protected:
	// The unique ID this monster instance has.
	// It is used for SKillObjectClass to identify which monsters have been hit already
	// and prevent double-damage to the same monster,
	// which is caused because skill object class can penetrate monster. 
	const int m_Id;

	// Health point of this monster instance.
	// The monster whose hp is below then zero is to die.
	int m_Hp, m_MaxHp;

	// Previous HP. This decrease toward current hp(m_Hp)
	// This is used to print white portion of monster hp bar. 
	int m_prevHp;

	// Which species this mosnter instance is.
	int m_Type;

	// Knock-back speed which is set if some skill objects hit this monster.
	int m_HitVx, m_HitVy;


public:
	MonsterClass(Point2d position, direction_t direction,
		int type, int hp, rect_t range);
	~MonsterClass() = default;

	inline int GetId() { return m_Id; }

	inline int GetType() { return m_Type; }

	inline float GetPrevHpRatio() { return m_prevHp / (float)m_MaxHp; }
	inline float GetHpRatio() { return m_Hp / (float)m_MaxHp; }

	virtual int GetVx() = 0;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground) = 0;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) = 0;

	bool Damage(const int amount, time_t damaged_time, int vx, int vy);
};

