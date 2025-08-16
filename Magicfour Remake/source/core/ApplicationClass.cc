#include "core/ApplicationClass.hh"

#include <algorithm>

#include "core/D3DClass.hh"
#include "core/D2DClass.hh"
#include "graphics/ModelClass.hh"
#include "core/InputClass.hh"
#include "shader/LightShaderClass.hh"
#include "shader/NormalMapShaderClass.hh"
#include "shader/FireShaderClass.hh"
#include "graphics/LightClass.hh"
#include "shader/StoneShaderClass.hh"
#include "game-object/CharacterClass.hh"
#include "game-object/MonsterClass.hh"
#include "game-object/Monsters.hh"
#include "core/AnimatedObjectClass.hh"
#include "core/CameraClass.hh"
#include "game-object/SkillObjectClass.hh"
#include "game-object/SkillObjects.hh"
#include "map/GroundClass.hh"
#include "core/MonsterSpawnerClass.hh"
#include "util/TimerClass.hh"
#include "shader/TextureShaderClass.hh"
#include "graphics/TextureClass.hh"
#include "ui/UserInterfaceClass.hh"
#include "game-object/ItemClass.hh"
#include "util/RandomClass.hh"
#include "core/GameException.hh"
#include "core/SoundClass.hh"
#include "util/CollisionProcessor.hh"
#include "map/FieldClass.hh"

//#define DEBUG_RANGE

using namespace std;
using namespace DirectX;

constexpr float kCameraZPosition = -20.0f;
constexpr int	kCameraXLimit = 1'500'000;
constexpr int	kItemDropProbability = 50;
constexpr XMFLOAT4 kSkillColor[5] =
{
	{0, 0, 0, 1.0f},
	{0.9f, 0.1f, 0.3f, 1.0f},
	{0.2f, 0.8f, 0.1f, 1.0f},
	{0.1f, 0.3f, 0.9f, 1.0f},
	{0.2f, 0.1f, 0.1f, 1.0f}
};

