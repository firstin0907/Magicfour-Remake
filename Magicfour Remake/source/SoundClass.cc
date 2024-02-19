#include "../include/SoundClass.hh"
#include "../third-party/Audio.h"

using namespace DirectX;

#pragma comment(lib, "third-party/DirectXTK.lib")

SoundClass::SoundClass()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags |= AudioEngine_Debug;
#endif

	aud_engine_ = std::make_unique<AudioEngine>(eflags);

	explode_ = std::make_unique<SoundEffect>(aud_engine_.get(),
		L"data/sound/media_Explo1.wav");
	ambient_ = std::make_unique<SoundEffect>(aud_engine_.get(),
		L"data/sound/background.wav");
	
	night_loop_ = ambient_->CreateInstance();
	night_loop_->Play(true);

	explode_->Play();
}


void SoundClass::Update()
{
}

SoundClass::~SoundClass()
{
	if (aud_engine_)
	{
		aud_engine_->Suspend();
	}
}
