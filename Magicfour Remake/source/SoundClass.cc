#include "../include/SoundClass.hh"
#include "../third-party/Audio.h"

using namespace DirectX;
using namespace std;

#pragma comment(lib, "third-party/DirectXTK.lib")

SoundClass::SoundClass()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags |= AudioEngine_Debug;
#endif

	aud_engine_ = std::make_unique<AudioEngine>(eflags);

	backgrounds_.emplace_back(new SoundEffect(aud_engine_.get(),
		L"data/sound/background.wav"));

	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/character_damage.wav"));
	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/character_death.wav"));
	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/skill_learn.wav"));
	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/spell1.wav"));
	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/spell2.wav"));
	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/spell3.wav"));
	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/heartbeat.wav"));
	effects_.emplace_back(new SoundEffect(aud_engine_.get(), L"data/sound/gameover.wav"));
	
	background_loop_ = backgrounds_[0]->CreateInstance();
	background_loop_->Play(true);
}

void SoundClass::PlayBackground(BackgroundSound background_music)
{
	background_loop_ = backgrounds_[
		static_cast<unsigned int>(background_music)
	]->CreateInstance();

	background_loop_->Play();
}

void SoundClass::PlayEffect(EffectSound effect)
{
	effects_[static_cast<unsigned int>(effect)]->Play();
}


SoundClass::~SoundClass()
{
	if (aud_engine_)
	{
		aud_engine_->Suspend();
	}
}
