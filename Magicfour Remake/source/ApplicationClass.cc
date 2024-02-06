#include "../include/ApplicationClass.hh"

#include <algorithm>

#include "../include/D3DClass.hh"
#include "../include/D2DClass.hh"
#include "../include/ModelClass.hh"
#include "../include/InputClass.hh"
#include "../include/LightShaderClass.hh"
#include "../include/NormalMapShaderClass.hh"
#include "../include/LightClass.hh"
#include "../include/StoneShaderClass.hh"
#include "../include/CharacterClass.hh"
#include "../include/MonsterClass.hh"
#include "../include/Monsters.hh"
#include "../include/AnimatedObjectClass.hh"
#include "../include/CameraClass.hh"
#include "../include/SkillObjectClass.hh"
#include "../include/SkillObjects.hh"
#include "../include/GroundClass.hh"
#include "../include/MonsterSpawnerClass.hh"
#include "../include/TimerClass.hh"
#include "../include/TextureShaderClass.hh"
#include "../include/TextureClass.hh"
#include "../include/SkillGaugeClass.hh"
#include "../include/UserInterfaceClass.hh"
#include "../include/ItemClass.hh"
#include "../include/RandomClass.hh"

constexpr float CAMERA_Z_POSITION = -20.0f;
constexpr int CAMERA_X_LIMIT = 1'500'000;

constexpr int ITEM_DROP_PROBABILITY = 50;

