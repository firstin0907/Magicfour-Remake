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
	virtual void FrameMove(time_t curr_time, time_t time_delta, const class FieldClass* ground) override final;

	// Proceed the logic for one frame, and return this is still alive.
	virtual bool Frame(time_t curr_time, time_t time_delta, class SoundClass* sound_manager) override final;

	// Check if this instance is on collidable state.
	virtual bool IsColliable() const override final;

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, ShaderManager* shader_manager,
		class GraphicResources* graphic_resources) const override final;

	XMMATRIX GetShapeMatrix(time_t curr_time) const;

	inline int GetType() const { return type_; };

private:
	int type_;
	time_t createTime_;
};