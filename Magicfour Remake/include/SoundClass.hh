#pragma once

#include "../third-party/Audio.h"

#include <vector>
#include <memory>

enum class BackgroundSound : unsigned int
{
	kSoundOnGameBackground
};

enum class EffectSound : unsigned int
{
	kSoundCharacterDamage,
	kSoundCharacterDie,
	kSoundSkillLearn,
	kSoundSpell1,
	kSoundSpell2,
	kSoundSpell3,
	kSoundHeartbeat,
	kSoundGameOver
};

class SoundClass
{

private:
	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public:
	SoundClass();
	~SoundClass();

	void PlayBackground(BackgroundSound background_music);
	void PlayEffect(EffectSound effect);

private:
	unique_ptr<DirectX::AudioEngine> aud_engine_;

	std::vector<unique_ptr<DirectX::SoundEffect> > backgrounds_;
	std::vector<unique_ptr<DirectX::SoundEffect> > effects_;

	unique_ptr<DirectX::SoundEffectInstance> background_loop_;
};