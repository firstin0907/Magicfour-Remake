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

using namespace std;

constexpr float CAMERA_Z_POSITION = -20.0f;
constexpr int CAMERA_X_LIMIT = 1'500'000;

constexpr int ITEM_DROP_PROBABILITY = 50;

ApplicationClass::ApplicationClass(int screenWidth, int screenHeight, HWND hwnd)
{
	direct3D_ = make_unique<D3DClass>(screenWidth, screenHeight,
		VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	direct2D_ = make_unique<D2DClass>(direct3D_->GetSwapChain(), hwnd);

	camera_ = make_unique<CameraClass>();
	camera_->SetPosition(0.0f, 0.0f, CAMERA_Z_POSITION);

	model_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/abox.obj", L"data/texture/stone01.tga", L"data/texture/normal01.tga");
	planeModel_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/PlaneObject.obj", L"data/texture/stone01.tga", L"data/texture/normal01.tga");
	diamondModel_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/diamond.obj", L"data/texture/stone01.tga");
	gemModel_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/Crystal/Crystals_low.obj",
		L"data/model/Crystal/None_BaseColor.png",
		L"data/model/Crystal/None_Normal.png",
		L"data/model/Crystal/None_Emissive.png");

	rainbowTexture_ = make_unique<TextureClass>(direct3D_->GetDevice(),
		L"data/texture/skill_gauge_rainbow.png");
	backgroundTexture_ = make_unique<TextureClass>(direct3D_->GetDevice(),
		L"data/texture/background.jpg");

	// Create and initialize the light shader object.
	light_shader_ = make_unique<LightShaderClass>(direct3D_->GetDevice(), hwnd);

	// Create and initialize the light object.
	light_ = make_unique<LightClass>();
	light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	light_->SetDirection(0.0f, 0.0f, 1.0f);

	stone_shader_ = make_unique<StoneShaderClass>(direct3D_->GetDevice(), hwnd);
	texture_shader_ = make_unique<TextureShaderClass>(direct3D_->GetDevice(), hwnd);
	normalMap_shader_ = make_unique<NormalMapShaderClass>(direct3D_->GetDevice(), hwnd);

	// Set model of skill object to be rendered.
	SkillObjectBead::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Orb/orb_low.obj",
		L"data/model/Orb/orb_low_fragment_BaseColor.png",
		L"data/model/Orb/orb_low_fragment_Normal.png",
		L"data/model/Orb/orb_low_fragment_Emissive.png"
	));
	SkillObjectSpear::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Blade/MagicCeramicBlade.obj",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_BaseColor.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Normal.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Emissive.jpg"
	));
	SkillObjectLeg::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Crystal/Crystals_low.obj",
		L"data/model/Crystal/None_BaseColor.png",
		L"data/model/Crystal/None_Normal.png"
	));
	SkillObjectBasic::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Blade/MagicCeramicBlade.obj",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_BaseColor.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Normal.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Emissive.jpg"
	));

	// Create character instance.
	character_ = make_unique<CharacterClass>(0, 0);
	
	// Temporary
	monsters_.emplace_back(new MonsterDuck(LEFT_FORWARD, 1000));
	monsters_.emplace_back(new MonsterOctopus(RIGHT_FORWARD, 1000));
	for(int i = 1; i <= 10; i++) monsters_.emplace_back(new MonsterBird(RIGHT_FORWARD, 1000));

	// Set ground of field.
	ground_.emplace_back(new GroundClass({ -100000, 400000, 300000, 460000 }));
	ground_.emplace_back(new GroundClass({ -1800000, 100000, -200000, 160000 }));
	ground_.emplace_back(new GroundClass({ -300000, -100000, 1300000, -40000 }));
	ground_.emplace_back(new GroundClass({ -20000, -20000, 20000, 20000 }));
	ground_.emplace_back(new GroundClass({ SPAWN_LEFT_X, GROUND_Y - 300000, SPAWN_RIGHT_X, GROUND_Y }));

	monster_spawner_ = make_unique<MonsterSpawnerClass>();

	timer_ = make_unique<TimerClass>();

	user_interface_ = make_unique<UserInterfaceClass>(direct2D_.get(),
		direct3D_->GetDevice(), screenWidth, screenHeight,
		L"data/texture/user_interface/monster_hp_frame.png", L"data/texture/user_interface/monster_hp_gauge.png");

}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Frame(InputClass* input)
{
	// Check if the user pressed escape and wants to exit the application.
	if (input->IsKeyPressed(DIK_ESCAPE)) return false;
	if (input->IsKeyDown(DIK_P))
	{
		timer_->Pause();
		game_state_ = GameState::kGamePause;
		Render(timer_->GetTime());
	}
	else if (input->IsKeyDown(DIK_R))
	{
		timer_->Resume();
		game_state_ = GameState::kGameRun;
	}

	timer_->Frame();

	const time_t curr_time = timer_->GetTime();
	switch (game_state_)
	{
	case GameState::kGameRun:
		GameFrame(input);
		Render(curr_time);
		return true;

	case GameState::kGamePause:
		return true;
	}
}


