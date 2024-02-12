#pragma once

#include <windows.h>

#include <memory>
#include <vector>

#include <DirectXMath.h>

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 1.0f;

using namespace std;
using namespace DirectX;

class ApplicationClass
{
public:
	ApplicationClass(int screenWidth, int screenHeight, HWND hwnd);
	ApplicationClass(const ApplicationClass&) = delete;
	~ApplicationClass();

	bool Frame(class InputClass* input);

private:
	void Render(time_t curr_time);

private:
	unique_ptr<class D3DClass>			m_Direct3D;
	unique_ptr<class D2DClass>			m_Direct2D;
	unique_ptr<class CameraClass>		m_Camera;

	unique_ptr<class ModelClass>		m_Model;
	unique_ptr<class ModelClass>		m_PlaneModel;
	unique_ptr<class ModelClass>		m_DiamondModel;
	unique_ptr<class ModelClass>		m_GemModel;
	unique_ptr<class TextureClass>		m_RainbowTexture;
	unique_ptr<class TextureClass>		m_BackgroundTexture;

	unique_ptr<class LightClass>		m_Light;

	unique_ptr<class LightShaderClass>	m_LightShader;
	unique_ptr<class StoneShaderClass>	m_StoneShader;
	unique_ptr<class TextureShaderClass>	m_TextureShader;
	unique_ptr<class NormalMapShaderClass>	m_NormalMapShader;

	unique_ptr<class CharacterClass>	m_Character;

	vector<unique_ptr<class SkillObjectClass> > m_SkillObjectList;
	vector<unique_ptr<class MonsterClass> > m_Monsters;
	vector<unique_ptr<class ItemClass> > m_Items;
	vector<unique_ptr<class GroundClass> > m_Ground;

	unique_ptr<class TimerClass> m_TimerClass;
	unique_ptr<class MonsterSpawnerClass> m_MonsterSpawner;

	unique_ptr<class UserInterfaceClass> m_UserInterface;
};
