#pragma once

#include <windows.h>

#include <memory>
#include <vector>
#include <functional>

#include <DirectXMath.h>

#include "GameObjectList.hh"

#include "shader/ShaderManager.hh"
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
	ApplicationClass(class ConfigManager* config, HWND hwnd, class InputClass* input);
	ApplicationClass(const ApplicationClass&) = delete;
	~ApplicationClass();

	bool Frame(class InputClass* input);

private:
	void ProcessRenderQueue();

private:
	GameState game_state_;
	time_t	state_start_time_;

	class ConfigManager* config_;

	unique_ptr<class D3DClass>			direct3d_;
	unique_ptr<class D2DClass>			direct2d_;
	unique_ptr<class SoundClass>		sound_;

	unique_ptr<class GraphicResources>	graphic_resources_;

	unique_ptr<class ShaderManager>		shader_manager_;

	unique_ptr<class TimerClass>			timer_;
	unique_ptr<class UserInterfaceClass>	user_interface_;

	std::unordered_map<std::string, shared_ptr<class IGameScene>> scenes_;

	shared_ptr<class IGameScene> current_scene_;

};
