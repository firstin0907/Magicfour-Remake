#include "../include/ItemClass.hh"

#include <algorithm>

#include "../include/GroundClass.hh"

using namespace std;

constexpr rect_t ITEM_RANGE = { -30000, -10000, 30000, 60000 };
constexpr time_t ITEM_LIFETIME = 10'000;

ItemClass::ItemClass(time_t create_time, int x_pos, int y_pos, int type)
	: m_createTime(create_time), m_xPos(x_pos), m_yPos(y_pos), m_type(type)
{
	m_state = STATE_NORMAL;
	m_vy = 1000;
}

void ItemClass::FrameMove(time_t curr_time, time_t time_delta,
	const GroundVector& ground)
{
	const int before_vy = m_vy, after_vy = m_vy - GRAVITY * time_delta;

	if (after_vy >= 0) m_yPos += (before_vy + after_vy) / 2 * time_delta;	
	else if (before_vy > 0)
	{
		const int max_y = m_yPos + before_vy / 2 * before_vy / GRAVITY - ITEM_RANGE.y1;
		const int target = m_yPos + (before_vy + after_vy) / 2 * time_delta - ITEM_RANGE.y1;

		m_yPos = target;
		for (auto& ground_obj : ground)
		{
			m_yPos = max(m_yPos, ground_obj->IsColiided(ITEM_RANGE.x1 + m_xPos,
					ITEM_RANGE.x2 + m_xPos, max_y, m_yPos));
		}

		// For the case item is collided with the ground, it should stop.
		if (m_yPos != target)
		{
			m_vy = 0; // it should stop.
		}
		m_yPos += ITEM_RANGE.y1;
	}
	else
	{
		const int max_y = m_yPos - ITEM_RANGE.y1;;
		const int target = m_yPos + (before_vy + after_vy) / 2 * time_delta - ITEM_RANGE.y1;;
		m_yPos = target;

		for (auto& ground_obj : ground)
		{
			m_yPos = max(m_yPos, ground_obj->IsColiided(ITEM_RANGE.x1 + m_xPos,
				ITEM_RANGE.x2 + m_xPos, max_y, m_yPos));
		}

		// For the case item is collided with the ground, it should stop.
		if (m_yPos != target)
		{
			m_vy = 0; 
		}
		m_yPos += ITEM_RANGE.y1;
	}
	
	m_vy = after_vy;
}

bool ItemClass::Frame(time_t curr_time, time_t time_delta)
{
	return curr_time <= m_createTime + ITEM_LIFETIME && m_state == STATE_NORMAL;
}

XMMATRIX ItemClass::GetLocalWorldMatrix()
{
	return XMMatrixTranslation(m_xPos * SCOPE, m_yPos * SCOPE, 0);
}

XMMATRIX ItemClass::GetShapeMatrix(time_t curr_time)
{
	constexpr float box_size = 0.3f;
	const time_t age = curr_time - m_createTime;

	return XMMatrixTranslation(0, sin(age * 0.001) * 30000 * SCOPE, 0) *
		XMMatrixRotationY(age * 0.001f) * XMMatrixScaling(box_size, box_size * 1.2f, box_size);
}

XMMATRIX ItemClass::GetRangeRepresentMatrix()
{
	return ITEM_RANGE.add(m_xPos, m_yPos).toMatrix();
}

rect_t ItemClass::GetGlobalRange()
{
	return ITEM_RANGE.add(m_xPos, m_yPos);
}