ApplicationClass::ApplicationClass(int screenWidth, int screenHeight, HWND hwnd, InputClass* input)
{
	direct3D_ = make_unique<D3DClass>(screenWidth, screenHeight,
		VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	direct2D_ = make_unique<D2DClass>(direct3D_->GetSwapChain(), hwnd);
	sound_ = make_unique<SoundClass>();

	camera_ = make_unique<CameraClass>();
	camera_->SetPosition(0.0f, 0.0f, kCameraZPosition);

	model_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/abox.obj", L"data/texture/stone01.tga", L"data/texture/normal01.tga");

	fire_model_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/PlaneObject.obj", L"data/texture/fire/fire02.png", L"data/texture/fire/noise01.png", L"data/texture/fire/alpha02.png");

	planeModel_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/PlaneObject.obj", L"data/texture/stone01.tga", L"data/texture/normal01.tga");
	diamondModel_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/diamond.obj", L"data/texture/stone01.tga");
	gemModel_ = make_unique<ModelClass>(direct3D_->GetDevice(),
		"data/model/Crystal/Crystals_low.obj", L"data/model/Crystal/None_BaseColor.png",
		L"data/model/Crystal/None_Normal.png", L"data/model/Crystal/None_Emissive.png");

	rainbowTexture_ = make_unique<TextureClass>(direct3D_->GetDevice(),
		L"data/texture/skill_gauge_rainbow.png");
	backgroundTexture_ = make_unique<TextureClass>(direct3D_->GetDevice(),
		L"data/texture/background.jpg");

	// Create and initialize the light shader object.
	light_shader_ = make_unique<LightShaderClass>(direct3D_->GetDevice(), direct3D_->GetDeviceContext(), hwnd);

	// Create and initialize the light object.
	light_ = make_unique<LightClass>();
	light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	light_->SetDirection(0.0f, 0.0f, 1.0f);

	stone_shader_		= make_unique<StoneShaderClass>(direct3D_->GetDevice(), direct3D_->GetDeviceContext(), hwnd);
	texture_shader_		= make_unique<TextureShaderClass>(direct3D_->GetDevice(), direct3D_->GetDeviceContext(), hwnd);
	normalMap_shader_	= make_unique<NormalMapShaderClass>(direct3D_->GetDevice(), direct3D_->GetDeviceContext(), hwnd);
	fire_shader_		= make_unique<FireShaderClass>(direct3D_->GetDevice(), direct3D_->GetDeviceContext(), hwnd);

	// Set model of skill object to be rendered.
	SkillObjectBead::initialize(
		new ModelClass(direct3D_->GetDevice(),
			"data/model/Orb/orb_low.obj",
			L"data/model/Orb/orb_low_fragment_BaseColor.png",
			L"data/model/Orb/orb_low_fragment_Normal.png",
			L"data/model/Orb/orb_low_fragment_Emissive.png"
		),
		new ModelClass(direct3D_->GetDevice(),
			"data/model/PlaneObject.obj",
			L"data/texture/fire/fire02.png",
			L"data/texture/fire/noise01.png",
			L"data/texture/fire/alpha02.png"
		));
	SkillObjectSpear::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Blade/MagicCeramicBlade.obj",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_BaseColor.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Normal.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Emissive.jpg"
	));
	SkillObjectLeg::initialize(
		new ModelClass(direct3D_->GetDevice(),
			"data/model/Crystal/Crystals_low.obj",
			L"data/model/Crystal/None_BaseColor.png",
			L"data/model/Crystal/None_Normal.png"
		)
	);
	SkillObjectBasic::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Blade/MagicCeramicBlade.obj",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_BaseColor.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Normal.jpg",
		L"data/model/Blade/MagicCeramicBlade_MagicCeramicKnife_Emissive.jpg"
	));
	SkillObjectShield::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Shield/shield.obj",
		L"data/model/Shield/Sheld_LPFF_Sheld_BaseColor.png",
		L"data/model/Shield/Sheld_LPFF_Sheld_Normal.png"
	));
	SkillObjectGuardian::initialize(new ModelClass(direct3D_->GetDevice(),
		"data/model/Orb/orb_low.obj",
		L"data/model/Orb/orb_low_fragment_BaseColor.png",
		L"data/model/Orb/orb_low_fragment_Normal.png",
		L"data/model/Orb/orb_low_fragment_Emissive.png"
	));

	// Create character instance.
	character_ = make_unique<CharacterClass>(0, 0, input, sound_.get(), skillObjectList_.elements);

	// Temporary
	//monsters_.emplace_back(new MonsterDuck(LEFT_FORWARD, 1000));
	//monsters_.emplace_back(new MonsterOctopus(RIGHT_FORWARD, 1000));
	//for(int i = 1; i <= 10; i++) monsters_.emplace_back(new MonsterBird(RIGHT_FORWARD, 1000));

	// Set ground of field.
	field_ = make_unique<FieldClass>("data/field/field001.txt");
	
	monster_spawner_ = make_unique<MonsterSpawnerClass>();
	monster_spawner_->SetBaseTotalSpawnRate(6);
	monster_spawner_->SetIndividualSpawnRate(25, 25, 25, 25);

	timer_ = make_unique<TimerClass>();
	timer_->Frame();


	items_.Insert(new ItemClass(timer_->GetTime(), 0, 0, 0));
	items_.Insert(new ItemClass(timer_->GetTime(), 7777770, 111110, 0));
	items_.Insert(new ItemClass(timer_->GetTime(), 1231230, 1231230, 3));
	items_.Insert(new ItemClass(timer_->GetTime(), -1231230, 242320, 2));

	user_interface_ = make_unique<UserInterfaceClass>(direct2D_.get(),
		direct3D_->GetDevice(), screenWidth, screenHeight);

	sound_->PlayBackground(BackgroundSound::kSoundOnGameBackground);
	
	game_state_ = GameState::kGameRun;
	state_start_time_ = timer_->GetTime();
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
		state_start_time_ = timer_->GetTime();
	}
	else if (input->IsKeyDown(DIK_R))
	{
		timer_->Resume();
		game_state_ = GameState::kGameRun;
		// state_start_time_ will be same,
		// because curr_time of timer_ is preserved while the game paused.
	}

	timer_->Frame();

	const time_t curr_time = timer_->GetTime();
	switch (game_state_)
	{
	case GameState::kGameRun:
		GameFrame(input);
		Render();
		return true;

	case GameState::kGamePause:
		Render();
		return true;

	case GameState::kGameOver:
		GameFrame(input);
		Render();
		return true;

	default:
		throw GAME_EXCEPTION(L"Unknown GameState");
	}
}


