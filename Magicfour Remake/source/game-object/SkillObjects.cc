#include "game-object/SkillObjects.hh"

#include "core/global.hh"
#include "game-object/MonsterClass.hh"
#include "graphics/ModelClass.hh"
#include "map/GroundClass.hh"

#include "shader/ShaderManager.hh"
#include "shader/NormalMapShaderClass.hh"
#include "shader/FireShaderClass.hh"

using namespace std;
using namespace DirectX;

std::string SkillObjectSpear::model_name_;
std::string SkillObjectBead::model_name_;
std::string SkillObjectBead::effect_model_name_;
std::string SkillObjectLeg::model_name_;
std::string SkillObjectBasic::model_name_;
std::string SkillObjectShield::model_name_;
std::string SkillObjectGuardian::model_name_;

SkillObjectSpear::SkillObjectSpear(int pos_x, int pos_y,
	int vx, int vy, int skill_level, time_t created_time)
	: SkillObjectClass(pos_x, pos_y, rect_t{ -30000, -30000, 30000, 30000 },
		vx, vy, skill_level, created_time)
{
	angle_ = (float)atan(vx / (double)vy);
}

void SkillObjectSpear::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	switch (state_)
	{
	case SkillObjectState::kEmbryo:

		if (curr_time < created_time_) break;

		// If created time is passed, set state as normal and move this instance.
		SetState(SkillObjectState::kNormal, created_time_);
		time_delta = curr_time - created_time_;
			
	case SkillObjectState::kNormal:
	{
		// Movement acoording to the current velocity.
		int start_y = position_.y;
		position_ = GetPositionAfterMove(time_delta);
		int target_y = position_.y;

		for (auto& ground_obj : ground)
		{
			position_.y = max(position_.y,
				ground_obj.IsCollided(range_.x1 + position_.x, range_.x2 + position_.x, start_y, target_y));
		}

		if (position_.y != target_y)
		{
			state_ = SkillObjectState::kSpearOnGround;
			state_start_time_ = curr_time;
		}

	}
		break;
	}
}

bool SkillObjectSpear::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;

	if (state_ == SkillObjectState::kNormal)
	{
		const int damage_amount = 20 + skill_level_ * 2;
		monster->Damage(damage_amount, collided_time, velocity_.x / 6, 0);
	}
	else if (state_ == SkillObjectState::kSpearOnGround)
	{
		const int damage_amount = 10 + skill_level_;
		monster->Damage(damage_amount, collided_time, velocity_.x / 6, 1000);
	}

	state_ = SkillObjectState::kDie;
	return true;
}

bool SkillObjectSpear::Frame(time_t curr_time, time_t time_delta)
{
	switch (state_)
	{
	case SkillObjectState::kNormal:
		if (position_.x <= kSpawnLeftX - 300000 || position_.x >= kSpawnRightX + 300000)
		{
			state_ = SkillObjectState::kDie;
			return false;
		}

	case SkillObjectState::kSpearOnGround:
		if (state_start_time_ + 1000 < curr_time)
		{
			state_ = SkillObjectState::kDie;
			return false;
		}
		break;

	case SkillObjectState::kDie:
		return false;
	}

	return true;
}

void SkillObjectSpear::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const
{
	const XMMATRIX shape = XMMatrixRotationY(XM_PI / 2) * XMMatrixRotationZ(XM_PI - angle_)
		* XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	shader_manager->normalMap_shader_->PushRenderQueue(models.get("spear"), shape);
}

XMMATRIX SkillObjectSpear::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixRotationY(XM_PI / 2) * XMMatrixRotationZ(XM_PI - angle_)
		* XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}

void SkillObjectSpear::initialize(const std::string& model_name)
{
	model_name_ = model_name;
}

SkillObjectBead::SkillObjectBead(int pos_x, int pos_y,
	int vx, int vy, int skill_level, time_t created_time)
	: SkillObjectClass(pos_x, pos_y,
		rect_t{ -30000, -30000, 30000, 30000 }, vx, vy, skill_level, created_time)
{

}

void SkillObjectBead::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	switch (state_)
	{
	case SkillObjectState::kEmbryo:

		if (curr_time < created_time_) break;

		// If created time is passed, set state as normal and move this instance.
		SetState(SkillObjectState::kNormal, created_time_);
		time_delta = curr_time - created_time_;

	case SkillObjectState::kNormal:
	case SkillObjectState::kBeadOneHit:

		position_ = GetPositionAfterMove(time_delta);
	}

}

