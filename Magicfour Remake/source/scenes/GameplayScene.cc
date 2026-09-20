#include "scenes/GameplayScene.hh"

#include "core/CameraClass.hh"
#include "core/InputClass.hh"
#include "core/AnimatedObjectClass.hh"
#include "core/MonsterSpawnerClass.hh"
#include "core/GameException.hh"
#include "core/SoundClass.hh"

#include "core/common/GraphicResources.hh"
#include "core/configuration/ConfigManager.hh"

#include "graphics/LightClass.hh"

#include "game-object/CharacterClass.hh"
#include "game-object/MonsterClass.hh"
#include "game-object/Monsters.hh"
#include "game-object/SkillObjectClass.hh"
#include "game-object/SkillObjects.hh"
#include "game-object/ItemClass.hh"

#include "shader/ShaderManager.hh"
#include "shader/LightShaderClass.hh"
#include "shader/NormalMapShaderClass.hh"
#include "shader/StoneShaderClass.hh"
#include "shader/FireShaderClass.hh"
#include "shader/CharacterShaderClass.hh"
#include "shader/ParticleShaderClass.hh"

#include "map/GroundClass.hh"
#include "map/FieldClass.hh"

#include "ui/UserInterfaceClass.hh"

#include "util/CollisionProcessor.hh"

//#define DEBUG_RANGE

constexpr float kCameraZPosition = -20.0f;
constexpr int	kCameraXLimit = 1'500'000;
constexpr int	kItemDropProbability = 50;

using namespace std;
using namespace DirectX;

GameplayScene::GameplayScene(ConfigManager* config_manager, InputClass* input) :
    config_manager_(config_manager), input_(input), next_scene_name_("")
{
	auto resolution_string = config_manager->GetConfigValue<std::wstring>(L"Resolution");
	
}

GameplayScene::~GameplayScene()
{
}

void GameplayScene::OnEnter()
{
	game_time_ = 0;

    camera_ = make_unique<CameraClass>(
		config_manager_->GetResolution().first,
		config_manager_->GetResolution().second,
		XM_PIDIV4, 0.1f, 1000.0f);
	camera_->SetPosition(0.0f, 0.0f, kCameraZPosition);
    
	// Create and initialize the light object.
	light_ = make_unique<LightClass>();
	light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	light_->SetDirection(0.0f, 0.0f, 1.0f);

    // Create character instance.
	character_ = make_unique<CharacterClass>(0, 0, input_, skill_object_list_.elements);

	// Temporary
	monsters_.Insert(new MonsterStop(1000));
	monsters_.Insert(new MonsterDuck(direction_t::kLeftForword, 1000));
	//monsters_.emplace_back(new MonsterOctopus(kRightForward, 1000));
	//for(int i = 1; i <= 10; i++) monsters_.emplace_back(new MonsterBird(kRightForward, 1000));

	// Set ground of field.
	field_ = make_unique<FieldClass>("data/field/field001.txt");
	
	monster_spawner_ = make_unique<MonsterSpawnerClass>();
	monster_spawner_->SetBaseTotalSpawnRate(6);
	monster_spawner_->SetIndividualSpawnRate(25, 25, 25, 25);

	game_state_ = GameState::kGameRun;
}

void GameplayScene::OnExit()
{
}


