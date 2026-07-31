#include "scenes/TitleScene.hh"

#include "core/InputClass.hh"

#include "shader/ShaderManager.hh"

#include "core/common/GraphicResources.hh"

TitleScene::TitleScene(float screen_width, float screen_height, InputClass* input) :
    screen_width_(screen_width), screen_height_(screen_height), input_(input), next_scene_name_()
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Frame(time_t scene_time, time_t time_delta, class SoundClass* sound_manager)
{
	next_scene_name_ = "GameplayScene"; // Transition to InitScene after rendering the title scene.
}

void TitleScene::Render(time_t scene_time, time_t time_delta, ShaderManager* shader_manager, GraphicResources* graphic_resources)
{

}

void TitleScene::RenderUI(time_t scene_time, time_t time_delta, UserInterfaceClass* user_interface)
{

}

void TitleScene::OnEnter()
{
    // Initialize the title scene when it is entered.
    next_scene_name_.clear();
}

void TitleScene::OnExit()
{
    // Clean up the title scene when it is exited.
    next_scene_name_.clear();
}

const std::string& TitleScene::NextScene() const
{
    return next_scene_name_;
}
