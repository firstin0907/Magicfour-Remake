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

	void FrameMove(time_t curr_time, time_t delta_time, const class FieldClass* ground);

	void Frame(time_t curr_time, time_t delta_time, class SoundClass* sound_manager,
		std::function<void(IGameObject*)> on_delete = nullptr);

	void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		class GraphicResources* graphic_resources) const;

	void DrawUI(time_t curr_time, time_t time_delta, class UserInterfaceClass* ui) const;
};
