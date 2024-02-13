#pragma once

#include <windows.h>

#include <memory>
#include <vector>

#include <DirectXMath.h>

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 1.0f;

class ApplicationClass
{
private:
	using XMMATRIX = DirectX::XMMATRIX;

	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

public:
	ApplicationClass(int screenWidth, int screenHeight, HWND hwnd);
	ApplicationClass(const ApplicationClass&) = delete;
	~ApplicationClass();

	bool Frame(class InputClass* input);

private:
	void Render(time_t curr_time);

private:
	unique_ptr<class D3DClass>			direct3D_;
	unique_ptr<class D2DClass>			direct2D_;
	unique_ptr<class CameraClass>		camera_;

	unique_ptr<class ModelClass>		model_;
	unique_ptr<class ModelClass>		planeModel_;
	unique_ptr<class ModelClass>		diamondModel_;
	unique_ptr<class ModelClass>		gemModel_;
	unique_ptr<class TextureClass>		rainbowTexture_;
	unique_ptr<class TextureClass>		backgroundTexture_;

	unique_ptr<class LightClass>		light_;

	unique_ptr<class LightShaderClass>	lightShader_;
	unique_ptr<class StoneShaderClass>	stoneShader_;
	unique_ptr<class TextureShaderClass>	textureShader_;
	unique_ptr<class NormalMapShaderClass>	normalMapShader_;

	unique_ptr<class CharacterClass>	character_;

	vector<unique_ptr<class SkillObjectClass> > skillObjectList_;
	vector<unique_ptr<class MonsterClass> > monsters_;
	vector<unique_ptr<class ItemClass> > items_;
	vector<unique_ptr<class GroundClass> > ground_;

	unique_ptr<class TimerClass> timerClass_;
	unique_ptr<class MonsterSpawnerClass> monsterSpawner_;

	unique_ptr<class UserInterfaceClass> userInterface_;
};
