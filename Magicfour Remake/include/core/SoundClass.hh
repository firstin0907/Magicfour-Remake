#pragma once

#include "../third-party/Audio.h"
#include "util/ResourceMap.hh"

#include <vector>
#include <memory>

class SoundClass
{

private:
	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public:
	SoundClass();
	~SoundClass();

	void PlayBackground(const std::string& background_music);
	void PlayEffect(const std::string& effect);

private:
	unique_ptr<DirectX::AudioEngine> aud_engine_;

	ResourceMap<DirectX::SoundEffect> sounds_;

	unique_ptr<DirectX::SoundEffectInstance> background_loop_;
};