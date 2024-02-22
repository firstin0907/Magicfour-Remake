#include "../include/SkillObjects.hh"

#include "../include/global.hh"
#include "../include/MonsterClass.hh"
#include "../include/ModelClass.hh"
#include "../include/GroundClass.hh"

using namespace std;
using namespace DirectX;


unique_ptr<class ModelClass> SkillObjectSpear::model_ = nullptr;
unique_ptr<class ModelClass> SkillObjectBead::model_ = nullptr;
unique_ptr<class ModelClass> SkillObjectLeg::model_ = nullptr;
unique_ptr<class ModelClass> SkillObjectBasic::model_ = nullptr;

SkillObjectSpear::SkillObjectSpear(int pos_x, int pos_y, int vx, int vy, time_t created_time)
	: SkillObjectClass(pos_x, pos_y, rect_t{ -30000, -30000, 30000, 30000 },
		vx, vy)
{
	SetState(SkillObjectState::kNormal, created_time);
	angle_ = (float)atan(vx / (double)vy);
}

void SkillObjectSpear::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	switch (state_)
	{
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

	if (state_ == SkillObjectState::kNormal) monster->Damage(25, collided_time, velocity_.x / 6, 0);
	else if (state_ == SkillObjectState::kSpearOnGround) monster->Damage(17, collided_time, velocity_.x / 6, 1000);

	state_ = SkillObjectState::kDie;
	return true;
}

bool SkillObjectSpear::Frame(time_t curr_time, time_t time_delta)
{
	switch (state_)
	{
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

XMMATRIX SkillObjectSpear::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixRotationY(XM_PI / 2)
		* XMMatrixRotationZ(XM_PI - angle_) * XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}

void SkillObjectSpear::initialize(ModelClass* model)
{
	model_ = unique_ptr<ModelClass>(model);
}

ModelClass* SkillObjectSpear::GetModel()
{
	return model_.get();
}

SkillObjectBead::SkillObjectBead(int pos_x, int pos_y, int vx, int vy, time_t created_time)
	: SkillObjectClass(pos_x, pos_y, rect_t{ -30000, -30000, 30000, 30000 }, vx, vy)
{
	SetState(SkillObjectState::kNormal, created_time);
}

void SkillObjectBead::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	time_t state_time = GetStateTime(curr_time);

	position_ = GetPositionAfterMove(min(state_time, time_delta));
}

bool SkillObjectBead::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;

	switch (state_)
	{
	case SkillObjectState::kNormal:
		monster->Damage(20, collided_time, velocity_.x / 8, 0);
		velocity_.x *= 2, velocity_.y *= 2;

		SetState(SkillObjectState::kBeadOneHit, collided_time);
		break;
		
	case SkillObjectState::kBeadOneHit:
		monster->Damage(20, collided_time, velocity_.x / 8, 0);
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

XMMATRIX SkillObjectBead::GetGlobalShapeTransform(time_t curr_time)
{
	return 
		XMMatrixScaling(0.45f, 0.45f, 0.45f) * XMMatrixRotationY(curr_time * 0.0002f * XM_PI)
		* XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}

void SkillObjectBead::initialize(class ModelClass* model)
{
	model_ = unique_ptr<ModelClass>(model);
}

ModelClass* SkillObjectBead::GetModel()
{
	return model_.get();
}

SkillObjectLeg::SkillObjectLeg(int pos_x, time_t created_time)
	: SkillObjectClass(pos_x, kGroundY, rect_t{ -50000, -1200000, 50000, 0 },
		0, 0)
{
	SetState(SkillObjectState::kNormal, created_time);
}

void SkillObjectLeg::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	time_delta = min(time_delta, curr_time - state_start_time_);
	position_.y += 2'000 * time_delta;
}

bool SkillObjectLeg::OnCollided(MonsterClass* monster, time_t collided_time)
{
	rect_t upper = GetGlobalRange();
	upper.y1 = upper.y2 - 6000;
	
	if (SkillObjectClass::OnCollided(monster, collided_time))
	{
		if (upper.collide(monster->GetGlobalRange()))
		{
			monster->Damage(50, collided_time, 0, 3000);
			// TODO: stop
		}
		// TODO; direction
		else monster->Damage(25, collided_time, 0, 500);
		return true;
	}
	return false;
}

bool SkillObjectLeg::Frame(time_t curr_time, time_t time_delta)
{
	return state_start_time_ + 1200 > curr_time;
}

XMMATRIX SkillObjectLeg::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixTranslation(position_.x * kScope, position_.y * kScope, 0.0f);
}


void SkillObjectLeg::initialize(class ModelClass* model)
{
	model_ = unique_ptr<ModelClass>(model);
}


ModelClass* SkillObjectLeg::GetModel()
{
	return model_.get();
}



SkillObjectBasic::SkillObjectBasic(int pos_x, int pos_y, int vx, time_t created_time)
	: SkillObjectClass(pos_x, pos_y, rect_t{ -60000, 80000, 60000, 380000 }, vx, 0)
{
	SetState(SkillObjectState::kNormal, created_time);
}

void SkillObjectBasic::FrameMove(time_t curr_time, time_t time_delta,
	const vector<class GroundClass>& ground)
{
	
}

bool SkillObjectBasic::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;
	monster->Damage(40, collided_time, velocity_.x, 0);
	return true;
}

bool SkillObjectBasic::Frame(time_t curr_time, time_t time_delta)
{
	constexpr time_t lifetime = 200;
	return state_start_time_ + lifetime > curr_time;
}

XMMATRIX SkillObjectBasic::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixIdentity();
}

void SkillObjectBasic::initialize(ModelClass* model)
{
	model_ = unique_ptr<ModelClass>(model);
}

ModelClass* SkillObjectBasic::GetModel()
{
	return model_.get();
}
