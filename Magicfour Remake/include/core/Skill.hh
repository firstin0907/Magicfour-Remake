#pragma once

#include <time.h>
#include <algorithm>

struct SkillType
{
	int		skill_type;
	int		skill_power;
	time_t	learned_time;

	bool	is_part_of_skillbonus;
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

static SkillBonus CalculateSkillBonus(SkillType(&skills)[4])
{
	auto set_is_part_of_skillbonus = [&](int bits)
	{
		// apply each bit to corresponding struct's member.
		skills[0].is_part_of_skillbonus = (bits & 0b0001) ? true : false;
		skills[1].is_part_of_skillbonus = (bits & 0b0010) ? true : false;
		skills[2].is_part_of_skillbonus = (bits & 0b0100) ? true : false;
		skills[3].is_part_of_skillbonus = (bits & 0b1000) ? true : false;
	};

	bool is_flush = true, is_straight = true;
	// check is it flush
	for (int i = 1; i <= 3; i++)
	{
		if (skills[i].skill_type != skills[i - 1].skill_type)
		{
			is_flush = false;
		}
	}

	// check is it straight
	std::pair<int, int> pw[4];
	for (int i = 0; i <= 3; i++) pw[i] = { skills[i].skill_power, 1 << i };
	sort(pw, pw + 4);
	for (int i = 1; i <= 3; i++)
	{
		if (pw[i].first != pw[i - 1].first + 1)
		{
			is_straight = false;
		}
	}

	if (is_flush && is_straight)
	{
		set_is_part_of_skillbonus(0b1111);
		return SkillBonus::BONUS_STRAIGHT_FLUSH;
	}

	if (pw[1].first == pw[2].first && pw[0].first == pw[1].first && pw[2].first == pw[3].first)
	{
		set_is_part_of_skillbonus(0b1111);
		return SkillBonus::BONUS_FOUR_CARDS;
	}

	if (is_flush)
	{
		set_is_part_of_skillbonus(0b1111);
		return SkillBonus::BONUS_FLUSH;
	}

	if (is_straight)
	{
		set_is_part_of_skillbonus(0b1111);
		return SkillBonus::BONUS_STRAIGHT;
	}

	if (pw[0].first == pw[1].first && pw[1].first == pw[2].first)
	{
		set_is_part_of_skillbonus(pw[0].second | pw[1].second | pw[2].second);
		return SkillBonus::BONUS_TRIPLE;
	}

	if (pw[1].first == pw[2].first && pw[2].first == pw[3].first)
	{
		set_is_part_of_skillbonus(pw[1].second | pw[2].second | pw[3].second);
		return SkillBonus::BONUS_TRIPLE;
	}

	if (pw[0].first == pw[1].first && pw[2].first == pw[3].first)
	{
		set_is_part_of_skillbonus(0b1111);
		return SkillBonus::BONUS_TWO_PAIR;
	}

	if (pw[0].first == pw[1].first)
	{
		set_is_part_of_skillbonus(pw[0].second | pw[1].second);
		return SkillBonus::BONUS_ONE_PAIR;
	}

	if (pw[1].first == pw[2].first)
	{
		set_is_part_of_skillbonus(pw[1].second | pw[2].second);
		return SkillBonus::BONUS_ONE_PAIR;
	}

	if (pw[2].first == pw[3].first)
	{
		set_is_part_of_skillbonus(pw[2].second | pw[3].second);
		return SkillBonus::BONUS_ONE_PAIR;
	}


	return SkillBonus::BONUS_NO_PAIR;

}

static const wchar_t* SkillBonusText(SkillBonus skill_bonus)
{	
	switch (skill_bonus)
	{

	case SkillBonus::BONUS_STRAIGHT_FLUSH:
		return L"Straight Flush : Straight + Flush";
		
	case SkillBonus::BONUS_FOUR_CARDS:
		return L"Four Cards : Coincided Skill";
		
	case SkillBonus::BONUS_FLUSH:
		return L"Flush : More Improved Skill";
		
	case SkillBonus::BONUS_STRAIGHT:
		return L"Straight : Faster Skill Charge";
		
	case SkillBonus::BONUS_TRIPLE:
		return L"Triple : More Powerful Skill";
		
	case SkillBonus::BONUS_TWO_PAIR:
		return L"Two Pair : Two Guardian Beads";
		
	case SkillBonus::BONUS_ONE_PAIR:
		return L"One Pair : One Guardian Bead";
		
	case SkillBonus::BONUS_NO_PAIR:
		return L"No Pair : More Powerful Basic Attack";

	default:
		return L"";
		
	}
}