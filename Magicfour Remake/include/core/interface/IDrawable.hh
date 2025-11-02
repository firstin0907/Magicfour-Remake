#pragma once

#include <ctime>
#include "util/ResourceMap.hh"

class IDrawable
{
	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const {};
};