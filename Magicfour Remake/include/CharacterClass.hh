#pragma once
#include <DirectXMath.h>

#include <memory>
#include <vector>

#include "global.hh"

using namespace std;
using namespace DirectX;

#define CHARACTER_STATE_NORMAL			1
#define CHARACTER_STATE_JUMP			2
#define CHARACTER_STATE_WALK			3
#define CHARACTER_STATE_STOP			4
#define CHARACTER_STATE_RUN				5
#define CHARACTER_STATE_RUNJUMP			6
#define CHARACTER_STATE_SPELL			7
#define CHARACTER_STATE_HIT				8
#define CHARACTER_STATE_SLIP			9
#define CHARACTER_STATE_DIE				20


class CharacterClass
{
public:
	CharacterClass(int pos_x, int pos_y);

	bool Frame(time_t time_delta, time_t curr_time, class InputClass* input,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs,
		const vector<unique_ptr<class GroundClass> >& ground
		);
	
	void GetShapeMatrices(time_t curr_time, vector<XMMATRIX>& shape_matrices);
	
	inline int GetPosX() { return pos_x; };

	template <int index>
	int GetSkill();

	XMMATRIX GetLocalWorldMatrix();

	inline rect_t GetGlobalRange() { return range.add(pos_x, pos_y); }
	
	// for debug
	inline XMMATRIX GetRangeRepresentMatrix()
	{
		return range.add(pos_x, pos_y).toMatrix();
	}

	// If character have been killed because of this collision,
	// this function returns false. Otherwise, returns true.
	bool OnCollided(time_t curr_time, int vx);

	float GetCooltimeGaugeRatio(time_t curr_time);

private:
	inline void SetState(int state, time_t start_time)
	{
		m_State = state;
		m_StateStartTime = start_time;
	}

	void OnSkill(time_t curr_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs);

	bool UseSkill(time_t curr_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs);

private:
	static constexpr rect_t range = { -50000, 0, 50000, 400000 };

	direction_t m_Direction;

	int m_HitVx;

	int pos_x, pos_y, pos_xv, pos_yv;
	int jump_cnt;

	int m_Skill[4] = { 0 };

	int m_State;
	time_t m_StateStartTime;

	
	int m_SkillState; // be used in OnSkill(...) method.
	int m_SkillUsed; // ID of skill which is being used.

	time_t m_TimeInvincibleEnd;
	time_t m_TimeSkillAvailable;
	time_t m_TimeSkillEnded;

	unique_ptr<class AnimatedObjectClass> m_JumpAnimationData;
	unique_ptr<class AnimatedObjectClass> m_FallAnimationData;
	unique_ptr<class AnimatedObjectClass> m_WalkAnimationData;
	unique_ptr<class AnimatedObjectClass> m_RunAnimationData;
	unique_ptr<class AnimatedObjectClass> m_SkillAnimationData;
};

template<int index>
inline int CharacterClass::GetSkill()
{
	static_assert(0 <= index && index <= 3);
	return m_Skill[index];
}


