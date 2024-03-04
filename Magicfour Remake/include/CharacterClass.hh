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
		int		skill_type;
		int		skill_power;
		time_t	learned_time;
	};

	enum class SkillBonus : unsigned int
	{
		BONUS_STRAIGHT_FLUSH,
		BONUS_FOUR_CARDS,
		BONUS_FLUSH,
		BONUS_STRAIGHT,
		BONUS_TRIPLE,
		BONUS_TWO_PAIR,
		BONUS_ONE_PAIR,
		BONUS_NO_PAIR,
		BONUS_NONE
	};

public:
	CharacterClass(int pos_x, int pos_y);
	~CharacterClass() = default;

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

	SkillBonus LearnSkill(int skill_id, time_t curr_time);
	inline const SkillType& GetSkill(const int index)
	{
		assert(0 <= index && index <= 3);
		return skill_[index];
	}

	inline unsigned int GetScore()
	{
		return score_;

	}
	inline unsigned int GetCombo()
	{
		return combo_;
	}
	
	inline time_t GetComboDurableTime(time_t curr_time)
	{
		return (time_combo_end_ > curr_time) ? (time_combo_end_ - curr_time) : 0;
	}

	void AddCombo(time_t curr_time);

	inline SkillBonus GetSkillBonus()
	{
		return skill_bonus_;
	}
	inline time_t GetSkillBonusElapsedTime(time_t curr_time)
	{
		return curr_time - time_skill_bonus_get_;
	}

	inline class SkillObjectGuardian* GetGuardian(int index)
	{
		return guardians_[index].get();
	}

private:
	void OnSkill(time_t curr_time, time_t delta_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs);

	bool UseSkill(time_t curr_time,
		vector<unique_ptr<class SkillObjectClass> >& skill_objs,
		class SoundClass* sound);

	SkillBonus CalculateSkillBonus();

private:
	int jump_cnt;
	unsigned int score_, combo_;

	// The list of skill which the character has.
	// And the skill which is spellled.
	struct SkillType skill_[4], skill_currently_used_;
	SkillBonus skill_bonus_;

	time_t time_skill_bonus_get_;

	time_t time_combo_end_;
	time_t time_invincible_end_;
	time_t time_skill_available_;
	time_t time_skill_ended_;


	unique_ptr<class SkillObjectGuardian> guardians_[2];

	unique_ptr<class AnimatedObjectClass> jump_animation_data_;
	unique_ptr<class AnimatedObjectClass> fall_animation_data_;
	unique_ptr<class AnimatedObjectClass> walk_animation_data_;
	unique_ptr<class AnimatedObjectClass> run_animation_data_;
	unique_ptr<class AnimatedObjectClass> skill_animation_data_;
};


