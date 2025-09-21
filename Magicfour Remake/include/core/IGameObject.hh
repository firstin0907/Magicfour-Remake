#pragma once

#include <vector>
#include <ctime>

#include "core/global.hh"
#include "util/ResourceMap.hh"

class IGameObject
{
public:
	virtual ~IGameObject() {};

	// Change the location for one frame.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const std::vector<class GroundClass>& ground) = 0;
	
	// Proceed the logic for one frame, and return this is still alive.
	virtual bool Frame(time_t curr_time, time_t time_delta) = 0;

	// Return the global range.
	virtual rect_t GetGlobalRange() const = 0;

	// Check if this instance is on collidable state.
	virtual bool IsColliable() const = 0;

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const {};
};