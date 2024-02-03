#include "../include/SkillObjects.hh"

#include "../include/global.hh"
#include "../include/MonsterClass.hh"
#include "../include/ModelClass.hh"
#include "../include/GroundClass.hh"

unique_ptr<class ModelClass> SkillObjectSpear::m_Model = nullptr;
unique_ptr<class ModelClass> SkillObjectBead::m_Model = nullptr;
unique_ptr<class ModelClass> SkillObjectLeg::m_Model = nullptr;
unique_ptr<class ModelClass> SkillObjectBasic::m_Model = nullptr;

SkillObjectSpear::SkillObjectSpear(int pos_x, int pos_y, int vx, int vy, time_t created_time)
	: SkillObjectClass(pos_x, pos_y, rect_t{ -30000, -30000, 30000, 30000 }),
	vx(vx), vy(vy), m_State(STATE_NORMAL), m_StateStartTime(created_time)
{
	m_Angle = (float)atan(vx / (double)vy);
}

void SkillObjectSpear::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	switch (m_State)
	{
	case STATE_NORMAL:
	{
		// Movement acoording to the current velocity.
		pos_x += (int)time_delta * vx;
		int start_y = pos_y, target_y = pos_y + (int)time_delta * vy;
		pos_y = target_y;

		for (auto& ground_obj : ground)
		{
			pos_y = max(pos_y,
				ground_obj->IsColiided(m_Range.x1 + pos_x, m_Range.x2 + pos_x, start_y, target_y));
		}

		if (pos_y != target_y)
		{
			m_State = STATE_ONGROUND;
			m_StateStartTime = curr_time;
		}

	}
		break;
	}
}

bool SkillObjectSpear::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;

	if (m_State == STATE_NORMAL) monster->Damage(10, collided_time, vx / 6, 0);
	else if (m_State == STATE_ONGROUND) monster->Damage(5, collided_time, vx / 6, 1000);

	m_State = STATE_DIE;
	return true;
}

bool SkillObjectSpear::Frame(time_t curr_time, time_t time_delta)
{
	switch (m_State)
	{
	case STATE_ONGROUND:
		if (m_StateStartTime + 1000 < curr_time)
		{
			m_State = STATE_DIE;
			return false;
		}
		break;

	case STATE_DIE:
		return false;
	}

	return true;
}

XMMATRIX SkillObjectSpear::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixRotationY(XM_PI / 2)
		* XMMatrixRotationZ(XM_PI - m_Angle) * XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixTranslation(pos_x * SCOPE, pos_y * SCOPE, 0.0f);
}

void SkillObjectSpear::initialize(ModelClass* model)
{
	m_Model = unique_ptr<ModelClass>(model);
}

ModelClass* SkillObjectSpear::GetModel()
{
	return m_Model.get();
}

SkillObjectBead::SkillObjectBead(int pos_x, int pos_y, int vx, int vy, time_t created_time)
	: SkillObjectClass(pos_x, pos_y, rect_t{ -30000, -30000, 30000, 30000 }),
	vx(vx), vy(vy), m_State(STATE_NORMAL), m_StateStartTime(created_time)
{
}

void SkillObjectBead::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	pos_x += (int)time_delta * vx;
	pos_y += (int)time_delta * vy;
}

bool SkillObjectBead::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;

	switch (m_State)
	{
	case STATE_NORMAL:
		monster->Damage(20, collided_time, vx / 8, 0);
		vx *= 2, vy *= 2;

		m_State = STATE_ONEHIT;
		m_StateStartTime = collided_time;
		break;
		
	case STATE_ONEHIT:
		monster->Damage(20, collided_time, vx / 8, 0);

		m_State = STATE_DIE;
		m_StateStartTime = collided_time;
	}
	return true;

}

bool SkillObjectBead::Frame(time_t curr_time, time_t time_delta)
{
	switch (m_State)
	{
	case STATE_NORMAL:
	case STATE_ONEHIT:
		if (m_StateStartTime + 1000 < curr_time)
		{
			m_State = STATE_DIE;
			return false;
		}
		break;

	case STATE_DIE:
		return false;
	}
	return true;
}

XMMATRIX SkillObjectBead::GetGlobalShapeTransform(time_t curr_time)
{
	return 
		XMMatrixScaling(0.35f, 0.35f, 0.35f) * XMMatrixRotationY(curr_time * 0.0002f * XM_PI)
		* XMMatrixTranslation(pos_x * SCOPE, pos_y * SCOPE, 0.0f);
}

void SkillObjectBead::initialize(class ModelClass* model)
{
	m_Model = unique_ptr<ModelClass>(model);
}

ModelClass* SkillObjectBead::GetModel()
{
	return m_Model.get();
}

SkillObjectLeg::SkillObjectLeg(int pos_x, time_t created_time)
	: SkillObjectClass(pos_x, GROUND_Y, rect_t{ -50000, -1200000, 50000, 0 }),
	m_StateStartTime(created_time)
{
	
}

void SkillObjectLeg::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	time_delta = min(time_delta, curr_time - m_StateStartTime);
	pos_y += 2'000 * time_delta;
}

bool SkillObjectLeg::OnCollided(MonsterClass* monster, time_t collided_time)
{
	rect_t upper = GetGlobalRange();
	upper.y1 = upper.y2 - 6000;
	
	if (SkillObjectClass::OnCollided(monster, collided_time))
	{
		if (upper.collide(monster->GetGlobalRange()))
		{
			monster->Damage(80, collided_time, 0, 3000);
			// TODO: stop
		}
		// TODO; direction
		else monster->Damage(30, collided_time, 0, 500);
		return true;
	}
	return false;
}

bool SkillObjectLeg::Frame(time_t curr_time, time_t time_delta)
{
	return m_StateStartTime + 1200 > curr_time;
}

XMMATRIX SkillObjectLeg::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixTranslation(pos_x * SCOPE, pos_y * SCOPE, 0.0f);
}


void SkillObjectLeg::initialize(class ModelClass* model)
{
	m_Model = unique_ptr<ModelClass>(model);
}


ModelClass* SkillObjectLeg::GetModel()
{
	return m_Model.get();
}



SkillObjectBasic::SkillObjectBasic(int pos_x, int pos_y, int vx, time_t created_time)
	: SkillObjectClass(pos_x, pos_y, rect_t{ -60000, 80000, 60000, 380000 }),
	m_StateStartTime(created_time), vx(vx)
{
}

void SkillObjectBasic::FrameMove(time_t curr_time, time_t time_delta,
	const vector<unique_ptr<class GroundClass> >& ground)
{
	
}

bool SkillObjectBasic::OnCollided(MonsterClass* monster, time_t collided_time)
{
	if (!SkillObjectClass::OnCollided(monster, collided_time)) return false;
	monster->Damage(40, collided_time, vx, 0);
	return true;
}

bool SkillObjectBasic::Frame(time_t curr_time, time_t time_delta)
{
	constexpr time_t lifetime = 200;
	return m_StateStartTime + lifetime > curr_time;
}

XMMATRIX SkillObjectBasic::GetGlobalShapeTransform(time_t curr_time)
{
	return XMMatrixIdentity();
}

void SkillObjectBasic::initialize(ModelClass* model)
{
	m_Model = unique_ptr<ModelClass>(model);
}

ModelClass* SkillObjectBasic::GetModel()
{
	return m_Model.get();
}
