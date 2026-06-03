#include "core/GameObjectList.hh"

#include "core/interface/IGameObject.hh"
#include "shader/ShaderManager.hh"
#include "util/ResourceMap.hh"

GameObjectList::~GameObjectList()
{
};


void GameObjectList::Insert(IGameObject* object)
{
	elements.emplace_back(object);
}


void GameObjectList::FrameMove(time_t curr_time, time_t delta_time, const FieldClass* ground)
{
	for (auto& element : elements)
	{
		element->FrameMove(curr_time, delta_time, ground);
	}
}

void GameObjectList::Frame(time_t curr_time, time_t delta_time, std::function<void(IGameObject*)> on_delete)
{
	for (int i = 0; i < elements.size(); i++)
	{
		// If this skill object should be deleted,
		if (!elements[i]->Frame(curr_time, delta_time))
		{
			if (on_delete) on_delete(elements[i].get());

			// swap with last element and pop it.
			swap(elements[i], elements.back());
			elements.pop_back();
		}
	}
}

void GameObjectList::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	class GraphicResources* graphic_resources) const
{
	for (auto& element : elements)
	{
		element->Draw(curr_time, time_delta, shader_manager, graphic_resources);
	}
}