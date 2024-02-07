#pragma once
#include <DirectXMath.h>

#include <memory>
#include <vector>

#include "global.hh"
#include "RigidbodyClass.hh"

enum class CharacterState
{
	kNormal, kJump, kWalk, kStop, kRun,
	kRunJump, kSpell, kHit, kSlip, kDie
};


class CharacterClass : public RigidbodyClass<CharacterState>
{
private:
	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public:
	CharacterClass(int pos_x, int pos_y);

	bool Frame(time_t time_delta, time_t curr_time, class InputClass* input,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs,
		const vector<unique_ptr<class GroundClass> >& ground
		);
	
	void GetShapeMatrices(time_t curr_time, vector<XMMATRIX>& shape_matrices);
	inline time_t GetTimeInvincibleEnd() { return m_TimeInvincibleEnd; }

	template <int index>
	int GetSkill();

	// If character have been killed because of this collision,
	// this function returns false. Otherwise, returns true.
	bool OnCollided(time_t curr_time, int vx);

	float GetCooltimeGaugeRatio(time_t curr_time);

	void LearnSkill(int skill_id);

	inline unsigned long long GetTotalScore(time_t curr_time)
	{
		if (state_ == CharacterState::kDie)
			return m_Score * 1'000 + state_start_time_;
		else return m_Score * 1'000 + curr_time;
	}
	inline int GetCombo() { return m_Combo; }
	inline time_t GetComboDurableTime(time_t curr_time)
	{ return (m_TimeComboEnd > curr_time) ? (m_TimeComboEnd - curr_time) : 0; }

	inline void AddScore() { ++m_Score; }
	void AddCombo(time_t curr_time);

private:

	void OnSkill(time_t curr_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs);

	bool UseSkill(time_t curr_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs);

private:
	int m_HitVx;

	int jump_cnt;

	int m_Skill[4] = { 0 };
	
	int m_SkillState; // be used in OnSkill(...) method.
	int m_SkillUsed; // ID of skill which is being used.

	int m_Score, m_Combo;

	time_t m_TimeComboEnd;
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


