#pragma once
#include <DirectXMath.h>

#include <memory>
#include <vector>
#include <utility>

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
	struct SkillType
	{
		int skill_type;
		int skill_power;
	};

public:
	CharacterClass(int pos_x, int pos_y);

	bool Frame(time_t time_delta, time_t curr_time, class InputClass* input,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs,
		const vector<class GroundClass>& ground,
		class SoundClass* sound);
	
	void GetShapeMatrices(time_t curr_time, vector<XMMATRIX>& shape_matrices);
	inline time_t GetTimeInvincibleEnd() { return time_invincible_end_; }


	// If character have been actually damanged because of this collision,
	// this function returns true. Otherwise, it returns false.
	bool OnCollided(time_t curr_time, int vx);

	float GetCooltimeGaugeRatio(time_t curr_time);
	float GetInvincibleGaugeRatio(time_t curr_time);

	void LearnSkill(int skill_id);
	inline SkillType GetSkill(const int index)
	{
		assert(0 <= index && index <= 3);
		return skill_[index];
	}

	inline unsigned long long GetTotalScore(time_t curr_time)
	{
		if (state_ == CharacterState::kDie)
			return score_ * 1'000 + state_start_time_;
		else return score_ * 1'000 + curr_time;
	}
	inline int GetCombo() { return combo_; }
	inline time_t GetComboDurableTime(time_t curr_time)
	{ return (time_combo_end_ > curr_time) ? (time_combo_end_ - curr_time) : 0; }

	inline void AddScore() { ++score_; }
	void AddCombo(time_t curr_time);

private:

	void OnSkill(time_t curr_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs);

	bool UseSkill(time_t curr_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs,
		class SoundClass* sound);

private:
	int jump_cnt;

	int skill_state_; // be used in OnSkill(...) method.

	int score_, combo_;

	// The list of skill which the character has.
	// And the skill which is spellled.
	struct SkillType skill_[4], skill_currently_used_;

	time_t time_combo_end_;
	time_t time_invincible_end_;
	time_t time_skill_available_;
	time_t time_skill_ended_;

	unique_ptr<class AnimatedObjectClass> jump_animation_data_;
	unique_ptr<class AnimatedObjectClass> fall_animation_data_;
	unique_ptr<class AnimatedObjectClass> walk_animation_data_;
	unique_ptr<class AnimatedObjectClass> run_animation_data_;
	unique_ptr<class AnimatedObjectClass> skill_animation_data_;
};


