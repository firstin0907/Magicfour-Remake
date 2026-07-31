#include "core/ApplicationClass.hh"

#include <algorithm>

#include "core/D3DClass.hh"
#include "core/D2DClass.hh"
#include "core/CameraClass.hh"
#include "core/InputClass.hh"
#include "core/AnimatedObjectClass.hh"
#include "core/MonsterSpawnerClass.hh"
#include "core/GameException.hh"
#include "core/SoundClass.hh"

#include "core/common/GraphicResources.hh"

#include "graphics/ModelClass.hh"
#include "graphics/LightClass.hh"
#include "graphics/TextureClass.hh"
#include "graphics/ParticleSystemBaseClass.hh"
#include "graphics/particle-system/RadialSpreadParticleSystem.hh"
#include "graphics/particle-system/LinearMoveParticleSystem.hh"
#include "graphics/FbxModel.hh"

#include "shader/LightShaderClass.hh"
#include "shader/NormalMapShaderClass.hh"
#include "shader/FireShaderClass.hh"
#include "shader/ParticleShaderClass.hh"
#include "shader/StoneShaderClass.hh"
#include "shader/TextureShaderClass.hh"
#include "shader/CharacterShaderClass.hh"

#include "game-object/CharacterClass.hh"
#include "game-object/MonsterClass.hh"
#include "game-object/Monsters.hh"
#include "game-object/SkillObjectClass.hh"
#include "game-object/SkillObjects.hh"
#include "game-object/ItemClass.hh"

#include "map/GroundClass.hh"
#include "map/FieldClass.hh"

#include "ui/UserInterfaceClass.hh"
#include "ui/MonsterUI.hh"

#include "util/TimerClass.hh"
#include "util/RandomClass.hh"
#include "util/CollisionProcessor.hh"

