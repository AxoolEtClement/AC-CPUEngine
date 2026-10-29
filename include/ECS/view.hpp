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
			Iterator(std::tuple<SparseSet<Components>*...>& sets, unsigned int index);
			Iterator& operator++();
			bool operator==(const Iterator& other) const;
			std::tuple<Components&...> operator*() const;
		private:
			bool entityHasRequiredComponents() const;
			unsigned int getMinSize() const;

			std::tuple<SparseSet<Components>*...> m_sets;
			unsigned int m_index = 0;

		};

		Iterator begin();
		Iterator end();

	private:
		std::tuple<SparseSet<Components>*...> m_sets;
	};


	// Definitions

	template<typename ...Components>
	inline View<Components...>::View(SparseSet<Components>*... sets)
		: m_sets(sets...)
	{
	}
	template<typename ...Components>
	inline View<Components...>::Iterator::Iterator(std::tuple<SparseSet<Components>*...>& sets, unsigned int index)
		: m_sets(sets), m_index(index)
	{
		if (m_index < getMinSize() && !entityHasRequiredComponents())
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
		} while (m_index < getMinSize() && !entityHasRequiredComponents());
		return *this;
	}
	template<typename ...Components>
	inline bool View<Components...>::Iterator::operator==(const Iterator& other) const
	{
		return m_sets == other.m_sets && m_index == other.m_index;
	}
	template<typename ...Components>
	inline std::tuple<Components&...> View<Components...>::Iterator::operator*() const
	{
		unsigned int entityID = std::get<0>(m_sets)->getEntityAt(m_index);

		return std::apply([entityID](SparseSet<Components>*... sets) {
			return std::forward_as_tuple(sets->getComponent(entityID)...);
			}, m_sets
		);
	}
	template<typename ...Components>
	inline bool View<Components...>::Iterator::entityHasRequiredComponents() const
	{
		unsigned int entityID = std::get<0>(m_sets)->getEntityAt(m_index);
		return std::apply([entityID](SparseSet<Components>*... sets) {
			return (... && sets->hasComponent(entityID));
			}, m_sets
		);
	}

	template<typename ...Components>
	inline unsigned int View<Components...>::Iterator::getMinSize() const
	{
		return std::min({ std::get<SparseSet<Components>*>(m_sets)->size()... });
	}

	template<typename ...Components>
	inline typename View<Components...>::Iterator View<Components...>::begin()
	{
		return Iterator(m_sets, 0);
	}

	template<typename ...Components>
	inline typename View<Components...>::Iterator View<Components...>::end()
	{
		return Iterator(
			m_sets, 
			std::min({ std::get<SparseSet<Components>*>(m_sets)->size()... })
		);
	}
}