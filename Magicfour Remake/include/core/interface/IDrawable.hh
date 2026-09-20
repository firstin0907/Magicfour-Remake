#pragma once

#include <ctime>

#include "core/common/GraphicResources.hh"

//ResourceMap<class ModelClass>& models, ResourceMap<class FbxModel>& fbx_models, ResourceMap<class TextureClass>& textures

class IDrawable
{
public:
	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		class GraphicResources* graphic_resources) const {};

	virtual void DrawUI(time_t curr_time, time_t time_delta, class UserInterfaceClass* ui) const {};
};