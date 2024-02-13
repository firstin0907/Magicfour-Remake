#pragma once
#include "global.hh"
#include "RigidbodyClass.hh"

#include <memory>
#include <vector>

enum class ItemState
{
	kNormal, kDie
};

class ItemClass : public RigidbodyClass<ItemState>
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

	XMMATRIX GetShapeMatrix(time_t curr_time);

	inline int GetType() { return type_; }

private:
	int type_;
	time_t createTime_;
};