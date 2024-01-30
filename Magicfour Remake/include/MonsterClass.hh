#pragma once

#include <time.h>
#include <DirectXMath.h>

#include <vector>
#include <memory>

#include "global.hh"

using namespace DirectX;
using namespace std;

#define MONSTER_STATE_EMBRACE	0
#define MONSTER_STATE_NORMAL	1
#define MONSTER_STATE_HIT		2
#define MONSTER_STATE_DIE		3

class MonsterClass
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

	int m_PosX, m_PosY; // Position of monster instance.
	rect_t m_Range;		// Range of monster Instance.

	// Knock-back speed which is set if some skill objects hit this monster.
	int m_HitVx, m_HitVy;

	// State of monster. The valid state is defined in MonsterClass.hh.
	int m_State;

	// When m_State variable has been changed lastly?
	time_t m_StateStartTime;

	// Which direction this monster instance is heading?
	direction_t m_Direction;


public:
	MonsterClass(int hp, direction_t direction, rect_t range);
	~MonsterClass();

	inline int GetId() { return m_Id; }

	inline float GetHpRatio() { return m_Hp / (float)m_MaxHp;  }

	virtual int GetVx() = 0;

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<unique_ptr<class GroundClass> >& ground) = 0;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) = 0;

	inline int GetState() { return m_State; }
	void SetState(const int state, time_t time);

	bool Damage(const int amount, time_t time, int vx, int vy);

	XMMATRIX GetLocalWorldMatrix();

	inline rect_t GetGlobalRange() { return m_Range.add(m_PosX, m_PosY); }
	inline XMMATRIX GetRangeRepresentMatrix() {
		return m_Range.add(m_PosX, m_PosY).toMatrix();
	}
};