bool SkillObjectBead::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;

	const int damage_amount = 15 + skill_level_;
	switch (state_)
	{
	case SkillObjectState::kNormal:
		monster->Damage(damage_amount, collided_time, velocity_.x / 8, 0);
		velocity_.x *= 2, velocity_.y *= 2;

		SetState(SkillObjectState::kBeadOneHit, collided_time);
		break;
		
	case SkillObjectState::kBeadOneHit:
		monster->Damage(damage_amount, collided_time, velocity_.x / 8, 0);
		SetState(SkillObjectState::kDie, collided_time);
	}
	return true;

}

bool SkillObjectBead::Frame(time_t curr_time, time_t time_delta)
{
	switch (state_)
	{
	case SkillObjectState::kNormal:
	case SkillObjectState::kBeadOneHit:
		if (GetStateTime(curr_time) >= 1'000)
		{
			state_ = SkillObjectState::kDie;
			return false;
		}
		break;

	case SkillObjectState::kDie:
		return false;
	}
	return true;
}

void SkillObjectBead::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const
{
	const XMMATRIX orb_shape = XMMatrixScaling(0.45f, 0.45f, 0.45f) * XMMatrixRotationY(curr_time * 0.0002f * XM_PI)
		;//* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);

	const XMMATRIX fire_shape =
		XMMatrixTranslation(0, 1.0f, 0)
		* XMMatrixScaling(0.5f, 1.3f * (velocity_.length() / 1'200), 1.0f)
		* XMMatrixRotationZ(XM_PI / 2 + atan2(velocity_.y, velocity_.x))
		* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, (velocity_.y / 1'200'000.0f));
	
	//shader_manager->normalMap_shader_->PushRenderQueue(models.get("orb"), orb_shape);
	shader_manager->fire_shader_->PushRenderQueue(models.get("fire"),
		fire_shape,
		{ 1.3f, 2.1f, 2.3f },
		{ 1.0f, 2.0f, 3.0f },
		{ 0.1f, 0.2f },
		{ 0.1f, 0.3f },
		{ 0.1f, 0.1f },
		0.8f, 0.0f);
}

XMMATRIX SkillObjectBead::GetGlobalShapeTransform(time_t curr_time)
{
	return 
		XMMatrixScaling(0.45f, 0.45f, 0.45f) * XMMatrixRotationY(curr_time * 0.0002f * XM_PI)
		* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}

void SkillObjectBead::initialize(const std::string& model_name, const std::string& effect_model_name)
{
	model_name_ = model_name;
	effect_model_name_ = effect_model_name;
}

std::string SkillObjectBead::GetEffectModel()
{
	return effect_model_name_;
}

SkillObjectLeg::SkillObjectLeg(int pos_x, int skill_level, time_t created_time)
	: SkillObjectClass(pos_x, kGroundY, rect_t{ -50000, -1200000, 50000, 0 },
		0, 0, skill_level, created_time)
{
}

void SkillObjectLeg::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	switch (state_)
	{
	case SkillObjectState::kEmbryo:
		if (curr_time < created_time_) break;

		// If created time is passed, set state as normal and move this instance.
		SetState(SkillObjectState::kNormal, created_time_);
		time_delta = curr_time - created_time_;

	case SkillObjectState::kNormal:
		position_.y += 2'000 * time_delta;
		break;
	}
}

bool SkillObjectLeg::OnCollided(MonsterClass* monster, time_t collided_time)
{
	rect_t upper = GetGlobalRange();
	upper.y1 = upper.y2 - 6000;
	
	if (SkillObjectClass::OnCollided(monster, collided_time))
	{
		if (upper.collide(monster->GetGlobalRange()))
		{
			const int damage_amount = skill_level_ * 2 + 27;
			monster->Damage(damage_amount, collided_time, 0, 3000);
			// TODO: stop
		}
		else
		{
			const int damage_amount = skill_level_ * 2 + 5;
			monster->Damage(25, collided_time, 0, 500);
			// TODO; direction
		}
		return true;
	}
	return false;
}

bool SkillObjectLeg::Frame(time_t curr_time, time_t time_delta)
{
	return state_start_time_ + 1200 > curr_time;
}

void SkillObjectLeg::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const
{
	const XMMATRIX shape = XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	shader_manager->normalMap_shader_->PushRenderQueue(models.get("leg"), shape);
}

XMMATRIX SkillObjectLeg::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}


void SkillObjectLeg::initialize(const std::string& model_name)
{
	model_name_ = model_name;
}


SkillObjectBasic::SkillObjectBasic(int pos_x, int pos_y, int vx,
	int skill_level, time_t created_time)
	: SkillObjectClass(pos_x, pos_y,
		rect_t{ -60000, 80000, 60000, 380000 }, vx, 0, skill_level, created_time)
{
}

