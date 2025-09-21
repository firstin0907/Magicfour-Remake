#pragma once

#include "core/global.hh"

#include <vector>
#include <memory>
#include <functional>

#include "util/ResourceMap.hh"

class GameObjectList
{
public:
	std::vector<std::unique_ptr<class IGameObject> > elements;

public:
	virtual ~GameObjectList();

	void Insert(class IGameObject* object);

	void FrameMove(time_t curr_time, time_t delta_time, const std::vector<class GroundClass>& ground);

	void Frame(time_t curr_time, time_t delta_time, std::function<void(IGameObject*)> on_delete = nullptr);

	void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const;
};
