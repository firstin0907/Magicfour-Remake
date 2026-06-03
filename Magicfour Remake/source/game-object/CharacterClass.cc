#include "game-object/CharacterClass.hh"

#include <Windows.h>

#include "core/global.hh"

#include "core/InputClass.hh"
#include "core/AnimatedObjectClass.hh"
#include "game-object/SkillObjects.hh"
#include "map/FieldClass.hh"
#include "core/SoundClass.hh"
#include "util/RandomClass.hh"

#include "graphics/ModelClass.hh"
#include "graphics/TextureClass.hh"
#include "graphics/FbxModel.hh"
#include "graphics/ParticleSystemBaseClass.hh"

#include "shader/CharacterShaderClass.hh"
#include "shader/ShaderManager.hh"
#include "shader/LightShaderClass.hh"
#include "shader/StoneShaderClass.hh"
#include "shader/ParticleShaderClass.hh"

#include "util/ResourceMap.hh"

using namespace DirectX;
using namespace std;

constexpr int kSkillCooltime = 800;
constexpr int kComboDuration = 5'000;
constexpr int kInvincibleDuration = 5'000;
constexpr int kWalkSpd = 700, kRunSpd = 1300;

constexpr double kYRotationDeg = XM_PIDIV2;


namespace
{
	std::unordered_map<std::string, std::string> kBvhToFbx = {
		// --- Root & Body Core (몸통) ---
		// {"hip",            "rp_nathan_animated_003_walking_hip"},
		// {"abdomen",        "rp_nathan_animated_003_walking_spine_01"},
		// {"chest_mid",      "rp_nathan_animated_003_walking_spine_02"}, // 추가: 척추 중간
		// {"chest",          "rp_nathan_animated_003_walking_spine_03"},
		// {"neck",           "rp_nathan_animated_003_walking_neck"},
		// {"head",           "rp_nathan_animated_003_walking_head"},

		// // --- Face & Eyes (얼굴) ---
		// {"leftEye",        "rp_nathan_animated_003_walking_eye_l"},
		// {"rightEye",       "rp_nathan_animated_003_walking_eye_r"},
		// {"jaw",            "rp_nathan_animated_003_walking_jaw"},       // 추가
		// {"eyebrow_l",      "rp_nathan_animated_003_walking_eyebrow_l"}, // 추가
		// {"eyebrow_r",      "rp_nathan_animated_003_walking_eyebrow_r"}, // 추가

		// // --- Right Arm (오른팔) ---
		// {"rCollar",        "rp_nathan_animated_003_walking_shoulder_r"},
		 {"rShldr",         "rp_nathan_animated_003_walking_upperarm_r"},
		// {"rUpperArmTwist", "rp_nathan_animated_003_walking_upperarm_twist_r"}, // 추가
		// {"rForeArm",       "rp_nathan_animated_003_walking_lowerarm_r"},
		// {"rLowerArmTwist", "rp_nathan_animated_003_walking_lowerarm_twist_r"}, // 추가
		// {"rHand",          "rp_nathan_animated_003_walking_hand_r"},

		// // --- Left Arm (왼팔) ---
		// {"lCollar",        "rp_nathan_animated_003_walking_shoulder_l"},
		// {"lShldr",         "rp_nathan_animated_003_walking_upperarm_l"},
		// {"lUpperArmTwist", "rp_nathan_animated_003_walking_upperarm_twist_l"}, // 추가
		// {"lForeArm",       "rp_nathan_animated_003_walking_lowerarm_l"},
		// {"lLowerArmTwist", "rp_nathan_animated_003_walking_lowerarm_twist_l"}, // 추가
		// {"lHand",          "rp_nathan_animated_003_walking_hand_l"},

		// --- Right Leg (오른다리) ---
		//{"rButtock",       "rp_nathan_animated_003_walking_upperleg_twist_r"},
		//{"rThigh",         "rp_nathan_animated_003_walking_upperleg_r"},
		//{"rShin",          "rp_nathan_animated_003_walking_lowerleg_r"},
		//{"rLowerLegTwist", "rp_nathan_animated_003_walking_lowerleg_twist_r"}, // 추가
		//{"rFoot",          "rp_nathan_animated_003_walking_foot_r"},
		//{"rBall",          "rp_nathan_animated_003_walking_ball_r"},          // 추가: 발가락

		// // --- Left Leg (왼다리) ---
		// {"lButtock",       "rp_nathan_animated_003_walking_upperleg_twist_l"},
		// {"lThigh",         "rp_nathan_animated_003_walking_upperleg_l"},
		// {"lShin",          "rp_nathan_animated_003_walking_lowerleg_l"},
		// {"lLowerLegTwist", "rp_nathan_animated_003_walking_lowerleg_twist_l"}, // 추가
		// {"lFoot",          "rp_nathan_animated_003_walking_foot_l"},
		// {"lBall",          "rp_nathan_animated_003_walking_ball_l"},          // 추가: 발가락

		// // --- Right Fingers (오른손가락 - 3마디 완성) ---
		// {"rThumb1",  "rp_nathan_animated_003_walking_thumb_01_r"},
		// {"rThumb2",  "rp_nathan_animated_003_walking_thumb_02_r"},
		// {"rThumb3",  "rp_nathan_animated_003_walking_thumb_03_r"},
		// {"rIndex1",  "rp_nathan_animated_003_walking_index_01_r"},
		// {"rIndex2",  "rp_nathan_animated_003_walking_index_02_r"},
		// {"rIndex3",  "rp_nathan_animated_003_walking_index_03_r"},
		// {"rMid1",    "rp_nathan_animated_003_walking_middle_01_r"},
		// {"rMid2",    "rp_nathan_animated_003_walking_middle_02_r"},
		// {"rMid3",    "rp_nathan_animated_003_walking_middle_03_r"},
		// {"rRing1",   "rp_nathan_animated_003_walking_ring_01_r"},
		// {"rRing2",   "rp_nathan_animated_003_walking_ring_02_r"},
		// {"rRing3",   "rp_nathan_animated_003_walking_ring_03_r"},
		// {"rPinky1",  "rp_nathan_animated_003_walking_pinky_01_r"},
		// {"rPinky2",  "rp_nathan_animated_003_walking_pinky_02_r"},
		// {"rPinky3",  "rp_nathan_animated_003_walking_pinky_03_r"},

		// // --- Left Fingers (왼손가락 - 3마디 완성) ---
		// {"lThumb1",  "rp_nathan_animated_003_walking_thumb_01_l"},
		// {"lThumb2",  "rp_nathan_animated_003_walking_thumb_02_l"},
		// {"lThumb3",  "rp_nathan_animated_003_walking_thumb_03_l"},
		// {"lIndex1",  "rp_nathan_animated_003_walking_index_01_l"},
		// {"lIndex2",  "rp_nathan_animated_003_walking_index_02_l"},
		// {"lIndex3",  "rp_nathan_animated_003_walking_index_03_l"},
		// {"lMid1",    "rp_nathan_animated_003_walking_middle_01_l"},
		// {"lMid2",    "rp_nathan_animated_003_walking_middle_02_l"},
		// {"lMid3",    "rp_nathan_animated_003_walking_middle_03_l"},
		// {"lRing1",   "rp_nathan_animated_003_walking_ring_01_l"},
		// {"lRing2",   "rp_nathan_animated_003_walking_ring_02_l"},
		// {"lRing3",   "rp_nathan_animated_003_walking_ring_03_l"},
		// {"lPinky1",  "rp_nathan_animated_003_walking_pinky_01_l"},
		// {"lPinky2",  "rp_nathan_animated_003_walking_pinky_02_l"},
		// {"lPinky3",  "rp_nathan_animated_003_walking_pinky_03_l"}
	};

}

