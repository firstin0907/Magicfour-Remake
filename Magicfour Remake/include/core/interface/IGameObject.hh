#pragma once

#include <vector>
#include <ctime>

#include "core/global.hh"
#include "util/ResourceMap.hh"
#include "core/interface/IDrawable.hh"

class IGameObject : public IDrawable
{
public:
	virtual ~IGameObject() {};

	// Change the location for one frame.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const class FieldClass* ground) = 0;
	
	// Proceed the logic for one frame, and return this is still alive.
	virtual bool Frame(time_t curr_time, time_t time_delta, class SoundClass* sound_manager) = 0;

	// Return the global range.
	virtual rect_t GetGlobalRange() const = 0;

	// Check if this instance is on collidable state.
	virtual bool IsColliable() const = 0;

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		class GraphicResources* graphic_resources) const {};
};