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
#define CHARACTER_STATE_INVINCIBLE		8


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
	
	// for debug
	inline XMMATRIX GetRangeRepresentMatrix()
	{
		return range.add(pos_x, pos_y).toMatrix();
	}

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

	int pos_x, pos_y, pos_yv;
	int jump_cnt;

	int m_Skill[4] = { 0 };

	int m_State;
	time_t m_StateStartTime;

	// be used in OnSkill(...) method.
	int m_SkillState;
	int m_SkillUsed;

	time_t m_TimeSkillAvailable;
	time_t m_TimeSkillEnded;

	unique_ptr<class AnimatedObjectClass> m_JumpAnimationData;
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


