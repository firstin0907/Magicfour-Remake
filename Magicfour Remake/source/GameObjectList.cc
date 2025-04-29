#include "../include/GameObjectList.hh"

#include "../include/IGameObject.hh"

GameObjectList::~GameObjectList()
{
};


void GameObjectList::Insert(IGameObject* object)
{
	elements.emplace_back(object);
}


void GameObjectList::FrameMove(time_t curr_time, time_t delta_time, const std::vector<class GroundClass>& ground)
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
