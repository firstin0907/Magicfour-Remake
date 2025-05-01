#pragma once

#include <vector>
#include <memory>
#include <functional>

class GameObjectList
{
public:
	std::vector<std::unique_ptr<class IGameObject> > elements;

public:
	virtual ~GameObjectList();

	void Insert(class IGameObject* object);

	void FrameMove(time_t curr_time, time_t delta_time, const std::vector<class GroundClass>& ground);

	void Frame(time_t curr_time, time_t delta_time, std::function<void(IGameObject*)> on_delete = nullptr);
};
