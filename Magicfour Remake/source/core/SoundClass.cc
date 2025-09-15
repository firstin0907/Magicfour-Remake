#include "core/SoundClass.hh"

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

	sounds_.loadFromXML("data/resources.xml", "Sound",
		[this](xml_node_wrapper node) -> unique_ptr<SoundEffect>
		{
			std::string src = node.get_required_attr("src");
			return std::make_unique<SoundEffect>(this->aud_engine_.get(),
				std::wstring(src.begin(), src.end()).c_str());
		});
}

void SoundClass::PlayBackground(const std::string& background_music)
{
	background_loop_ = sounds_.get(background_music)->CreateInstance();
	background_loop_->Play();
}

void SoundClass::PlayEffect(const std::string& sound_name)
{
	sounds_.get(sound_name)->Play();
}


SoundClass::~SoundClass()
{
	if (aud_engine_)
	{
		aud_engine_->Suspend();
	}
}