CharacterClass::CharacterClass(int pos_x, int pos_y,
	class InputClass* input, class SoundClass* sound,
	vector<unique_ptr<class IGameObject> >& skill_objs)
	: RigidbodyClass(
		Point2d(pos_x, pos_y),
		rect_t{ -50000, 0, 50000, 400000 }, kLeftForword
	), jump_cnt(0), score_(0), input(input), sound(sound), skill_objs(skill_objs)
{

	motions_.loadFromXML("data/resources.xml", "Motion",
		[this](xml_node_wrapper node) -> unique_ptr<AnimatedObjectClass>
		{
			std::string src = node.get_required_attr("src");

			std::string scale_str = node.get_attr("scale", "");
			float scale_x = stof(node.get_attr("scale_x", "1.0"));
			float scale_y = stof(node.get_attr("scale_y", "1.0"));
			float scale_z = stof(node.get_attr("scale_z", "1.0"));
			if (!scale_str.empty())
			{
				scale_x = scale_y = scale_z = stof(scale_str);
			}

			float offset_x = stof(node.get_attr("offset_x", "0"));
			float offset_y = stof(node.get_attr("offset_y", "0"));
			float offset_z = stof(node.get_attr("offset_z", "0"));

			size_t start_frame = stoull(node.get_attr("start", "0"));

			return std::make_unique<AnimatedObjectClass>(src.c_str(),
				start_frame,
				XMFLOAT3(scale_x, scale_y, scale_z),
				XMFLOAT3(offset_x, offset_y, offset_z));
		});

	SetState(CharacterState::kNormal, 0);

	skill_[0] = { 1, 1 };
	skill_[1] = { 2, 1 };
	skill_[2] = { 3, 1 };
	skill_[3] = { 4, 10 };

	time_invincible_end_ = 0;

	skill_bonus_ = SkillBonus::BONUS_NONE;
	time_skill_bonus_get_ = 0;

	guardians_[0] = make_unique<SkillObjectGuardian>();
	guardians_[1] = make_unique<SkillObjectGuardian>();
}