void ApplicationClass::GameFrame(InputClass* input)
{
	time_t curr_time = timer_->GetTime();
	time_t delta_time = timer_->GetElapsedTime();

	const int GAME_OVER_SLOW = 4;
	if (character_->GetState() == CharacterState::kDie)
	{
		if (curr_time - delta_time < state_start_time_ + 1000 && state_start_time_ + 1000 <= curr_time)
		{
			sound_->PlayEffect(EffectSound::kSoundGameOver);
		}
	}
	else monster_spawner_->Frame(curr_time, delta_time, monsters_.elements);

	character_->FrameMove(curr_time, delta_time, field_->GetGrounds());
	character_->Frame(curr_time, delta_time);

	// Move skill object instances.
	skillObjectList_.FrameMove(curr_time, delta_time, field_->GetGrounds());

	// Move monsters.
	monsters_.FrameMove(curr_time, delta_time, field_->GetGrounds());

	// Move items.
	items_.FrameMove(curr_time, delta_time, field_->GetGrounds());


	// Handle collision for the gaurdians.
	// The content of this loop is proceeded at most two times at once,
	// because character_->GetGuardian(3) always returns nullptr.
	for (int i = 0; character_->GetGuardian(i) != nullptr; i++)
	{
		CollisionProcessor::Process<SkillObjectGuardian, MonsterClass>(
			character_->GetGuardian(i), monsters_, [this, curr_time](SkillObjectGuardian* skill_obj, MonsterClass* monster)
			{
				if (!skill_obj->OnCollided(monster, curr_time)) return;
				character_->AddCombo(curr_time);
			});
	}

	// Coliide check
	CollisionProcessor::Process<SkillObjectClass, MonsterClass>(
		skillObjectList_, monsters_, [this, curr_time](SkillObjectClass* skill_obj, MonsterClass* monster)
		{
			if (!skill_obj->OnCollided(monster, curr_time)) return;				
			character_->AddCombo(curr_time);
		});


	CollisionProcessor::Process<CharacterClass, MonsterClass>(
		character_.get(), monsters_, [this, curr_time](CharacterClass* character, MonsterClass* monster)
		{
			if (!character->OnCollided(curr_time, monster->GetVx())) return;

			if (character_->GetState() == CharacterState::kDie)
			{
				game_state_ = GameState::kGameOver;
				state_start_time_ = curr_time;
				sound_->PlayEffect(EffectSound::kSoundCharacterDie);
				timer_->SetGameSpeed(250);
			}
			else
			{
				sound_->PlayEffect(EffectSound::kSoundCharacterDamage);
				if (character_->GetSkill(0).skill_type == 0)
				{
					sound_->PlayEffect(EffectSound::kSoundHeartbeat);
				}
			}
		});


	CollisionProcessor::Process<CharacterClass, ItemClass>(
		character_.get(), items_, [this, curr_time](CharacterClass* character, ItemClass* item)
		{
			character->LearnSkill(item->GetType(), curr_time);
			item->SetState(ItemState::kDie, curr_time);

			sound_->PlayEffect(EffectSound::kSoundSkillLearn);
		});


	// Process some work which should be conducted per frame,
	// for skill object instances
	skillObjectList_.Frame(curr_time, delta_time);

	// Process some work which should be conducted per frame,
	// for monster object instances
	monsters_.Frame(curr_time, delta_time, [this, curr_time](IGameObject* obj)
		{
			auto monster = static_cast<MonsterClass*>(obj);
			this->items_.Insert(new ItemClass(curr_time, monster->GetPosition().x,
				monster->GetPosition().y, monster->GetType()));
		});

	items_.Frame(curr_time, delta_time);
}

