#pragma once

#include <windows.h>

#include <memory>
#include <vector>

#include <DirectXMath.h>

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 1.0f;

enum class GameState
{
	kGameRun, kGamePause, kGameOver
};

class ApplicationClass
{
private:
	using XMMATRIX = DirectX::XMMATRIX;

	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

	template<typename T>
	using shared_ptr = std::shared_ptr<T>;

public:
	ApplicationClass(int screenWidth, int screenHeight, HWND hwnd);
	ApplicationClass(const ApplicationClass&) = delete;
	~ApplicationClass();

	bool Frame(class InputClass* input);

private:
	void GameFrame(class InputClass* input);
	void Render();

private:
	GameState game_state_;
	time_t	state_start_time_;

	unique_ptr<class D3DClass>			direct3D_;
	unique_ptr<class D2DClass>			direct2D_;
	unique_ptr<class SoundClass>		sound_;

	unique_ptr<class CameraClass>		camera_;

	unique_ptr<class ModelClass>		model_;
	unique_ptr<class ModelClass>		planeModel_;
	unique_ptr<class ModelClass>		diamondModel_;
	unique_ptr<class ModelClass>		gemModel_;
	unique_ptr<class TextureClass>		rainbowTexture_;
	unique_ptr<class TextureClass>		backgroundTexture_;

	unique_ptr<class LightClass>		light_;

	unique_ptr<class LightShaderClass>	light_shader_;
	unique_ptr<class StoneShaderClass>	stone_shader_;
	unique_ptr<class TextureShaderClass>	texture_shader_;
	unique_ptr<class NormalMapShaderClass>	normalMap_shader_;

	unique_ptr<class CharacterClass>	character_;

	vector<unique_ptr<class SkillObjectClass> > skillObjectList_;
	vector<unique_ptr<class MonsterClass> >	monsters_;
	vector<unique_ptr<class ItemClass> >	items_;

	unique_ptr<class FieldClass>			field_;


	unique_ptr<class TimerClass>			timer_;
	unique_ptr<class MonsterSpawnerClass>	monster_spawner_;

	unique_ptr<class UserInterfaceClass>	user_interface_;
};
