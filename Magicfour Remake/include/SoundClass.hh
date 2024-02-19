#pragma once

#include "../third-party/Audio.h"

#include <memory>

class SoundClass
{
public:
	SoundClass();
	void test()
	{
		explode_->Play();
	}
	void Update();
	~SoundClass();

private:
	std::unique_ptr<DirectX::AudioEngine> aud_engine_;

	std::unique_ptr<DirectX::SoundEffect> ambient_;
	std::unique_ptr<DirectX::SoundEffect> explode_;

	std::unique_ptr<DirectX::SoundEffectInstance> night_loop_;
};