void CharacterClass::FrameMove(time_t curr_time, time_t time_delta, const FieldClass* ground)
{
	bool is_walk = input->IsKeyPressed(DIK_LEFT) ^ input->IsKeyPressed(DIK_RIGHT);
	bool state_controllable = IsStateNotIn({
		CharacterState::kSpell, CharacterState::kHit, CharacterState::kSlip, CharacterState::kDie
		});
	bool is_on_ground = (jump_cnt == 0);
	bool is_run = IsStateIn({
		CharacterState::kRun, CharacterState::kRunJump
		});


	if (state_controllable)
	{
		// Move Attempt
		// If both left and right keys are pressed, the character will not move.
		if (input->IsKeyPressed(DIK_LEFT))  direction_ = kLeftForword;
		if (input->IsKeyPressed(DIK_RIGHT)) direction_ = kRightForward;
		
		// Skill Attempt
		if (input->IsKeyDown(DIK_Z)) UseSkill(curr_time, skill_objs, sound);

		// Down Attempt
		if (input->IsKeyDown(DIK_DOWN) && state_ != CharacterState::kJump && is_on_ground)
		{
			velocity_.y = -1.0, jump_cnt = 1;
			position_.y = max(position_.y - 30, kGroundY); // To prevent the character from felling down below the ground
		}

		// Jump Attempt
		if(input->IsKeyDown(DIK_UP) && jump_cnt <= 1)
		{
			velocity_.y = 2'800, jump_cnt++;
			if (is_run) SetState(CharacterState::kRunJump, curr_time);
			else		SetState(CharacterState::kJump, curr_time);
		}
	}
	
	// gravity
	const int start_y = position_.y;
	const int target_y = position_.y + time_delta * (velocity_.y - (kGravity / 2) * time_delta);
	velocity_.y -= kGravity * (int)time_delta;

	if (ground->IsCollided(range_.x1 + position_.x, range_.x2 + position_.x,
			start_y, target_y, &position_.y))
	{
		velocity_.y = jump_cnt = 0;
	}
	else
	{
		if (jump_cnt == 0) jump_cnt = 1;
		if (state_ == CharacterState::kRun) SetState(CharacterState::kRunJump, curr_time);
		if (state_ == CharacterState::kWalk) SetState(CharacterState::kJump, curr_time);
	}



	switch (state_)
	{
	case CharacterState::kJump:
		
		velocity_.x = (is_walk) ? DIR_WEIGHT(direction_, kWalkSpd) : 0;
		if (jump_cnt == 0)
		{
			if (is_walk) SetState(CharacterState::kWalk, curr_time);
			else SetState(CharacterState::kNormal, curr_time);
		}
		break;

	case CharacterState::kRunJump:
		velocity_.x = (is_walk) ? DIR_WEIGHT(direction_, kRunSpd) : 0;

		if (jump_cnt == 0)
		{
			if (is_walk) SetState(CharacterState::kRun, curr_time);
			else state_ = CharacterState::kJump;
		}
		break;

	case CharacterState::kNormal:
		if (is_walk)
		{
			SetState(CharacterState::kWalk, curr_time);
			velocity_.x = DIR_WEIGHT(direction_, kWalkSpd);
		}
		else velocity_.x = 0;
		break;

	case CharacterState::kWalk:
		if (!is_walk) SetState(CharacterState::kStop, curr_time);
		else
		{
			velocity_.x = DIR_WEIGHT(direction_, kWalkSpd);
		}
		break;

	case CharacterState::kRun:
		if (!is_walk) SetState(CharacterState::kStop, curr_time);
		else
		{
			velocity_.x = DIR_WEIGHT(direction_, kRunSpd);;
		}
		break;

	case CharacterState::kStop:

		velocity_.x = 0;
		if (is_walk)
		{
			if (GetStateTime(curr_time) >= 150) SetState(CharacterState::kWalk, curr_time);
			else SetState(CharacterState::kRun, curr_time);
		}
		break;

	case CharacterState::kSpell:
	{
		velocity_.x = 0;
		if (skill_bonus_ == SkillBonus::BONUS_FOUR_CARDS)
		{
			// if the character BONUS_FOUR_CARDS bounus, use all skills he has.
			int used[5] = { 0 };
			for (int i = 0; i <= 3 && skill_[i].skill_type; i++)
			{
				if (used[skill_[i].skill_type]) continue;
				used[skill_[i].skill_type] = 1;

				skill_currently_used_ = skill_[i];
				OnSkill(curr_time, time_delta, skill_objs);
			}
		}
		else OnSkill(curr_time, time_delta, skill_objs);

		break;
	}

	case CharacterState::kHit:
		position_.x += (int)time_delta * velocity_.x;
		SetStateIfTimeOver(CharacterState::kSlip, state_start_time_, 500);
		break;

	case CharacterState::kSlip:
		SetStateIfTimeOver(CharacterState::kNormal, curr_time, 1'000);
		break;

	case CharacterState::kDie:
		break;
	}

	position_.x = std::clamp(position_.x + static_cast<int>(time_delta) * velocity_.x, kFieldLeftX, kFieldRightX);


	if (skill_bonus_ == SkillBonus::BONUS_ONE_PAIR || skill_bonus_ == SkillBonus::BONUS_TWO_PAIR)
	{
		constexpr float radius = 500'000.0f;
		const int offset_x = static_cast<int>(radius * cos(curr_time * 0.003f));
		const int offset_y = static_cast<int>(radius * sin(curr_time * 0.003f));

		guardians_[0]->SetPosition(position_.x + offset_x, position_.y + 200000 + offset_y);
		if (skill_bonus_ == SkillBonus::BONUS_TWO_PAIR)
		{
			guardians_[1]->SetPosition(position_.x - offset_x, position_.y + 200000 - offset_y);
		}
	}
}

bool CharacterClass::Frame(time_t time_delta, time_t curr_time)
{
	if (time_combo_end_ < curr_time)
	{
		combo_ = 0;
	}


	if (input->IsKeyDown(DIK_X)) // Skill Drop
	{
		for (int i = 3; i >= 0; i--)
		{
			if (skill_[i].skill_type)
			{
				skill_[i].skill_type = 0;
				break;
			}
		}
	}

	return true;
}

void CharacterClass::Draw(time_t curr_time, time_t time_delta, ShaderManager* shader_manager,
	class GraphicResources* graphic_resources) const
{
	ID3D11ShaderResourceView* char_texture = graphic_resources->models_.get("cube")->GetDiffuseTexture();
	if (curr_time <= GetTimeInvincibleEnd()) char_texture = graphic_resources->textures_.get("rainbow")->GetTexture();

	auto char_model_matrices = GetShapeMatrices(curr_time);
	for (auto& [name, box] : char_model_matrices)
	{
		shader_manager->light_shader_->PushRenderQueue(graphic_resources->models_.get("cube"),
			box * GetLocalWorldMatrix(), char_texture);
	}

	/// 하하하 이제부터 리깅 된 모델로 그릴거야 하하하
	auto nam2 = [&](CharacterState state, float state_elapsed_seconds) -> AnimatedObjectClass::FrameShape
		{
			XMMATRIX root_transform = XMMatrixRotationY(DIR_WEIGHT(direction_, XM_PI * 0.65f));
			switch (state)
			{
			case CharacterState::kNormal:
			case CharacterState::kStop:
				root_transform *= XMMatrixRotationY(-XM_PI * 0.5f);
				return motions_.get("idle_new")->GetJointMatrix(
					state_elapsed_seconds / 0.01333333 );

			case CharacterState::kWalk:
				return motions_.get("walk_legacy")->GetJointMatrix(
					state_elapsed_seconds / 0.00333333 );

			case CharacterState::kRun:
				return motions_.get("run_legacy")->GetJointMatrix(
					state_elapsed_seconds / 0.00333333 );
			case CharacterState::kJump:
			case CharacterState::kRunJump:
				if (GetStateTime(curr_time) > 90)
					return motions_.get("jump_new")->GetJointMatrix(
						20 + state_elapsed_seconds / 0.00333333 );
				else
					return motions_.get("jump_new")->GetJointMatrix(
						43 + state_elapsed_seconds / 0.00833333 );
				break;

			case CharacterState::kSpell:
				if (skill_currently_used_.skill_type == 1)
				{
					root_transform *= XMMatrixRotationY(-XM_PI);
					return motions_.get("stumble_new")->GetJointMatrix(
						50 + state_elapsed_seconds / 0.00333333 );
				}
				else
				{
					root_transform *= XMMatrixRotationY(XM_PI * 0.5f);
					return motions_.get("punch_new")->GetJointMatrix(
						state_elapsed_seconds / 0.00133333 );
				}
				break;

			case CharacterState::kHit:
			case CharacterState::kSlip:
				return motions_.get("fall_new")->GetJointMatrix(
					50 + state_elapsed_seconds / 0.00433333 );

			case CharacterState::kDie:
				return motions_.get("fall_new")->GetJointMatrix(
					min(394., 50 + state_elapsed_seconds / 0.01433333 ));

			}
		};


	auto current_shape2 = nam2(state_, (float)GetStateTime(curr_time) / 1000.0f);
	AnimatedObjectClass::FrameShape current_shape3;
	for (auto& [name, matrix] : current_shape2)
	{
		std::string converted_name = name;

		if (kBvhToFbx.find(name) != kBvhToFbx.end())
			converted_name = kBvhToFbx[name];

		current_shape3[converted_name] = matrix;
	}
	graphic_resources->fbx_models_.get("character")->Update(current_shape3);

	if (state_ == CharacterState::kRun)
	{
		if (prev_state_ != CharacterState::kRun && curr_time - time_delta < state_start_time_)
		{
			// If state just changed to run, reset the particle system to make sure the particles are emitted from the start.
			graphic_resources->particle_system_.get("dust-spread")->Clear();
		}

		shader_manager->particle_shader_->PushRenderQueue(
			graphic_resources->particle_system_.get("dust-spread"),
			XMMatrixRotationZ(
				direction_ == direction_t::kLeftForword ? 0 : XM_PI) * GetLocalWorldMatrix()
		);
	}



	double scaling = 0.05;
	/*
	shader_manager->character_shader_->PushRenderQueue(
		fbx_graphic_resources->models.get("character"),
		DirectX::XMMatrixRotationY(M_PI_2) *
		DirectX::XMMatrixScaling(scaling, scaling, scaling)
		* DirectX::XMMatrixTranslation(-2, 0, 0)
		* GetLocalWorldMatrix()
	);*/

	XMMATRIX skill_stone_pos = GetSkillStonePos(curr_time);

	int sb_count = 0;
	time_t elapsed_time_for_sb = curr_time - GetSkill(3).learned_time;
	for (int i = 0; i < 4; i++)
	{
		const int type = GetSkill(i).skill_type;
		const int power = GetSkill(i).skill_power;
		if (type == 0) break;

		const float scale = (power + 13) * 0.04f;
		float brightness = 0.0f;

		if (GetSkill(i).is_part_of_skillbonus)
		{
			const time_t local_time = (elapsed_time_for_sb + 5'000 - 400 * ++sb_count) % 5'000;
			if (local_time <= 800.0f) brightness = max(0.0f, sin(local_time / 200.0f));

			const time_t global_time = (elapsed_time_for_sb + 5'000 - 2'000) % 5'000;
			if (global_time <= 1200.0f) brightness = max(0.0f, sin(global_time / 300.0f));
		}

		constexpr XMFLOAT4 kSkillColor[5] =
		{
			{0, 0, 0, 1.0f},
			{0.9f, 0.1f, 0.3f, 1.0f},
			{0.2f, 0.8f, 0.1f, 1.0f},
			{0.1f, 0.3f, 0.9f, 1.0f},
			{0.2f, 0.1f, 0.1f, 1.0f}
		};

		XMFLOAT4 skill_color = kSkillColor[type];
		if (brightness > 0.0f)
		{
			skill_color.x += (1 - skill_color.x) * brightness * 0.6;
			skill_color.y += (1 - skill_color.y) * brightness * 0.6;
			skill_color.z += (1 - skill_color.z) * brightness * 0.6;
			skill_color.w += (1 - skill_color.w) * brightness * 0.6;
		}

		shader_manager->stone_shader_->PushRenderQueue(graphic_resources->models_.get("diamond"),
			XMMatrixScaling(scale, scale, scale) * skill_stone_pos * XMMatrixTranslation(0, -0.6f * i, 0),
			skill_color);
	}


	// Draw Gaurdian bead
	for (int i = 0; GetGuardian(i) != nullptr; i++)
	{
		GetGuardian(i)->Draw(curr_time, time_delta, shader_manager, graphic_resources);
	}
}

std::unordered_map<std::string, XMMATRIX> CharacterClass::GetShapeMatrices(time_t curr_time) const
{
	// TODO: Fix It. (prev_state_'s state time is not considered.)
	auto lerp_shape = AnimatedObjectClass::MergeFrameShapes(
		GetShapeMatricesAtState(prev_state_, (float)GetStateTime(curr_time) / 1000.0f),
		GetShapeMatricesAtState(state_,      (float)GetStateTime(curr_time) / 1000.0f),
		min(1.0f, (float)GetStateTime(curr_time) / 100.0f));

	return lerp_shape;
}

std::unordered_map<std::string, XMMATRIX> CharacterClass::GetShapeMatricesAtState
	(CharacterState state, float state_elapsed_seconds) const
{
	XMMATRIX root_transform = XMMatrixRotationY(DIR_WEIGHT(direction_, XM_PI * 0.65f));
	switch (state)
	{
	case CharacterState::kNormal:
	case CharacterState::kStop:
		root_transform *= XMMatrixRotationY(-XM_PI * 0.5f);
		return motions_.get("idle_new")->UpdateAndGetShapeMatrix(
			state_elapsed_seconds / 0.01333333, root_transform);

	case CharacterState::kWalk:
		return motions_.get("walk_legacy")->UpdateAndGetShapeMatrix(
			state_elapsed_seconds / 0.00333333, root_transform);

	case CharacterState::kRun:
		return motions_.get("run_legacy")->UpdateAndGetShapeMatrix(
			state_elapsed_seconds / 0.00333333, root_transform);

	case CharacterState::kJump:
	case CharacterState::kRunJump:
		if (state_elapsed_seconds > 0.0090)
			return motions_.get("jump_new")->UpdateAndGetShapeMatrix(
				20 + state_elapsed_seconds / 0.00333333, root_transform);
		else
			return motions_.get("jump_new")->UpdateAndGetShapeMatrix(
				43 + state_elapsed_seconds / 0.00833333, root_transform);
		break;

	case CharacterState::kSpell:
		if (skill_currently_used_.skill_type == 1)
		{
			root_transform *= XMMatrixRotationY(-XM_PI);
			return motions_.get("stumble_new")->UpdateAndGetShapeMatrix(
				50 + state_elapsed_seconds / 0.00333333 , root_transform);
		}
		else
		{
			root_transform *= XMMatrixRotationY(XM_PI * 0.5f);
			return motions_.get("punch_new")->UpdateAndGetShapeMatrix(
				state_elapsed_seconds / 0.00133333 , root_transform);
		}
		break;

	case CharacterState::kHit:
	case CharacterState::kSlip:
		return motions_.get("fall_new")->UpdateAndGetShapeMatrix(
			50 + state_elapsed_seconds / 0.00433333 , root_transform);

	case CharacterState::kDie:
		return motions_.get("fall_new")->UpdateAndGetShapeMatrix(
			min(394., 50 + state_elapsed_seconds / 0.01433333 ), root_transform);
	}
}

std::pair<std::string, size_t> CharacterClass::GetMotionNameAndFrame(CharacterState state, float state_elapsed_seconds) const
{
	switch (state_)
	{
	case CharacterState::kNormal:
	case CharacterState::kStop:
		return { "idle_new",	(size_t)(state_elapsed_seconds / 0.01333333) };

	case CharacterState::kWalk:
		return { "walk_legacy",	(size_t)(state_elapsed_seconds / 0.00333333) };

	case CharacterState::kRun:
		return { "run_legacy",	(size_t)(state_elapsed_seconds / 0.00333333) };

	case CharacterState::kJump:
	case CharacterState::kRunJump:
		if (state_elapsed_seconds > 0.0090)
			return { "jump_new", (size_t)(20 + state_elapsed_seconds / 0.00333333) };
		else
			return { "jump_new", (size_t)(43 + state_elapsed_seconds / 0.00833333) };

	default:
		// NOt used for now.
		return { "", 0 };
	}
	/*case CharacterState::kSpell:
		if (skill_currently_used_.skill_type == 1)
		{
			root_transform *= XMMatrixRotationY(-XM_PI);
			return motions_.get("stumble_new")->UpdateAndGetShapeMatrix(
				50 + state_elapsed_seconds / 0.00333333, root_transform);
		}
		else
		{
			root_transform *= XMMatrixRotationY(XM_PI * 0.5f);
			return motions_.get("punch_new")->UpdateAndGetShapeMatrix(
				state_elapsed_seconds / 0.00133333, root_transform);
		}
		break;

	case CharacterState::kHit:
	case CharacterState::kSlip:
		return motions_.get("fall_new")->UpdateAndGetShapeMatrix(
			50 + state_elapsed_seconds / 0.00433333, root_transform);

	case CharacterState::kDie:
		return motions_.get("fall_new")->UpdateAndGetShapeMatrix(
			min(394., 50 + state_elapsed_seconds / 0.01433333), root_transform);
	};*/
}

bool CharacterClass::OnCollided(time_t curr_time, int vx)
{
	if (time_invincible_end_ < curr_time && time_skill_ended_ < curr_time)
	{
		combo_ = 0;

		SetState(CharacterState::kHit, curr_time);
		direction_ = (vx > 0) ? kLeftForword : kRightForward;

		// lost skill
		if (skill_[0].skill_type == 0)
		{
			SetState(CharacterState::kDie, curr_time);
			velocity_.x = vx / 2;

			velocity_.y = 1500;
			time_invincible_end_ = 1LL << 59;
		}
		else
		{
			for (int i = 3; i >= 0; i--)
			{
				if (skill_[i].skill_type)
				{
					skill_[i].skill_type = 0;
					skill_[i].is_part_of_skillbonus = 0;
					break;
				}
			}
			velocity_.x = vx / 3;
			velocity_.y = 1500;
			time_invincible_end_ = state_start_time_ + kInvincibleDuration;
		}
		return true;
	}
	return false;
}

float CharacterClass::GetCooltimeGaugeRatio(time_t curr_time) const
{
	return std::clamp((time_skill_available_ - (long long)curr_time) / (float)kSkillCooltime, -0.3f, 1.0f);
}

float CharacterClass::GetInvincibleGaugeRatio(time_t curr_time) const
{
	return std::clamp((time_invincible_end_ - (long long)curr_time) / (float)kInvincibleDuration, -0.3f, 1.0f);
}

XMMATRIX CharacterClass::GetSkillStonePos(time_t curr_time) const
{
	constexpr float kBoxSize = 0.32f;
	XMMATRIX skill_stone_pos =
		XMMatrixRotationX(XM_PI / 18) * XMMatrixRotationY(curr_time * 0.001f) * XMMatrixRotationX(-XM_PI / 10) *
		XMMatrixScaling(kBoxSize, kBoxSize * 1.2f, kBoxSize) * XMMatrixTranslation(-1.3f, 4.0f, 0.f) *
		GetLocalWorldMatrix();

	return skill_stone_pos;
}



SkillBonus CharacterClass::LearnSkill(
	int skill_id, time_t curr_time)
{
	int skill_power = RandomClass::rand(1, 10);
	score_ += skill_power;

	for (auto& skill : skill_)
	{
		// If an empty skill slot exists,
		if (skill.skill_type == 0)
		{
			// fill the skill with random power.
			skill.skill_type = skill_id;
			skill.skill_power = skill_power;
			skill.learned_time = curr_time;
			skill.is_part_of_skillbonus = false;

			// If all skills is loaded,
			if (skill_[3].skill_type)
			{
				// check bonus and set it.
				time_skill_bonus_get_ = curr_time;
				return skill_bonus_ = CalculateSkillBonus(skill_);
			}
			// Otherwise, don't need to do anything.
			else return SkillBonus::BONUS_NONE;
		}
	}
	return SkillBonus::BONUS_NONE;
}

void CharacterClass::AddCombo(time_t curr_time)
{
	++combo_;
	time_combo_end_ = curr_time + kComboDuration;
}

void CharacterClass::OnSkill(time_t curr_time, time_t time_delta,
	vector<unique_ptr<class IGameObject> >& skill_objs)
{
	const time_t state_time = GetStateTime(curr_time);
	const time_t prev_state_time =
		(state_time >= time_delta) ? state_time - time_delta : 0;

	switch (skill_currently_used_.skill_type)
	{
	case 0:
		if (prev_state_time < 100 && 100 <= state_time)
		{
			if (skill_bonus_ == SkillBonus::BONUS_NO_PAIR)
			{
				skill_objs.emplace_back(new SkillObjectBasic(position_.x + DIR_WEIGHT(direction_, 185000), position_.y,
					DIR_WEIGHT(direction_, 100), 1, state_start_time_ + 100));
			}
			else
			{
				skill_objs.emplace_back(new SkillObjectBasic(position_.x + DIR_WEIGHT(direction_, 185000), position_.y,
					DIR_WEIGHT(direction_, 100), 0, state_start_time_ + 100));
			}

		}
		SetStateIfTimeOver(CharacterState::kNormal, curr_time, 300);
		break;

	case 1:
		if (prev_state_time < 200 && 200 <= state_time)
		{
			constexpr int object_vx[9] = { 6000, 6000, 4000, 2000, 0, -2000, -4000, -6000, -6000 };
			constexpr int object_vy[9] = { 0, -2000, -3000, -4000, -4000, -4000, -3000, -2000, 0 };


			if (skill_bonus_ == SkillBonus::BONUS_FLUSH || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
			{
				for (int i = 0; i < 9; i++)
				{
					skill_objs.emplace_back(
						new SkillObjectSpear(position_.x, position_.y,
							object_vx[i], object_vy[i],
							skill_currently_used_.skill_power, state_start_time_ + 200));
				}
			}
			else
			{
				for (int i = 1; i < 8; i++)
				{
					skill_objs.emplace_back(
						new SkillObjectSpear(position_.x, position_.y,
							object_vx[i], object_vy[i],
							skill_currently_used_.skill_power, state_start_time_ + 200));
				}
			}


		}
		SetStateIfTimeOver(CharacterState::kNormal, curr_time, 800);
		break;

	case 2:
	{
		constexpr int kBeadSpeed = 1'200;
		constexpr time_t kFirstBeadTime = 40;
		constexpr time_t kLastBeadTime = 300;

		int bead_cnt = (skill_bonus_ == SkillBonus::BONUS_FLUSH
			|| skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH) ? 6 : 4;
		time_t bead_interval = (kLastBeadTime - kFirstBeadTime) / (bead_cnt - 1);

		if (kFirstBeadTime < state_time)
		{
			time_t target_bead = (state_time - kFirstBeadTime) / bead_interval + 1;
			time_t prev_bead = (prev_state_time < kFirstBeadTime) ? 0 : ((prev_state_time - kFirstBeadTime) / bead_interval + 1);

			for (time_t i = prev_bead + 1; i <= target_bead; i++)
			{
				double angle = M_PI_4 / 6 * (i - 1);

				double cos_angle = cos(angle), sin_angle = sin(angle);

				const int position_x = position_.x + static_cast<int>(DIR_WEIGHT(direction_, cos_angle * 100000 + 200000));
				const int position_y = position_.y + static_cast<int>(sin_angle * 100000) + 300000;

				const int velocity_x = static_cast<int>(DIR_WEIGHT(direction_, kBeadSpeed * cos_angle));
				const int velocity_y = static_cast<int>(kBeadSpeed * sin_angle);

				skill_objs.emplace_back(new SkillObjectBead(
					position_x, position_y,
					velocity_x, velocity_y, skill_currently_used_.skill_power,
					state_start_time_ + i * 40));
			}
		}
		SetStateIfTimeOver(CharacterState::kNormal, curr_time, 300);
		break;
	}

	case 3:
		if (prev_state_time < 50 && 50 <= state_time)
		{
			if (skill_bonus_ == SkillBonus::BONUS_FLUSH || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
			{

				skill_objs.emplace_back(new SkillObjectLeg(
					position_.x - 300'000,
					skill_currently_used_.skill_power, state_start_time_ + 50));
				skill_objs.emplace_back(new SkillObjectLeg(
					position_.x + 300'000,
					skill_currently_used_.skill_power, state_start_time_ + 50));
			}
			else
			{
				skill_objs.emplace_back(new SkillObjectLeg(
					position_.x + DIR_WEIGHT(direction_, 300'000),
					skill_currently_used_.skill_power, state_start_time_ + 50));
			}
		}
		SetStateIfTimeOver(CharacterState::kNormal, curr_time, 300);
		break;

	case 4:
		SetStateIfTimeOver(CharacterState::kNormal, curr_time, 300);
		break;
	}
}



bool CharacterClass::UseSkill(time_t curr_time,
	vector<unique_ptr<class IGameObject> >& skill_objs,
	SoundClass* sound)
{
	if (curr_time < time_skill_available_)
	{
		// TODO: message print
		return false;
	}

	SetState(CharacterState::kSpell, curr_time);

	skill_currently_used_ = { 0, 0 };
	for (int i = 3; i >= 0; i--)
	{
		if (skill_[i].skill_type || i == 0) skill_currently_used_ = skill_[i];
		else continue;

		switch (skill_currently_used_.skill_type)
		{
		case 0:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect("spell3");
			break;

		case 1:
			velocity_.y = 3'600;

			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect("spell1");
			break;

		case 2:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect("spell2");
			break;

		case 3:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect("spell2");
			break;

		case 4:
			time_skill_ended_ = state_start_time_ + 300;
			sound->PlayEffect("spell2");


			skill_objs.emplace_back(new SkillObjectShield(
				position_.x, position_.y + 200000, -1600, 0,
				skill_currently_used_.skill_power, state_start_time_));
			skill_objs.emplace_back(new SkillObjectShield(
				position_.x, position_.y + 200000, 1600, 0,
				skill_currently_used_.skill_power, state_start_time_));
			skill_objs.emplace_back(new SkillObjectShield(
				position_.x, position_.y + 200000, 0, 1600,
				skill_currently_used_.skill_power, state_start_time_));

			if (skill_bonus_ == SkillBonus::BONUS_FLUSH || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
			{
				skill_objs.emplace_back(new SkillObjectShield(
					position_.x, position_.y + 200000, 0, -1600,
					skill_currently_used_.skill_power, state_start_time_));
			}

			break;
		}

		if (skill_bonus_ != SkillBonus::BONUS_FOUR_CARDS)
		{
			break;
		}
	}


	if (skill_bonus_ == SkillBonus::BONUS_TRIPLE)
		skill_currently_used_.skill_power += 3;




	if (skill_bonus_ == SkillBonus::BONUS_STRAIGHT || skill_bonus_ == SkillBonus::BONUS_STRAIGHT_FLUSH)
	{
		time_skill_available_ = time_skill_ended_ + kSkillCooltime / 2;
	}
	else time_skill_available_ = time_skill_ended_ + kSkillCooltime;

	return true;
}