void GameplayScene::Frame(time_t scene_time, time_t time_delta, SoundClass* sound_manager)
{
	// Game State Transition
	if (game_state_ == GameState::kGameRun && input_->IsKeyDown(DIK_P))
	{
		game_state_ = GameState::kGamePause;
		state_start_time_ = scene_time;
	}
	else if (game_state_ == GameState::kGamePause && input_->IsKeyDown(DIK_R))
	{
		game_state_ = GameState::kGameRun;
		state_start_time_ = scene_time;
	}

	// Calculate time according to game state.
	if (game_state_ == GameState::kGameRun)
	{
		game_time_ += time_delta;
		game_time_delta_ = time_delta;
	}
	else if (game_state_ == GameState::kGameOver)
	{
		constexpr int kGameOverSlow = 4;
		game_time_ += ((scene_time + time_delta) / kGameOverSlow) - (scene_time / kGameOverSlow);
		game_time_delta_ = ((scene_time + time_delta) / kGameOverSlow) - (scene_time / kGameOverSlow);
	}

	// Process Game Object
	if (game_state_ == GameState::kGameRun || game_state_ == GameState::kGameOver)
	{
		if (character_->GetState() == CharacterState::kDie)
		{
			if (game_time_ - game_time_delta_ < state_start_time_ + 1000 && state_start_time_ + 1000 <= game_time_)
			{
				sound_manager->PlayEffect("gameover");
			}
		}
		else monster_spawner_->Frame(game_time_, game_time_delta_, monsters_.elements);

		character_->FrameMove(game_time_, game_time_delta_, field_.get());
		character_->Frame(game_time_, game_time_delta_, sound_manager);

		// Move skill object instances.
		skill_object_list_.FrameMove(game_time_, game_time_delta_, field_.get());

		// Move monsters.
		monsters_.FrameMove(game_time_, game_time_delta_, field_.get());

		// Move items.
		items_.FrameMove(game_time_, game_time_delta_, field_.get());
	}


	// Handle collision
	if(game_state_ == GameState::kGameRun)
	{
		// Handle collision for the gaurdians.
		// The content of this loop is proceeded at most two times at once,
		// because character_->GetGuardian(3) always returns nullptr.
		for (int i = 0; character_->GetGuardian(i) != nullptr; i++)
		{
			CollisionProcessor::Process<SkillObjectGuardian, MonsterClass>(
				character_->GetGuardian(i), monsters_, [this](SkillObjectGuardian* skill_obj, MonsterClass* monster)
				{
					if (!skill_obj->OnCollided(monster, game_time_)) return;
					character_->AddCombo(game_time_);
				});
		}

		// Coliide check
		CollisionProcessor::Process<SkillObjectClass, MonsterClass>(
			skill_object_list_, monsters_, [this](SkillObjectClass* skill_obj, MonsterClass* monster)
			{
				if (!skill_obj->OnCollided(monster, game_time_)) return;
				character_->AddCombo(game_time_);
			});
		

		CollisionProcessor::Process<CharacterClass, MonsterClass>(
			character_.get(), monsters_, [this, sound_manager](CharacterClass* character, MonsterClass* monster)
			{
				if (!character->OnCollided(game_time_, monster->GetVx())) return;

				if (character_->GetState() == CharacterState::kDie)
				{
					game_state_ = GameState::kGameOver;
					state_start_time_ = game_time_;
					sound_manager->PlayEffect("character_death");
				}
				else
				{
					sound_manager->PlayEffect("character_damage");
					if (character_->GetSkill(0).skill_type == 0)
					{
						sound_manager->PlayEffect("heartbeat");
					}
				}
			});
			

		CollisionProcessor::Process<CharacterClass, ItemClass>(
			character_.get(), items_, [this, sound_manager](CharacterClass* character, ItemClass* item)
			{
				character->LearnSkill(item->GetType(), game_time_);
				item->SetState(ItemState::kDie, game_time_);

				sound_manager->PlayEffect("skill_learn");
			});

	}


	// Call Frame() of each objects.
	if (game_state_ == GameState::kGameRun || game_state_ == GameState::kGameOver)
	{
		// Process some work which should be conducted per frame,
		// for skill object instances
		skill_object_list_.Frame(game_time_, game_time_delta_, sound_manager);

		// Process some work which should be conducted per frame,
		// for monster object instances
		monsters_.Frame(game_time_, game_time_delta_, sound_manager, [this, sound_manager](IGameObject* obj)
			{
				auto monster = static_cast<MonsterClass*>(obj);
				this->items_.Insert(new ItemClass(game_time_, monster->GetPosition().x,
					monster->GetPosition().y, monster->GetType()));
			});

		items_.Frame(game_time_, game_time_delta_, sound_manager);
	}
	
}

void GameplayScene::Render(time_t scene_time, time_t time_delta, ShaderManager* shader_manager, GraphicResources* graphic_resources)
{
    const float camera_x = std::clamp(character_->GetPosition().x, -kCameraXLimit, kCameraXLimit) * kScope;
	const float camera_y = max(0, character_->GetPosition().y + 200'000) * kScope;

	camera_->SetPosition(camera_x, camera_y, kCameraZPosition);

	// Generate the view matrix based on the camera's position.
	camera_->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	character_->Draw(game_time_, game_time_delta_, shader_manager, graphic_resources);

	// Draw Items
	items_.Draw(game_time_, game_time_delta_, shader_manager, graphic_resources);
	skill_object_list_.Draw(game_time_, game_time_delta_, shader_manager, graphic_resources);

	monsters_.Draw(game_time_, game_time_delta_, shader_manager, graphic_resources);
	field_->Draw(game_time_, game_time_delta_, shader_manager, graphic_resources);

	const XMMATRIX vp_matrix = camera_->GetViewMatrix() * camera_->GetProjectionMatrix();

	shader_manager->light_shader_->EnableShaderForFrame(vp_matrix, light_->GetDirection(), light_->GetDiffuseColor());
	shader_manager->normalMap_shader_->EnableShaderForFrame(vp_matrix, light_->GetDirection(), light_->GetDiffuseColor(), camera_->GetPosition());
	shader_manager->stone_shader_->EnableShaderForFrame(vp_matrix, light_->GetDirection(), camera_->GetPosition());
	shader_manager->character_shader_->EnableShaderForFrame(vp_matrix);
	shader_manager->fire_shader_->EnableShaderForFrame(vp_matrix, game_time_ * 0.0004f);
	shader_manager->particle_shader_->EnableShaderForFrame(vp_matrix);
}

void GameplayScene::RenderUI(time_t scene_time, time_t time_delta, UserInterfaceClass* user_interface)
{
	const XMMATRIX vp_matrix = camera_->GetViewMatrix() * camera_->GetProjectionMatrix();

	user_interface->Begin2dDraw(vp_matrix, camera_->GetOrthoMatrix());

	user_interface->UpdateSystemContext(game_state_, game_time_);
	monsters_.DrawUI(game_time_, time_delta, user_interface);
	character_->DrawUI(game_time_, time_delta, user_interface);
	user_interface->DrawSystemUI();

	user_interface->End2dDraw();
}


const std::string& GameplayScene::NextScene() const
{
    return next_scene_name_;
}