void SkillObjectBasic::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	switch (state_)
	{
	case SkillObjectState::kEmbryo:
		if (curr_time < created_time_) break;

		// If created time is passed, set state as normal and move this instance.
		SetState(SkillObjectState::kNormal, created_time_);
		time_delta = curr_time - created_time_;

	case SkillObjectState::kNormal:
		position_ = GetPositionAfterMove(time_delta);
		break;
	}
}

bool SkillObjectBasic::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;
	monster->Damage((skill_level_) ? 70 : 35, collided_time, velocity_.x, 0);
	return true;
}

bool SkillObjectBasic::Frame(time_t curr_time, time_t time_delta)
{
	constexpr time_t lifetime = 200;
	return created_time_ + lifetime > curr_time;
}

void SkillObjectBasic::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const
{
	const XMMATRIX shape = XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	shader_manager->normalMap_shader_->PushRenderQueue(models.get("basic"), shape);
}

XMMATRIX SkillObjectBasic::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}

void SkillObjectBasic::initialize(const std::string& model_name)
{
	model_name_ = model_name;
}

SkillObjectShield::SkillObjectShield(int pos_x, int pos_y,
	int vx, int vy, int skill_level, time_t created_time)
	: SkillObjectClass(pos_x, pos_y,
		(vx != 0) ? rect_t{-40000, -200000, 40000, 200000} :
		rect_t{ -200000, -40000, 200000, 40000 }, vx, vy, skill_level, created_time)
{

}

void SkillObjectShield::FrameMove(time_t curr_time,
	time_t time_delta, const vector<class GroundClass>& ground)
{
	switch (state_)
	{
	case SkillObjectState::kEmbryo:
		if (curr_time < created_time_) break;

		// If created time is passed, set state as normal and move this instance.
		SetState(SkillObjectState::kNormal, created_time_);
		time_delta = curr_time - created_time_;

	case SkillObjectState::kNormal:

		position_ = GetPositionAfterMove(time_delta);
		break;
	}
}

bool SkillObjectShield::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;

	const int damage_amount = skill_level_ * 3 + 10;
	monster->Damage(damage_amount, collided_time, velocity_.x / 2, velocity_.y / 2);

	return true;
}

bool SkillObjectShield::Frame(time_t curr_time, time_t time_delta)
{
	constexpr time_t lifetime = 200;
	return state_start_time_ + lifetime > curr_time;
}


void SkillObjectShield::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const
{
	XMMATRIX shape;
	if (velocity_.x > 0)
	{
		shape = XMMatrixRotationZ(-XM_PI / 2) * XMMatrixScaling(0.7f, 0.7f, 0.7f)
			* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	}
	else if (velocity_.x == 0)
	{
		shape = XMMatrixScaling(0.7f, 0.7f, 0.7f) * XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	}
	else //(velocity_.x < 0)
	{
		shape = XMMatrixRotationZ(XM_PI / 2) * XMMatrixScaling(0.7f, 0.7f, 0.7f)
			* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	}

	shader_manager->normalMap_shader_->PushRenderQueue(models.get("shield"), shape);
}

XMMATRIX SkillObjectShield::GetGlobalShapeTransform(time_t curr_time)
{
	if (velocity_.x > 0)
	{
		return XMMatrixRotationZ(-XM_PI / 2) * XMMatrixScaling(0.7f, 0.7f, 0.7f)
			* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	}
	else if (velocity_.x == 0)
	{
		return XMMatrixScaling(0.7f, 0.7f, 0.7f) * XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	}
	else //(velocity_.x < 0)
	{
		return XMMatrixRotationZ(XM_PI / 2) * XMMatrixScaling(0.7f, 0.7f, 0.7f)
			* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	}	
}

void SkillObjectShield::initialize(const std::string& model_name)
{
	model_name_ = model_name;
}

SkillObjectGuardian::SkillObjectGuardian()
	: SkillObjectClass(0, 0, rect_t{ -30000, -30000, 30000, 30000 },
		0, 0, 0, 0)
{
	SetState(SkillObjectState::kNormal, 0);
}

void SkillObjectGuardian::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const
{
	const XMMATRIX shape = XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
	shader_manager->normalMap_shader_->PushRenderQueue(models.get("orb"), shape);
}

bool SkillObjectGuardian::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;

	const int damage_amount = 20;
	monster->DamageWithNoKnockBack(damage_amount, collided_time);
	return true;
}

XMMATRIX SkillObjectGuardian::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}

void SkillObjectGuardian::initialize(const std::string& model_name)
{
	model_name_ = model_name;
}