#include "scenes/InitScene.hh"
#include "scenes/TitleScene.hh"
#include "scenes/GameplayScene.hh"

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
	direct3d_ = make_unique<D3DClass>(screenWidth, screenHeight,
		kVsyncEnabled, hwnd, kFullScreen, kScreenDepth, kScreenNear);
	direct2d_ = make_unique<D2DClass>(direct3d_->GetSwapChain(), hwnd);
	sound_ = make_unique<SoundClass>();

	// Load resources.
	auto texture_loader = [this](xml_node_wrapper node) -> std::shared_ptr<TextureClass>
		{
			return make_shared<TextureClass>(this->direct3d_->GetDevice(),
				node.get_required_attr("src"));
		};

	auto model_loader = [this](xml_node_wrapper node) -> std::shared_ptr<ModelClass>
		{
			std::unordered_map<std::string, std::string> textures;

			for (auto texture_node = node.first_node("Texture"); texture_node; texture_node = texture_node.next_sibling("Texture"))
			{
				std::string type = texture_node.get_required_attr("type");
				std::string src = texture_node.get_required_attr("src");
				textures[type] = src;
			}

			std::shared_ptr<TextureClass> diffuse_texture = nullptr;
			std::shared_ptr<TextureClass> normal_texture = nullptr;
			std::shared_ptr<TextureClass> emissive_texture = nullptr;

			if (textures.find("diffuse") != textures.end())
				diffuse_texture = this->graphic_resources_->textures_.get_by_path(textures["diffuse"]);
			else
				throw GAME_EXCEPTION(L"Diffuse texture is required for model: " + std::wstring(node.get_required_attr("name").begin(), node.get_required_attr("name").end()));

			if (textures.find("normal") != textures.end())
				normal_texture = this->graphic_resources_->textures_.get_by_path(textures["normal"]);

			if (textures.find("emissive") != textures.end())
				emissive_texture = this->graphic_resources_->textures_.get_by_path(textures["emissive"]);

			return make_shared<ModelClass>(this->direct3d_->GetDevice(),
				node.get_required_attr("model_path").c_str(),
				diffuse_texture,
				normal_texture,
				emissive_texture
			);
		};

	auto fbxmodel_loader = [this](xml_node_wrapper node) -> std::shared_ptr<FbxModel>
		{
			return std::make_shared<FbxModel>(this->direct3d_->GetDevice(),
				node.get_required_attr("model_path").c_str()
			);
		};

	auto particle_loader = [this](xml_node_wrapper node) -> std::shared_ptr<ParticleSystemBaseClass>
		{
			return std::make_shared<RadialSpreadParticleSystem>(
				this->direct3d_->GetDevice(),
				node.get_required_attr("src").c_str(),
				std::stof(node.get_required_attr("particleSize")),
				std::stof(node.get_required_attr("particlePerSecond")),
				std::stof(node.get_required_attr("velocity")),
				std::stof(node.get_required_attr("degree")),
				std::stoull(node.get_required_attr("lifetime")));
		};

	graphic_resources_ = make_unique<GraphicResources>();
	graphic_resources_->textures_.loadFromXML("data/resources.xml", "Texture", texture_loader);
	graphic_resources_->models_.loadFromXML("data/resources.xml", "Model", model_loader);
	graphic_resources_->fbx_models_.loadFromXML("data/resources.xml", "FbxModel", fbxmodel_loader);
	graphic_resources_->particle_system_.loadFromXML("data/resources.xml", "RadialSpreadParticleSystem", particle_loader);
	graphic_resources_->particle_system_.insert("star-spread2",
		make_unique<LinearMoveParticleSystem>(
			direct3d_->GetDevice(),
			"data/texture/particle/star1.png",
			0.2f,
			100.0f,
			200,
			DirectX::XMFLOAT3(-1, -1, -1),
			DirectX::XMFLOAT3(1, 1, 1),
			0.005f,
			0.005f
		));

	// Create and initialize the light shader object.
	shader_manager_ = make_unique<ShaderManager>(
		direct3d_->GetDevice(),
		direct3d_->GetDeviceContext(),
		hwnd
	);
	
	
	// Set model of skill object to be rendered.
	SkillObjectBead::initialize("orb", "fire-effect");
	SkillObjectSpear::initialize("spear");
	SkillObjectLeg::initialize("leg");
	SkillObjectBasic::initialize("basic");
	SkillObjectShield::initialize("shield");
	SkillObjectGuardian::initialize("orb");

	timer_ = make_unique<TimerClass>();
	timer_->Frame();

	user_interface_ = make_unique<UserInterfaceClass>(direct2d_.get(),
		direct3d_->GetDevice(), screenWidth, screenHeight);

	sound_->PlayBackground("background");

	scenes_["InitScene"]	 = make_shared<InitScene>();
	scenes_["TitleScene"] 	 = make_shared<TitleScene>(screenWidth, screenHeight, input);
	scenes_["GameplayScene"] = make_shared<GameplayScene>(screenWidth, screenHeight, input);

	current_scene_ = scenes_["InitScene"];

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

	timer_->Frame();
	current_scene_->Frame(timer_->GetTime(), timer_->GetElapsedTime(), sound_.get());
	current_scene_->Render(timer_->GetTime(), timer_->GetElapsedTime(), shader_manager_.get(), graphic_resources_.get());

	direct3d_->BeginScene(0.0f, 0.0f, 0.5f, 1.0f); // Clear the buffers to begin the scene.
	
	ProcessRenderQueue();

	direct3d_->SetDepthStencilState(D3DClass::DepthStencilMode::Disabled2D);
	direct3d_->SetAlphaBlending(D3DClass::BlendStateMode::AlphaDisable);
	current_scene_->RenderUI(timer_->GetTime(), timer_->GetElapsedTime(), user_interface_.get());
	
	direct3d_->EndScene(); // Present the rendered scene to the screen.
	
	// transition to next scene if current scene has a next scene.
	std::string next_scene_name = current_scene_->NextScene();
	if(!next_scene_name.empty())
	{
		if(scenes_.find(next_scene_name) != scenes_.end())
		{
			current_scene_->OnExit();
			current_scene_ = scenes_[next_scene_name];
			current_scene_->OnEnter();
		}
		else
		{
			throw GAME_EXCEPTION(L"Scene not found: " + std::wstring(next_scene_name.begin(), next_scene_name.end()));
		}
	}

	time_t curr_time = timer_->GetTime();
	time_t time_delta = timer_->GetElapsedTime();

	graphic_resources_->particle_system_.get("star-spread")->Frame(curr_time, time_delta, direct3d_->GetDeviceContext());
	graphic_resources_->particle_system_.get("dust-spread")->Frame(curr_time, time_delta, direct3d_->GetDeviceContext());
	graphic_resources_->particle_system_.get("star-spread2")->Frame(curr_time, time_delta, direct3d_->GetDeviceContext());


	return true;
}

void ApplicationClass::ProcessRenderQueue()
{
	direct3d_->SetDepthStencilState(D3DClass::DepthStencilMode::Default3D); 
	shader_manager_->light_shader_	  ->ProcessRenderQueue(direct3d_->GetDeviceContext());
	shader_manager_->normalMap_shader_->ProcessRenderQueue(direct3d_->GetDeviceContext());
	shader_manager_->stone_shader_	  ->ProcessRenderQueue(direct3d_->GetDeviceContext());
	shader_manager_->character_shader_->ProcessRenderQueue(direct3d_->GetDeviceContext());
	
	direct3d_->SetDepthStencilState(D3DClass::DepthStencilMode::Transparent3D);
	direct3d_->SetAlphaBlending(D3DClass::BlendStateMode::AlphaEnable); // Turn on alpha blending for the fire transparency.
	shader_manager_->fire_shader_	  ->ProcessRenderQueue(direct3d_->GetDeviceContext());

	direct3d_->SetAlphaBlending(D3DClass::BlendStateMode::AlphaAdditive);
	shader_manager_->particle_shader_ ->ProcessRenderQueue(direct3d_->GetDeviceContext());
}
