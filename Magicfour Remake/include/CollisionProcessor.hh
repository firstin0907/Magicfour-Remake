#pragma once

#include "GameObjectList.hh"
#include "IGameObject.hh"

class CollisionProcessor
{
public:

	template <typename A, typename B>
	static void Process(GameObjectList& listA, GameObjectList& listB, std::function<void(A*, B*)> handler)
	{
		for (auto& element_a : listA.elements)
		{
			if (!element_a->IsColliable()) continue;
			for (auto& element_b : listB.elements)
			{
				if (!element_b->IsColliable()) continue;
				if (element_a->GetGlobalRange().collide(element_b->GetGlobalRange()))
				{
					handler(static_cast<A*>(element_a.get()), static_cast<B*>(element_b.get()));
				}

			}
		}
	}

	template <typename A, typename B>
	static void Process(IGameObject* instance, GameObjectList& list, std::function<void(A*, B*)> handler)
	{
		if (!instance->IsColliable()) return;

		for (auto& element : list.elements)
		{
			if (!element->IsColliable()) continue;
			if (instance->GetGlobalRange().collide(element->GetGlobalRange()))
			{
				handler(static_cast<A*>(instance), static_cast<B*>(element.get()));
			}
		}
	}

	template <typename A, typename B>
	static void Process(GameObjectList& list, IGameObject* instance, std::function<void(A*, B*)> handler)
	{
		if (!instance->IsColliable()) return;

		for (auto& element : list.elements)
		{
			if (!element->IsColliable()) continue;
			if (instance->GetGlobalRange().collide(element->GetGlobalRange()))
			{
				handler(static_cast<A*>(element.get()), static_cast<B*>(instance));
			}
		}
	};


};
