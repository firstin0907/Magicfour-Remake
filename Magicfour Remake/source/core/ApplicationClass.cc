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

#define DEBUG_RANGE

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

	models_.loadFromXML("data/resources.xml", "Model",
		[this](xml_node_wrapper node) -> std::unique_ptr<ModelClass>
		{
			return make_unique<ModelClass>(this->direct3D_->GetDevice(),
				node.get_required_attr("src"),
				node.get_required_attr("texture"),
				node.get_attr("normal"),
				node.get_attr("emissive")
			);
		});

	textures_.loadFromXML("data/resources.xml", "Texture",
		[this](xml_node_wrapper node) -> std::unique_ptr<TextureClass>
		{
			return make_unique<TextureClass>(this->direct3D_->GetDevice(),
				node.get_required_attr("src"));
		});

	// Create and initialize the light shader object.
	shader_manager_ = make_unique<ShaderManager>(
		direct3D_->GetDevice(),
		direct3D_->GetDeviceContext(),
		hwnd
	);
	
	// Create and initialize the light object.
	light_ = make_unique<LightClass>();
	light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	light_->SetDirection(0.0f, 0.0f, 1.0f);

	
	// Set model of skill object to be rendered.
	SkillObjectBead::initialize("orb", "fire-effect");
	SkillObjectSpear::initialize("spear");
	SkillObjectLeg::initialize("leg");
	SkillObjectBasic::initialize("basic");
	SkillObjectShield::initialize("shield");
	SkillObjectGuardian::initialize("orb");

	// Create character instance.
	character_ = make_unique<CharacterClass>(0, 0, input, sound_.get(), skillObjectList_.elements);

	// Temporary
	monsters_.Insert(new MonsterStop(1000));
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

	sound_->PlayBackground("background");
	
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
			sound_->PlayEffect("gameover");
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
				sound_->PlayEffect("character_death");
				timer_->SetGameSpeed(250);
			}
			else
			{
				sound_->PlayEffect("character_damage");
				if (character_->GetSkill(0).skill_type == 0)
				{
					sound_->PlayEffect("heartbeat");
				}
			}
		});


	CollisionProcessor::Process<CharacterClass, ItemClass>(
		character_.get(), items_, [this, curr_time](CharacterClass* character, ItemClass* item)
		{
			character->LearnSkill(item->GetType(), curr_time);
			item->SetState(ItemState::kDie, curr_time);

			sound_->PlayEffect("skill_learn");
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
	time_t time_delta = timer_->GetElapsedTime();

	// Clear the buffers to begin the scene.
	direct3D_->BeginScene(0.0f, 0.0f, 0.5f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera_->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	camera_->GetViewMatrix(viewMatrix);
	direct3D_->GetProjectionMatrix(projectionMatrix);
	direct3D_->GetOrthoMatrix(orthoMatrix);

	const XMMATRIX vp_matrix = viewMatrix * projectionMatrix;

	const static XMMATRIX kBackgroundMarix = XMMatrixScaling(192.0f, 153.6f, 1) * XMMatrixTranslation(0, 0, 100.0f);
	shader_manager_->light_shader_->PushRenderQueue(models_.get("plane"), kBackgroundMarix,
		textures_.get("background")->GetTexture());


	character_->Draw(curr_time, time_delta, shader_manager_.get(), models_, textures_);

	// Draw Items
	items_.Draw(curr_time, time_delta, shader_manager_.get(), models_, textures_);
	skillObjectList_.Draw(curr_time, time_delta, shader_manager_.get(), models_, textures_);

	for (int i = 0; character_->GetGuardian(i) != nullptr; i++)
	{
		character_->GetGuardian(i)->Draw(curr_time, time_delta, shader_manager_.get(), models_, textures_);
	}

	monsters_.Draw(curr_time, time_delta, shader_manager_.get(), models_, textures_);


	for (auto& ground : field_->GetGrounds())
	{
		// for grass
		rect_t grass_range = ground.GetRange();
		const int grass_drawing_steps = grass_range.get_w() / 100000 + 1;
		for (long long i = 0; i < grass_drawing_steps; i++)
		{
			rect_t grass_section_range = grass_range;

			const long long curr_x = grass_range.x1 + grass_range.get_w() * i / grass_drawing_steps;
			const long long next_x = grass_range.x1 + grass_range.get_w() * (i + 1) / grass_drawing_steps;
			grass_section_range.x1 = curr_x, grass_section_range.x2 = next_x;

			const XMMATRIX&& grass_matrix = XMMatrixRotationY(3 * M_PI_2) * XMMatrixRotationZ(3 * M_PI_2) *
				XMMatrixTranslation(0.0f, 0.0f, -0.0001f) *
				grass_section_range.toMatrix();

			shader_manager_->fire_shader_->PushRenderQueue(
				models_.get("cube"),
				grass_matrix,
				models_.get("grass")->GetDiffuseTexture(),
				models_.get("grass")->GetNormalTexture(),
				models_.get("grass")->GetEmissiveTexture(),
				{ -0.3f, -0.1f, -0.3f },
				{ 1.0f, 2.0f, 3.0f },
				{ 0.1f, 0.2f },
				{ 0.1f, 0.3f },
				{ 0.1f, 0.1f },
				0.4f, 0.0f);
		}

		rect_t ground_range = ground.GetRange();
		const int ground_drawing_steps = grass_range.get_w() / 420000 + 1;
		for (long long i = 0; i < ground_drawing_steps; i++)
		{
			// for ground
			rect_t ground_display_range = ground_range;

			const long long curr_x = ground_range.x1 + ground_range.get_w() * i / ground_drawing_steps;
			const long long next_x = ground_range.x1 + ground_range.get_w() * (i + 1) / ground_drawing_steps;
			ground_display_range.x1 = curr_x, ground_display_range.x2 = next_x;

			shader_manager_->light_shader_->PushRenderQueue(models_.get("cube"),
				ground_display_range.toMatrix(), models_.get("cube")->GetDiffuseTexture());
		}
	}

	shader_manager_->normalMap_shader_->PushRenderQueue(models_.get("gem"),
		XMMatrixScaling(3, 3, 3) * XMMatrixTranslation(1750000 * kScope, (kGroundY - 50000) * kScope, +0.5f),
		models_.get("gem")->GetDiffuseTexture(), models_.get("gem")->GetNormalTexture(), models_.get("gem")->GetEmissiveTexture());
	
	shader_manager_->normalMap_shader_->PushRenderQueue(models_.get("gem"),
		XMMatrixScaling(4, 4, 4) * XMMatrixTranslation(1950000 * kScope, (kGroundY - 50000)* kScope, 0.0f),
		models_.get("gem")->GetDiffuseTexture(), models_.get("gem")->GetNormalTexture(), models_.get("gem")->GetEmissiveTexture());


#ifdef DEBUG_RANGE

	shader_manager_->light_shader_->PushRenderQueue(
		models_.get("plane"), character_->GetRangeRepresentMatrix(), models_.get("cube")->GetDiffuseTexture());

	for (auto& obj : skillObjectList_.elements)
	{
		auto skill_obj = static_cast<SkillObjectClass*>(obj.get());
		shader_manager_->light_shader_->PushRenderQueue(
			models_.get("plane"), skill_obj->GetRangeRepresentMatrix(), models_.get("cube")->GetDiffuseTexture());
	}

	for (auto& obj : monsters_.elements)
	{
		auto skill_obj = static_cast<MonsterClass*>(obj.get());
		shader_manager_->light_shader_->PushRenderQueue(
			models_.get("plane"), skill_obj->GetRangeRepresentMatrix(), models_.get("cube")->GetDiffuseTexture());
	}

#endif

	shader_manager_->fire_shader_->PushRenderQueue(
		models_.get("plane"),
		XMMatrixScaling(2.0f, 2.0f, 1),
		models_.get("fire")->GetDiffuseTexture(),
		models_.get("fire")->GetNormalTexture(),
		models_.get("fire")->GetEmissiveTexture(),
		{ 1.3f, 2.1f, 2.3f },
		{ 1.0f, 2.0f, 3.0f },
		{ 0.1f, 0.2f },
		{ 0.1f, 0.3f },
		{ 0.1f, 0.1f },
		0.8f, 0.0f);


	shader_manager_->light_shader_	  ->ProcessRenderQueue(direct3D_->GetDeviceContext(), vp_matrix, light_->GetDirection(), light_->GetDiffuseColor());
	shader_manager_->normalMap_shader_->ProcessRenderQueue(direct3D_->GetDeviceContext(), vp_matrix, light_->GetDirection(), light_->GetDiffuseColor(), camera_->GetPosition());
	shader_manager_->stone_shader_	  ->ProcessRenderQueue(direct3D_->GetDeviceContext(), vp_matrix, light_->GetDirection(), camera_->GetPosition());

	direct3D_->EnableAlphaBlending(); // Turn on alpha blending for the fire transparency.
	shader_manager_->fire_shader_	  ->ProcessRenderQueue(direct3D_->GetDeviceContext(), vp_matrix, curr_time * 0.0004f);
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

	XMMATRIX skill_stone_pos = character_->GetSkillStonePos(curr_time);
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
