#pragma once
#include "core/global.hh"
#include "core/RigidbodyClass.hh"

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
	using GroundVector = std::vector<class GroundClass>;

public:
	ItemClass(time_t create_time, int x_pos, int y_pos, int type);

	// Change the location for one frame.
	virtual void FrameMove(time_t curr_time, time_t time_delta, const std::vector<class GroundClass>& ground) override final;

	// Proceed the logic for one frame, and return this is still alive.
	virtual bool Frame(time_t curr_time, time_t time_delta) override final;

	// Check if this instance is on collidable state.
	virtual bool IsColliable() const override final;

	XMMATRIX GetShapeMatrix(time_t curr_time);

	inline int GetType() { return type_; }

private:
	int type_;
	time_t createTime_;
};