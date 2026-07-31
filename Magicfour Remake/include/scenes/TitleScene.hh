#pragma once
#include "scenes/interface/IGameScene.hh"

class TitleScene : public IGameScene
{
public:
    /// @brief Default constructor for the IGameScene class.
    TitleScene(float screen_width, float screen_height, class InputClass* input);

    /// @brief Virtual destructor for the IGameScene class.
    virtual ~TitleScene();

    /// @brief Called every frame to update the scene's state.
    virtual void Frame(time_t scene_time, time_t time_delta, class SoundClass* sound_manager);

    /// @brief Called every frame to render the scene.
    virtual void Render(time_t scene_time, time_t time_delta, class ShaderManager* shader_manager, class GraphicResources* graphic_resources);
    
    /// @brief Called very frame to render the scene's UI.
    virtual void RenderUI(time_t scene_time, time_t time_delta, class UserInterfaceClass* user_interface);
    
    /// @brief Called when the scene is entered.
    virtual void OnEnter();

    /// @brief Called when the scene is exited.
    virtual void OnExit();

    /// @brief Returns the name of the next scene to transition to.
    /// @return The name of the next scene as a string. Empty string indicates no transition. 
    virtual const std::string& NextScene() const;
    
private:
    float screen_width_, screen_height_;
    class InputClass* input_;
    
    std::string next_scene_name_;
};