ApplicationClass::ApplicationClass(int screenWidth, int screenHeight, HWND hwnd)
{
	m_Direct3D = make_unique<D3DClass>(screenWidth, screenHeight,
		VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	m_Direct2D = make_unique<D2DClass>(m_Direct3D->GetSwapChain(), hwnd);

	m_Camera = make_unique<CameraClass>();
	m_Camera->SetPosition(0.0f, 0.0f, CAMERA_Z_POSITION);

	m_Model = make_unique<ModelClass>(m_Direct3D->GetDevice(),
		"data/model/abox.obj", L"data/texture/stone01.tga", L"data/texture/normal01.tga");
	m_PlaneModel = make_unique<ModelClass>(m_Direct3D->GetDevice(),
		"data/model/PlaneObject.obj", L"data/texture/stone01.tga", L"data/texture/normal01.tga");
	m_DiamondModel = make_unique<ModelClass>(m_Direct3D->GetDevice(),
		"data/model/diamond.obj", L"data/texture/stone01.tga");

	m_RainbowTexture = make_unique<TextureClass>(m_Direct3D->GetDevice(),
		L"data/texture/skill_gauge_rainbow.png");
	m_BackgroundTexture = make_unique<TextureClass>(m_Direct3D->GetDevice(),
		L"data/texture/background.jpg");

	// Create and initialize the light shader object.
	m_LightShader = make_unique<LightShaderClass>(m_Direct3D->GetDevice(), hwnd);

	// Create and initialize the light object.
	m_Light = make_unique<LightClass>();
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	m_StoneShader = make_unique<StoneShaderClass>(m_Direct3D->GetDevice(), hwnd);
	m_TextureShader = make_unique<TextureShaderClass>(m_Direct3D->GetDevice(), hwnd);
	m_NormalMapShader = make_unique<NormalMapShaderClass>(m_Direct3D->GetDevice(), hwnd);

	// Set model of skill object to be rendered.
	SkillObjectBead::initialize(new ModelClass(m_Direct3D->GetDevice(),
		"data/model/orb_low.obj", L"data/model/orb_low_fragment_BaseColor.tga"
	));
	SkillObjectSpear::initialize(new ModelClass(m_Direct3D->GetDevice(),
		"data/model/MagicCeramicBlade.obj", L"data/model/MagicCeramicBlade_MagicCeramicKnife_Emissive.tga",
		L"data/model/MagicCeramicBlade_MagicCeramicKnife_Normal.tga"
	));
	SkillObjectLeg::initialize(new ModelClass(m_Direct3D->GetDevice(),
		"data/model/leg.obj", L"data/model/MagicCeramicBlade_MagicCeramicKnife_BaseColor.tga",
		L"data/model/MagicCeramicBlade_MagicCeramicKnife_Normal.tga"
	));
	SkillObjectBasic::initialize(new ModelClass(m_Direct3D->GetDevice(),
		"data/model/MagicCeramicBlade.obj", L"data/model/MagicCeramicBlade_MagicCeramicKnife_BaseColor.tga",
		L"data/model/MagicCeramicBlade_MagicCeramicKnife_Normal.tga"
	));

	// Create character instance.
	m_Character = make_unique<CharacterClass>(0, 0);
	
	// Temporary
	m_Monsters.emplace_back(new MonsterDuck(LEFT_FORWARD, 1000));
	m_Monsters.emplace_back(new MonsterOctopus(RIGHT_FORWARD, 1000));
	for(int i = 1; i <= 10; i++) m_Monsters.emplace_back(new MonsterBird(RIGHT_FORWARD, 1000));

	// Set ground of field.
	m_Ground.emplace_back(new GroundClass({ -100000, 400000, 300000, 460000 }));
	m_Ground.emplace_back(new GroundClass({ -1800000, 100000, -200000, 160000 }));
	m_Ground.emplace_back(new GroundClass({ -300000, -100000, 1300000, -40000 }));
	m_Ground.emplace_back(new GroundClass({ -20000, -20000, 20000, 20000 }));
	m_Ground.emplace_back(new GroundClass({ SPAWN_LEFT_X, GROUND_Y - 300000, SPAWN_RIGHT_X, GROUND_Y }));

	m_MonsterSpawner = make_unique<MonsterSpawnerClass>();

	m_TimerClass = make_unique<TimerClass>();

	m_UserInterface = make_unique<UserInterfaceClass>(m_Direct2D.get(),
		m_Direct3D->GetDevice(), screenWidth, screenHeight,
		L"data/texture/user_interface/monster_hp_frame.png", L"data/texture/user_interface/monster_hp_gauge.png");

}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Frame(InputClass* input)
{
	static time_t character_death_time = 1LL << 60;

	// Check if the user pressed escape and wants to exit the application.
	if (input->IsKeyPressed(DIK_ESCAPE)) return false;

	m_TimerClass->Frame();
	time_t curr_time = m_TimerClass->GetTime();
	time_t delta_time = m_TimerClass->GetElapsedTime();

	const int GAME_OVER_SLOW = 4;
	if (character_death_time <= curr_time)
	{
		delta_time = curr_time / GAME_OVER_SLOW - (curr_time - delta_time) / GAME_OVER_SLOW;
		curr_time = character_death_time + (curr_time - character_death_time) / GAME_OVER_SLOW;
	}
	else m_MonsterSpawner->Frame(curr_time, delta_time, m_Monsters);

	m_Character->Frame(delta_time, curr_time, input, m_SkillObjectList, m_Ground);

	const float camera_x = SATURATE(-CAMERA_X_LIMIT, m_Character->GetPosX(), CAMERA_X_LIMIT) * SCOPE;
	const float camera_y = max(0, m_Character->GetPosY()) * SCOPE;
	m_Camera->SetPosition(camera_x, camera_y, CAMERA_Z_POSITION);


	// Move skill object instances.
	for (auto& skill_obj : m_SkillObjectList)
		skill_obj->FrameMove(curr_time, delta_time, m_Ground);

	// Move monsters.
	for (auto& monster : m_Monsters)
		monster->FrameMove(curr_time, delta_time, m_Ground);

	// Move items.
	for (auto& item : m_Items)
		item->FrameMove(curr_time, delta_time, m_Ground);


	// Coliide check
	for (auto& skill_obj : m_SkillObjectList)
	{
		for (auto& monster : m_Monsters)
		{
			if (monster->GetState() == MONSTER_STATE_DIE) continue;

			if (skill_obj->GetGlobalRange().collide(monster->GetGlobalRange()))
			{
				if (skill_obj->OnCollided(monster.get(), curr_time))
				{
					// If monster was sucessfully hit, add combo
					m_Character->AddCombo(curr_time);
				}
			}
		}
	}


	for (auto& monster : m_Monsters)
	{
		if (monster->GetState() == MONSTER_STATE_DIE) continue;

		if (m_Character->GetGlobalRange().collide(monster->GetGlobalRange()))
		{
			bool result = m_Character->OnCollided(curr_time, monster->GetVx());
			if (!result) character_death_time = curr_time;
		}
	}

	for (auto& item : m_Items)
	{
		if (item->GetState() == ItemClass::STATE_DIE) continue;

		if (m_Character->GetGlobalRange().collide(item->GetGlobalRange()))
		{
			m_Character->LearnSkill(item->GetType());
			item->SetState(ItemClass::STATE_DIE);
		}
	}

	// Process some work which should be conducted per frame,
	// for skill object instances
	for (int i = 0; i < m_SkillObjectList.size(); i++)
	{
		// If this skill object should be deleted,
		if (!m_SkillObjectList[i]->Frame(curr_time, delta_time))
		{
			// swap with last element and pop it.
			swap(m_SkillObjectList[i], m_SkillObjectList.back());
			m_SkillObjectList.pop_back();
		}
	}


	// Process some work which should be conducted per frame,
	// for monster object instances
	for (int i = 0; i < m_Monsters.size(); i++)
	{
		// If this monster instance should be deleted,
		if (!m_Monsters[i]->Frame(curr_time, delta_time))
		{
			// create item for 50% probablity.
			if (RandomClass::rand(0, 100) < ITEM_DROP_PROBABILITY)
			{
				m_Items.emplace_back(new ItemClass(curr_time, m_Monsters[i]->GetPosX(),
					m_Monsters[i]->GetPosY(), m_Monsters[i]->GetType()));
			}

			// and swap with last element and pop it.
			swap(m_Monsters[i], m_Monsters.back());
			m_Character->AddScore();
			m_Monsters.pop_back();
		}
	}

	// Process some work which should be conducted per frame,
	// for items.
	for (int i = 0; i < m_Items.size(); i++)
	{
		// If this monster instance should be deleted,
		if (!m_Items[i]->Frame(curr_time, delta_time))
		{
			// swap with last element and pop it.
			swap(m_Items[i], m_Items.back());
			m_Items.pop_back();
		}
	}

	// Render the graphics scene.
	Render(curr_time);
	return true;
}

void ApplicationClass::Render(time_t curr_time)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.5f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	XMMATRIX vpMatrix = viewMatrix * projectionMatrix;

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_PlaneModel->Render(m_Direct3D->GetDeviceContext());
	m_LightShader->Render(m_Direct3D->GetDeviceContext(), 
		m_Model->GetIndexCount(),
		XMMatrixScaling(192.0f, 153.6f, 1)
		* XMMatrixTranslation(0, 0, 100.0f),
		vpMatrix,
		m_BackgroundTexture->GetTexture(),
		m_Light->GetDirection(), m_Light->GetDiffuseColor());
	m_Model->Render(m_Direct3D->GetDeviceContext());

#if 0
	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(),
		m_Character->GetRangeRepresentMatrix(), vpMatrix, m_Model->GetDiffuseTexture(),
		m_Light->GetDirection(), m_Light->GetDiffuseColor());
	if (!result) return false;
