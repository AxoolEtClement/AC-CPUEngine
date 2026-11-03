#pragma once
#include <tuple>
#include <ECS/sparseSet.hpp>
#include <algorithm>

namespace AC_Engine::ECS
{
	template <typename... Components>
	class View
	{
	public:
		View(SparseSet<Components>*... sets);

		class Iterator
		{
		public:
			Iterator(std::tuple<SparseSet<Components>*...>& sets, ISparseSet* smallestSet, unsigned int index);
			Iterator& operator++();
			bool operator==(const Iterator& other) const;
			std::tuple<Components&...> operator*() const;
		private:
			bool entityHasRequiredComponents() const;

			std::tuple<SparseSet<Components>*...> m_sets;
			ISparseSet* m_smallestSet = nullptr;
			unsigned int m_index = 0;
		};

		Iterator begin();
		Iterator end();

	private:
		std::tuple<SparseSet<Components>*...> m_sets;
		ISparseSet* m_smallestSet = nullptr;
	};


	// Definitions

	template<typename ...Components>
	inline View<Components...>::View(SparseSet<Components>*... sets)
		: m_sets(sets...)
	{
		ISparseSet* setsArray[] = { static_cast<ISparseSet*>(sets)... };
		m_smallestSet = setsArray[0];
		for (ISparseSet* set : setsArray)
		{
			if (set->size() < m_smallestSet->size())
			{
				m_smallestSet = set;
			}
		}
	}

	template<typename ...Components>
	inline View<Components...>::Iterator::Iterator(std::tuple<SparseSet<Components>*...>& sets, ISparseSet* smallestSet, unsigned int index)
		: m_sets(sets), m_index(index), m_smallestSet(smallestSet)
	{
		if (m_index < m_smallestSet->size() && !entityHasRequiredComponents())
		{
			++(*this);
		}
	}

	template<typename ...Components>
	inline typename View<Components...>::Iterator& View<Components...>::Iterator::operator++()
	{
		do
		{
			m_index++;
		} while (m_index < m_smallestSet->size() && !entityHasRequiredComponents());
		return *this;
	}
	template<typename ...Components>
	inline bool View<Components...>::Iterator::operator==(const Iterator& other) const
	{
		return m_index == other.m_index;
	}
	template<typename ...Components>
	inline std::tuple<Components&...> View<Components...>::Iterator::operator*() const
	{
		unsigned int entityID = m_smallestSet->getEntityAt(m_index);

		return std::apply([entityID](SparseSet<Components>*... sets) {
			return std::forward_as_tuple(sets->getComponent(entityID)...);
			}, m_sets
		);
	}
	template<typename ...Components>
	inline bool View<Components...>::Iterator::entityHasRequiredComponents() const
	{
		unsigned int entityID = m_smallestSet->getEntityAt(m_index);
		return std::apply([entityID](SparseSet<Components>*... sets) {
			return (... && sets->hasComponent(entityID));
			}, m_sets
		);
	}

	template<typename ...Components>
	inline typename View<Components...>::Iterator View<Components...>::begin()
	{
		return Iterator(m_sets, m_smallestSet, 0);
	}

	template<typename ...Components>
	inline typename View<Components...>::Iterator View<Components...>::end()
	{
		return Iterator(m_sets, m_smallestSet, m_smallestSet->size());
	}
}