void ApplicationClass::Render()
{
	const float camera_x = SATURATE(-kCameraXLimit, character_->GetPosition().x, kCameraXLimit) * kScope;
	const float camera_y = max(0, character_->GetPosition().y + 200'000) * kScope;
	camera_->SetPosition(camera_x, camera_y, kCameraZPosition);

	XMMATRIX viewMatrix, projectionMatrix, orthoMatrix;
	time_t curr_time = timer_->GetTime();

	// Clear the buffers to begin the scene.
	direct3D_->BeginScene(0.0f, 0.0f, 0.5f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera_->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	camera_->GetViewMatrix(viewMatrix);
	direct3D_->GetProjectionMatrix(projectionMatrix);
	direct3D_->GetOrthoMatrix(orthoMatrix);

	XMMATRIX vp_matrix = viewMatrix * projectionMatrix;

	const static XMMATRIX kBackgroundMarix = XMMatrixScaling(192.0f, 153.6f, 1) * XMMatrixTranslation(0, 0, 100.0f);
	light_shader_->PushRenderQueue(planeModel_.get(), kBackgroundMarix,
		backgroundTexture_->GetTexture());

	vector<XMMATRIX> char_model_matrices;
	character_->GetShapeMatrices(curr_time, char_model_matrices);


	ID3D11ShaderResourceView* char_texture = model_->GetDiffuseTexture();
	if (curr_time <= character_->GetTimeInvincibleEnd()) char_texture = rainbowTexture_->GetTexture();

	for (auto& box : char_model_matrices) {
		light_shader_->PushRenderQueue(model_.get(),
			box * character_->GetLocalWorldMatrix(), char_texture);
	}


	constexpr float kBoxSize = 0.32f;
	XMMATRIX skill_stone_pos =
		XMMatrixRotationX(XM_PI / 18) * XMMatrixRotationY(curr_time * 0.001f) * XMMatrixRotationX(-XM_PI / 10) *
		XMMatrixScaling(kBoxSize, kBoxSize * 1.2f, kBoxSize) * XMMatrixTranslation(-1.3f, 4.0f, 0.f) *
		character_->GetLocalWorldMatrix();

	
	auto drawStone = [this, &vp_matrix](const XMMATRIX& shape, int type, float brightness = 0.0f)
		{
			XMFLOAT4 skill_color = kSkillColor[type];
			if (brightness > 0.0f)
			{
				XMFLOAT4 skill_color = kSkillColor[type];
				skill_color.x += (1 - skill_color.x) * brightness * 0.6;
				skill_color.y += (1 - skill_color.y) * brightness * 0.6;
				skill_color.z += (1 - skill_color.z) * brightness * 0.6;
				skill_color.w += (1 - skill_color.w) * brightness * 0.6;
			}
			this->stone_shader_->PushRenderQueue(diamondModel_.get(), shape, skill_color);
		};

	int sb_count = 0;
	time_t elapsed_time_for_sb = curr_time - character_->GetSkill(3).learned_time;
	for (int i = 0; i < 4; i++)
	{
		const int type = character_->GetSkill(i).skill_type;
		const int power = character_->GetSkill(i).skill_power;
		if (type == 0) break;

		const float scale = (power + 13) * 0.04f;
		float brightness = 0.0f;

		if (character_->GetSkill(i).is_part_of_skillbonus)
		{
			const time_t local_time = (elapsed_time_for_sb + 5'000 - 400 * ++sb_count) % 5'000;
			if(local_time <= 800.0f) brightness = max(0.0f, sin(local_time / 200.0f));

			const time_t global_time = (elapsed_time_for_sb + 5'000 - 2'000) % 5'000;
			if(global_time <= 1200.0f) brightness = max(0.0f, sin(global_time / 300.0f));
		}

		drawStone(XMMatrixScaling(scale, scale, scale) * skill_stone_pos * XMMatrixTranslation(0, -0.6f * i, 0),
			type, brightness);
	}

	// Draw Items
	for (auto& obj : items_.elements)
	{
		auto item = static_cast<ItemClass*>(obj.get());
		drawStone(item->GetShapeMatrix(curr_time) * item->GetLocalWorldMatrix(), item->GetType());
	}


	// Draw skill object
	for (auto& obj : skillObjectList_.elements)
	{		
		auto skill_obj = static_cast<SkillObjectClass*>(obj.get());
		auto obj_model = skill_obj->GetModel();

		if (obj_model->GetNormalTexture())
		{
			normalMap_shader_->PushRenderQueue(obj_model,
				skill_obj->GetGlobalShapeTransform(curr_time));
		}
		else
		{			
			light_shader_->PushRenderQueue(obj_model,
				skill_obj->GetGlobalShapeTransform(curr_time), obj_model->GetDiffuseTexture());
		}
		
	}
	if (character_->GetSkillBonus() == SkillBonus::BONUS_ONE_PAIR ||
		character_->GetSkillBonus() == SkillBonus::BONUS_TWO_PAIR)
	{
		ModelClass* obj_model = character_->GetGuardian(0)->GetModel();

		for (int i = 0; character_->GetGuardian(i) != nullptr; i++)
		{
			SkillObjectGuardian* skill_obj = character_->GetGuardian(i);
			if (obj_model->GetNormalTexture())
			{
				normalMap_shader_->PushRenderQueue(obj_model,
					skill_obj->GetGlobalShapeTransform(curr_time));
			}
			else
			{

				light_shader_->PushRenderQueue(obj_model,
					skill_obj->GetGlobalShapeTransform(curr_time),
					obj_model->GetDiffuseTexture());
			}

		}
	}


	for (auto& obj: monsters_.elements)
	{
		MonsterClass* monster = static_cast<MonsterClass*>(obj.get());

		light_shader_->PushRenderQueue(model_.get(),
			monster->GetRangeRepresentMatrix(), model_->GetDiffuseTexture());
	}

	for (auto& ground : field_->GetGrounds())
	{
		light_shader_->PushRenderQueue(model_.get(),
			ground.GetRange().toMatrix(), model_->GetDiffuseTexture());

	}

	normalMap_shader_->PushRenderQueue(gemModel_.get(),
		XMMatrixScaling(3, 3, 3) * XMMatrixTranslation(1750000 * kScope, (kGroundY - 50000) * kScope, +0.5f),
		gemModel_->GetDiffuseTexture(), gemModel_->GetNormalTexture(), gemModel_->GetEmissiveTexture());
	
	normalMap_shader_->PushRenderQueue(gemModel_.get(),
		XMMatrixScaling(4, 4, 4) * XMMatrixTranslation(1950000 * kScope, (kGroundY - 50000)* kScope, 0.0f),
		gemModel_->GetDiffuseTexture(), gemModel_->GetNormalTexture(), gemModel_->GetEmissiveTexture());


#ifdef DEBUG_RANGE

	light_shader_->PushRenderQueue(planeModel_.get(), character_->GetRangeRepresentMatrix(), model_->GetDiffuseTexture());

	for (auto& obj : skillObjectList_.elements)
	{
		auto skill_obj = static_cast<SkillObjectClass*>(obj.get());
		light_shader_->PushRenderQueue(planeModel_.get(), skill_obj->GetRangeRepresentMatrix(), model_->GetDiffuseTexture());
	}

#endif

	fire_shader_->PushRenderQueue(
		planeModel_.get(),
		XMMatrixScaling(2.0f, 2.0f, 1)* vp_matrix,
		fire_model_->GetDiffuseTexture(),
		fire_model_->GetNormalTexture(),
		fire_model_->GetEmissiveTexture(),
		{ 1.3f, 2.1f, 2.3f },
		{ 1.0f, 2.0f, 3.0f },
		{ 0.1f, 0.2f },
		{ 0.1f, 0.3f },
		{ 0.1f, 0.1f },
		0.8f, 0.0f);


	light_shader_	 ->ProcessRenderQueue(vp_matrix, light_->GetDirection(), light_->GetDiffuseColor());
	normalMap_shader_->ProcessRenderQueue(vp_matrix, light_->GetDirection(), light_->GetDiffuseColor(), camera_->GetPosition());
	stone_shader_	 ->ProcessRenderQueue(vp_matrix, light_->GetDirection(), camera_->GetPosition());

	direct3D_->EnableAlphaBlending(); // Turn on alpha blending for the fire transparency.
	fire_shader_	 ->ProcessRenderQueue(curr_time * 0.0004f);
	direct3D_->DisableAlphaBlending();

	// Turn off the Z buffer to begin all 2D rendering.
	direct3D_->TurnZBufferOff();
		
	user_interface_->Begin2dDraw(direct2D_.get());

	const XMMATRIX ortho_inv = XMMatrixInverse(nullptr, orthoMatrix);
	float screen_x, screen_y;
	for (auto& object : monsters_.elements)
	{
		MonsterClass* monster = static_cast<MonsterClass*>(object.get());

		user_interface_->CalculateScreenPos(monster->GetLocalWorldMatrix() * vp_matrix,
			ortho_inv, screen_x, screen_y);


		if (monster->GetState() == MonsterState::kStopEmbryo)
		{
			user_interface_->DrawWarningVerticalRect(direct2D_.get(), screen_x, 50, monster->GetStateTime(curr_time) / 700.0f);
		}
		else
		{
			user_interface_->DrawMonsterHp(direct2D_.get(), screen_x, screen_y - 23,
				monster->GetHpRatio(), monster->GetPrevHpRatio());
		}


	}

	user_interface_->CalculateScreenPos(character_->GetLocalWorldMatrix() * vp_matrix,
		ortho_inv, screen_x, screen_y);
	user_interface_->DrawCharacterInfo(direct2D_.get(), character_.get(),
		screen_x, screen_y, curr_time);

	// Get the coordinate of stone with respect to screen coordinate.
	for (int i = 0; i < 4; i++)
	{
		user_interface_->CalculateScreenPos(skill_stone_pos * XMMatrixTranslation(0, -0.6f * i, 0) * vp_matrix, ortho_inv, screen_x, screen_y);
		if (character_->GetSkill(i).skill_type)
		{
			user_interface_->DrawSkillPower(direct2D_.get(),
				character_->GetSkill(i).skill_type,
				character_->GetSkill(i).skill_power,
				curr_time - character_->GetSkill(i).learned_time,
				screen_x, screen_y);
		}
	}
	user_interface_->DrawFps(direct2D_.get(),
		timer_->GetActualTime(), timer_->GetActualElapsedTime());


	switch (game_state_)
	{
	case GameState::kGamePause:
		user_interface_->DrawPauseMark(direct2D_.get());
		break;
	case GameState::kGameOver:
		user_interface_->DrawGameoverScreen(direct2D_.get(),
			character_->GetStateTime(curr_time));
		break;
	}

	user_interface_->End2dDraw(direct2D_.get());


	// Turn the Z buffer back on now that all 2D rendering has completed.
	direct3D_->TurnZBufferOn();



	// Present the rendered scene to the screen.
	direct3D_->EndScene();
}
