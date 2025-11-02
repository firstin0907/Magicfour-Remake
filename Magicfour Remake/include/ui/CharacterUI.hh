#pragma once
#include "ui/interface/IGameObjectUI.hh"

#include "util/ResourceMap.hh"

class CharacterUI
{
public:
	static void DrawUI(class D2DClass* direct2D, class UserInterfaceClass* ui,
		class IGameObject* obj, time_t curr_time);

private:
	static void DrawScoreAndCombo(class D2DClass* direct2D,
		const struct UIContext& context,
		class CharacterClass* character, time_t curr_time);
	static void DrawSkillGauge(class D2DClass* direct2D,
		const struct UIContext& context,
		float char_screen_x, float char_screen_y,
		float skill_charge_ratio);
	static void DrawInvincibleGauge(class D2DClass* direct2D,
		const struct UIContext& context,
		float char_screen_x, float char_screen_y,
		float invincible_ratio);
	static void DrawSkillBonus(class D2DClass* direct2D,
		const struct UIContext& context, unsigned int skill_bonus,
		time_t learn_elapsed_time);
	static void DrawSkillPower(class D2DClass* direct2D,
		int skill_type, int skill_power, time_t skill_learned_elapsed_time,
		float skill_stone_screen_x, float skill_stone_screen_y);
};