void ApplicationClass::GameFrame(InputClass* input)
{
	static time_t character_death_time = 1LL << 60;

	time_t curr_time = timer_->GetTime();
	time_t delta_time = timer_->GetElapsedTime();

	const int GAME_OVER_SLOW = 4;
	if (character_death_time <= curr_time)
	{
		delta_time = curr_time / GAME_OVER_SLOW - (curr_time - delta_time) / GAME_OVER_SLOW;
		curr_time = character_death_time + (curr_time - character_death_time) / GAME_OVER_SLOW;
	}
	else monster_spawner_->Frame(curr_time, delta_time, monsters_);

	character_->Frame(delta_time, curr_time, input, skillObjectList_, ground_);

	const float camera_x = SATURATE(-CAMERA_X_LIMIT, character_->GetPosition().x, CAMERA_X_LIMIT) * SCOPE;
	const float camera_y = max(0, character_->GetPosition().y) * SCOPE;
	camera_->SetPosition(camera_x, camera_y, CAMERA_Z_POSITION);


	// Move skill object instances.
	for (auto& skill_obj : skillObjectList_)
		skill_obj->FrameMove(curr_time, delta_time, ground_);

	// Move monsters.
	for (auto& monster : monsters_)
		monster->FrameMove(curr_time, delta_time, ground_);

	// Move items.
	for (auto& item : items_)
		item->FrameMove(curr_time, delta_time, ground_);


	// Coliide check
	for (auto& skill_obj : skillObjectList_)
	{
		for (auto& monster : monsters_)
		{
			if (monster->GetState() == MonsterState::kDie) continue;
			if (monster->GetState() == MonsterState::kEmbryo) continue;

			if (skill_obj->GetGlobalRange().collide(monster->GetGlobalRange()))
			{
				if (skill_obj->OnCollided(monster.get(), curr_time))
				{
					// If monster was sucessfully hit, add combo
					character_->AddCombo(curr_time);
				}
			}
		}
	}


	for (auto& monster : monsters_)
	{
		if (monster->GetState() == MonsterState::kDie) continue;

		if (character_->GetGlobalRange().collide(monster->GetGlobalRange()))
		{
			bool result = character_->OnCollided(curr_time, monster->GetVx());
			if (!result) character_death_time = curr_time;
		}
	}

	for (auto& item : items_)
	{
		if (item->GetState() == ItemState::kDie) continue;

		if (character_->GetGlobalRange().collide(item->GetGlobalRange()))
		{
			character_->LearnSkill(item->GetType());
			item->SetState(ItemState::kDie, curr_time);
		}
	}

	// Process some work which should be conducted per frame,
	// for skill object instances
	for (int i = 0; i < skillObjectList_.size(); i++)
	{
		// If this skill object should be deleted,
		if (!skillObjectList_[i]->Frame(curr_time, delta_time))
		{
			// swap with last element and pop it.
			swap(skillObjectList_[i], skillObjectList_.back());
			skillObjectList_.pop_back();
		}
	}


	// Process some work which should be conducted per frame,
	// for monster object instances
	for (int i = 0; i < monsters_.size(); i++)
	{
		// If this monster instance should be deleted,
		if (!monsters_[i]->Frame(curr_time, delta_time))
		{
			// create item for 50% probablity.
			if (RandomClass::rand(0, 100) < ITEM_DROP_PROBABILITY)
			{
				items_.emplace_back(new ItemClass(curr_time, monsters_[i]->GetPosition().x,
					monsters_[i]->GetPosition().y, monsters_[i]->GetType()));
			}

			// and swap with last element and pop it.
			swap(monsters_[i], monsters_.back());
			character_->AddScore();
			monsters_.pop_back();
		}
	}

	// Process some work which should be conducted per frame,
	// for items.
	for (int i = 0; i < items_.size(); i++)
	{
		// If this monster instance should be deleted,
		if (!items_[i]->Frame(curr_time, delta_time))
		{
			// swap with last element and pop it.
			swap(items_[i], items_.back());
			items_.pop_back();
		}
	}
}


