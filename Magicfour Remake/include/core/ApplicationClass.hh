#pragma once

#include <windows.h>

#include <memory>
#include <vector>
#include <functional>

#include <DirectXMath.h>

#include "GameObjectList.hh"

#include "shader/ShaderManager.hh"
#include "util/ResourceMap.hh"
#include "core/global.hh"


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
	ApplicationClass(int screenWidth, int screenHeight, HWND hwnd, class InputClass* input);
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

	ResourceMap<class ModelClass>		models_;
	ResourceMap<class TextureClass>		textures_;

	unique_ptr<class LightClass>		light_;
	unique_ptr<class ShaderManager>		shader_manager_;

	unique_ptr<class CharacterClass>	character_;

	GameObjectList	skillObjectList_;
	GameObjectList	monsters_;
	GameObjectList	items_;

	unique_ptr<class FieldClass>			field_;


	unique_ptr<class TimerClass>			timer_;
	unique_ptr<class MonsterSpawnerClass>	monster_spawner_;

	unique_ptr<class UserInterfaceClass>	user_interface_;
};
