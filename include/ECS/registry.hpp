#pragma once
#include <unordered_map>
#include <ECS/sparseSet.hpp>
#include <typeindex>

namespace AC_Engine::ECS
{
	class Registry
	{
		template <typename T, typename ...Args>
		void addComponent(unsigned int entityID, T component, Args&... args);
		
		template <typename T>
		T& getComponent(unsigned int entityID);

		template <typename ...Types>
		class view
		{

		};

	private:
		std::unordered_map<std::type_index, ISparseSet*> m_storage{};
	};

	// Definitions

	template<typename T, typename ...Args>
	inline void Registry::addComponent(unsigned int entityID, T component, Args& ...args)
	{
		if (!m_storage.contains(std::type_index(typeid(T))))
		{
			m_storage[std::type_index(typeid(T))] = new SparseSet<T>{};
		}

		m_storage[std::type_index(typeid(T))]->addComponent(entityID);
	}

	template <typename T>
	T& Registry::getComponent(unsigned int entityID)
	{
		auto sSet = dynamic_cast<SparseSet<T>*>(m_storage[std::type_index(typeid(T))]);
		sSet->getComponent(entityID);
		
	}
}