void ApplicationClass::Render(time_t curr_time)
{
	XMMATRIX world_matrix, viewMatrix, projectionMatrix, orthoMatrix;

	// Clear the buffers to begin the scene.
	direct3D_->BeginScene(0.0f, 0.0f, 0.5f, 1.0f);
	
	// Generate the view matrix based on the camera's position.
	camera_->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	direct3D_->GetWorldMatrix(world_matrix);
	camera_->GetViewMatrix(viewMatrix);
	direct3D_->GetProjectionMatrix(projectionMatrix);
	direct3D_->GetOrthoMatrix(orthoMatrix);

	XMMATRIX vp_matrix = viewMatrix * projectionMatrix;

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	planeModel_->Render(direct3D_->GetDeviceContext());
	light_shader_->Render(direct3D_->GetDeviceContext(), 
		model_->GetIndexCount(),
		XMMatrixScaling(192.0f, 153.6f, 1)
		* XMMatrixTranslation(0, 0, 100.0f),
		vp_matrix,
		backgroundTexture_->GetTexture(),
		light_->GetDirection(), light_->GetDiffuseColor());
	model_->Render(direct3D_->GetDeviceContext());

#if 1
	light_shader_->Render(direct3D_->GetDeviceContext(), model_->GetIndexCount(),
		character_->GetRangeRepresentMatrix(), vp_matrix, model_->GetDiffuseTexture(),
		light_->GetDirection(), light_->GetDiffuseColor());
#endif
	vector<XMMATRIX> char_model_matrices;
	character_->GetShapeMatrices(curr_time, char_model_matrices);


	ID3D11ShaderResourceView* char_texture = model_->GetDiffuseTexture();
	if (curr_time <= character_->GetTimeInvincibleEnd()) char_texture = rainbowTexture_->GetTexture();
	
	for(auto &box : char_model_matrices) {
		light_shader_->Render(direct3D_->GetDeviceContext(), model_->GetIndexCount(),
			box * character_->GetLocalWorldMatrix(), vp_matrix, char_texture,
			light_->GetDirection(), light_->GetDiffuseColor());
	}


	constexpr float box_size = 0.30f;
	XMMATRIX pos = 
		XMMatrixRotationX(XM_PI / 18) * XMMatrixRotationY(curr_time * 0.001f) * XMMatrixRotationX(-XM_PI / 10) *
		XMMatrixScaling(box_size, box_size * 1.2f, box_size) * XMMatrixTranslation(-1.3f, 4.0f, 0.f) *
		character_->GetLocalWorldMatrix();


	constexpr XMFLOAT4 skill_color[5] =
	{
		{0, 0, 0, 1.0f},
		{0.9f, 0.1f, 0.3f, 1.0f},
		{0.2f, 0.8f, 0.1f, 1.0f},
		{0.1f, 0.3f, 0.9f, 1.0f},
		{0.2f, 0.1f, 0.1f, 1.0f}
	};

	diamondModel_->Render(direct3D_->GetDeviceContext());
	
	if (character_->GetSkill<0>())
	{
		stone_shader_->Render(direct3D_->GetDeviceContext(), diamondModel_->GetIndexCount(),
			pos, vp_matrix, light_->GetDirection(), skill_color[character_->GetSkill<0>()],
			camera_->GetPosition());
	}
	if (character_->GetSkill<1>())
	{
		pos *= XMMatrixTranslation(0, -0.6f, 0);
		stone_shader_->Render(direct3D_->GetDeviceContext(), diamondModel_->GetIndexCount(),
			pos, vp_matrix, light_->GetDirection(), skill_color[character_->GetSkill<1>()],
			camera_->GetPosition());
	}
	if (character_->GetSkill<2>())
	{
		pos *= XMMatrixTranslation(0, -0.6f, 0);
		stone_shader_->Render(direct3D_->GetDeviceContext(), diamondModel_->GetIndexCount(),
			pos, vp_matrix, light_->GetDirection(), skill_color[character_->GetSkill<2>()],
			camera_->GetPosition());
	}
	if (character_->GetSkill<3>())
	{
		pos *= XMMatrixTranslation(0, -0.6f, 0);
		stone_shader_->Render(direct3D_->GetDeviceContext(), diamondModel_->GetIndexCount(),
			pos, vp_matrix, light_->GetDirection(), skill_color[character_->GetSkill<3>()],
			camera_->GetPosition());
	}


	// Draw Items
	for (auto& item : items_)
	{
		stone_shader_->Render(direct3D_->GetDeviceContext(), diamondModel_->GetIndexCount(),
			item->GetShapeMatrix(curr_time) * item->GetLocalWorldMatrix(), vp_matrix,
			light_->GetDirection(), skill_color[item->GetType()], camera_->GetPosition());
	}


	// Draw skill object
	for (auto& skill_obj : skillObjectList_)
	{
		/*
		model_->Render(direct3D_->GetDeviceContext());
		result = light_shader_->Render(direct3D_->GetDeviceContext(), model_->GetIndexCount(),
			skill_obj->GetRangeRepresentMatrix(), vp_matrix, model_->GetDiffuseTexture(),
			light_->GetDirection(), light_->GetDiffuseColor());*/
		
		auto obj_model = skill_obj->GetModel();
		obj_model->Render(direct3D_->GetDeviceContext());

		if (obj_model->GetNormalTexture())
		{
			normalMap_shader_->Render(direct3D_->GetDeviceContext(), obj_model,
				skill_obj->GetGlobalShapeTransform(curr_time), vp_matrix,
				light_->GetDirection(), light_->GetDiffuseColor(), camera_->GetPosition());
		}
		else
		{			
			light_shader_->Render(direct3D_->GetDeviceContext(), obj_model->GetIndexCount(),
				skill_obj->GetGlobalShapeTransform(curr_time), vp_matrix, obj_model->GetDiffuseTexture(),
				light_->GetDirection(), light_->GetDiffuseColor());
		}
		
	}

	model_->Render(direct3D_->GetDeviceContext());
	for (auto& monster : monsters_)
	{
		normalMap_shader_->Render(direct3D_->GetDeviceContext(), model_->GetIndexCount(),
			monster->GetRangeRepresentMatrix(), vp_matrix, model_->GetDiffuseTexture(),
			model_->GetNormalTexture(), model_->GetEmissiveTexture(), light_->GetDirection(), light_->GetDiffuseColor(), camera_->GetPosition());
	}

	for (auto& ground : ground_)
	{
		light_shader_->Render(direct3D_->GetDeviceContext(), model_->GetIndexCount(),
			ground->GetRange().toMatrix(), vp_matrix, model_->GetDiffuseTexture(),
			light_->GetDirection(), light_->GetDiffuseColor());
	}

	gemModel_->Render(direct3D_->GetDeviceContext());
	normalMap_shader_->Render(direct3D_->GetDeviceContext(), gemModel_->GetIndexCount(),
		XMMatrixScaling(5, 5, 5) * XMMatrixTranslation(0, 0, 0), vp_matrix, gemModel_->GetDiffuseTexture(),
		gemModel_->GetNormalTexture(), gemModel_->GetEmissiveTexture(), light_->GetDirection(),
		light_->GetDiffuseColor(), camera_->GetPosition());

	// Turn off the Z buffer to begin all 2D rendering.
	direct3D_->TurnZBufferOff();

	user_interface_->Render(direct2D_.get(), texture_shader_.get(), direct3D_->GetDeviceContext(),
		character_.get(), monsters_, vp_matrix, orthoMatrix, curr_time, game_state_ == GameState::kGamePause);

	// Turn the Z buffer back on now that all 2D rendering has completed.
	direct3D_->TurnZBufferOn();


	// Present the rendered scene to the screen.
	direct3D_->EndScene();
}