#endif
	
	vector<XMMATRIX> char_model_matrices;
	m_Character->GetShapeMatrices(curr_time, char_model_matrices);


	ID3D11ShaderResourceView* char_texture = m_Model->GetDiffuseTexture();
	if (curr_time <= m_Character->GetTimeInvincibleEnd()) char_texture = m_RainbowTexture->GetTexture();
	
	for(auto &box : char_model_matrices) {
		m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(),
			box * m_Character->GetLocalWorldMatrix(), vpMatrix, char_texture,
			m_Light->GetDirection(), m_Light->GetDiffuseColor());
	}


	constexpr float box_size = 0.30f;
	XMMATRIX pos = 
		XMMatrixRotationX(XM_PI / 18) * XMMatrixRotationY(curr_time * 0.001f) * XMMatrixRotationX(-XM_PI / 10) *
		XMMatrixScaling(box_size, box_size * 1.2f, box_size) * XMMatrixTranslation(-1.3f, 4.0f, 0.f) *
		m_Character->GetLocalWorldMatrix();


	constexpr XMFLOAT4 skill_color[5] =
	{
		{0, 0, 0, 1.0f},
		{0.9f, 0.1f, 0.3f, 1.0f},
		{0.2f, 0.8f, 0.1f, 1.0f},
		{0.1f, 0.3f, 0.9f, 1.0f},
		{0.2f, 0.1f, 0.1f, 1.0f}
	};

	m_DiamondModel->Render(m_Direct3D->GetDeviceContext());
	
	if (m_Character->GetSkill<0>())
	{
		m_StoneShader->Render(m_Direct3D->GetDeviceContext(), m_DiamondModel->GetIndexCount(),
			pos, vpMatrix, m_Light->GetDirection(), skill_color[m_Character->GetSkill<0>()],
			m_Camera->GetPosition());
	}
	if (m_Character->GetSkill<1>())
	{
		pos *= XMMatrixTranslation(0, -0.6f, 0);
		m_StoneShader->Render(m_Direct3D->GetDeviceContext(), m_DiamondModel->GetIndexCount(),
			pos, vpMatrix, m_Light->GetDirection(), skill_color[m_Character->GetSkill<1>()],
			m_Camera->GetPosition());
	}
	if (m_Character->GetSkill<2>())
	{
		pos *= XMMatrixTranslation(0, -0.6f, 0);
		m_StoneShader->Render(m_Direct3D->GetDeviceContext(), m_DiamondModel->GetIndexCount(),
			pos, vpMatrix, m_Light->GetDirection(), skill_color[m_Character->GetSkill<2>()],
			m_Camera->GetPosition());
	}
	if (m_Character->GetSkill<3>())
	{
		pos *= XMMatrixTranslation(0, -0.6f, 0);
		m_StoneShader->Render(m_Direct3D->GetDeviceContext(), m_DiamondModel->GetIndexCount(),
			pos, vpMatrix, m_Light->GetDirection(), skill_color[m_Character->GetSkill<3>()],
			m_Camera->GetPosition());
	}


	// Draw Items
	for (auto& item : m_Items)
	{
		m_StoneShader->Render(m_Direct3D->GetDeviceContext(), m_DiamondModel->GetIndexCount(),
			item->GetShapeMatrix(curr_time) * item->GetLocalWorldMatrix(), vpMatrix,
			m_Light->GetDirection(), skill_color[item->GetType()], m_Camera->GetPosition());
	}


	// Draw skill object
	for (auto& skill_obj : m_SkillObjectList)
	{
		/*
		m_Model->Render(m_Direct3D->GetDeviceContext());
		result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(),
			skill_obj->GetRangeRepresentMatrix(), vpMatrix, m_Model->GetDiffuseTexture(),
			m_Light->GetDirection(), m_Light->GetDiffuseColor());*/
		
		auto obj_model = skill_obj->GetModel();
		obj_model->Render(m_Direct3D->GetDeviceContext());

		if (obj_model->GetNormalTexture())
		{
			m_NormalMapShader->Render(m_Direct3D->GetDeviceContext(), obj_model->GetIndexCount(),
				skill_obj->GetGlobalShapeTransform(curr_time), vpMatrix, obj_model->GetDiffuseTexture(),
				obj_model->GetNormalTexture(), m_Light->GetDirection(), m_Light->GetDiffuseColor());
		}
		else
		{
			m_LightShader->Render(m_Direct3D->GetDeviceContext(), obj_model->GetIndexCount(),
				skill_obj->GetGlobalShapeTransform(curr_time), vpMatrix, obj_model->GetDiffuseTexture(),
				m_Light->GetDirection(), m_Light->GetDiffuseColor());
		}
		
	}

	m_Model->Render(m_Direct3D->GetDeviceContext());
	for (auto& monster : m_Monsters)
	{
		m_NormalMapShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(),
			monster->GetRangeRepresentMatrix(), vpMatrix, m_Model->GetDiffuseTexture(),
			m_Model->GetNormalTexture(), m_Light->GetDirection(), m_Light->GetDiffuseColor());
	}

	for (auto& ground : m_Ground)
	{
		m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(),
			ground->GetRange().toMatrix(), vpMatrix, m_Model->GetDiffuseTexture(),
			m_Light->GetDirection(), m_Light->GetDiffuseColor());
	}


	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	m_UserInterface->Render(m_Direct2D.get(), m_TextureShader.get(), m_Direct3D->GetDeviceContext(),
		m_Character.get(), m_Monsters, vpMatrix, orthoMatrix, curr_time);

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_Direct3D->TurnZBufferOn();


	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();
}