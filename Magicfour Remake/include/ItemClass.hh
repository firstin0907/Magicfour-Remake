#pragma once
#include "global.hh"

#include <memory>
#include <vector>

class ItemClass
{
private:
	using XMMATRIX = DirectX::XMMATRIX;
	using GroundVector = std::vector<std::unique_ptr<class GroundClass> >;

public:
	enum state_t { STATE_NORMAL, STATE_DIE };

	ItemClass(time_t create_time, int x_pos, int y_pos, int type);

	// Move instance as time goes by.
	void FrameMove(time_t curr_time, time_t time_delta,
		const GroundVector& ground);

	// Decide this item should be deleted due to the time.
	bool Frame(time_t curr_time, time_t time_delta);

	XMMATRIX GetLocalWorldMatrix();
	XMMATRIX GetShapeMatrix(time_t curr_time);
	
	// for debug
	XMMATRIX GetRangeRepresentMatrix();
	rect_t GetGlobalRange();


	inline int GetType() { return m_type; }

	inline void SetState(state_t state)
	{
		m_state = state;
	}
	inline state_t GetState()
	{
		return m_state;
	}

private:
	state_t m_state;

	int m_xPos, m_yPos, m_vy, m_type;
	time_t m_createTime;
};