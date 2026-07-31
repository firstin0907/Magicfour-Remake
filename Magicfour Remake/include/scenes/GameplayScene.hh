#pragma once
#include "scenes/interface/IGameScene.hh"

#include <memory>
#include <string>

#include "core/GameObjectList.hh"

class GameplayScene : public IGameScene
{
private:
    enum class SceneState
    {
        kGameBegin, kGameRun, kGamePause, kGameOver
    };

public:
    /// @brief Default constructor for the IGameScene class.
    GameplayScene(float screen_width, float screen_height, class InputClass* input);

    /// @brief Virtual destructor for the IGameScene class.
    virtual ~GameplayScene();

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

    time_t game_time_; // Actually played time (exclude for paused time)
    time_t game_time_delta_;

    GameState game_state_;
    time_t state_start_time_;

	std::unique_ptr<class CameraClass>		camera_;
	std::unique_ptr<class LightClass>		light_;
	std::unique_ptr<class FieldClass>		field_;

	std::unique_ptr<class CharacterClass>	character_;
	GameObjectList	skill_object_list_;
	GameObjectList	monsters_;
	GameObjectList	items_;

	
	std::unique_ptr<class MonsterSpawnerClass>	monster_spawner_;
    std::string next_scene_name_;
};