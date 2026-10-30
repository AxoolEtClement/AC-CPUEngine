#pragma once
#include <unordered_map>
#include "ECS/sparseSet.hpp"
#include "ECS/view.hpp"
#include "ECS/Entity.hpp"
#include <typeindex>


namespace AC_Engine::ECS
{
	class Registry
	{
		~Registry();
		
		template <typename Component, typename ...Args>
		Component& addComponent(unsigned int entityID, Component component, Args&... args);

		template <typename Component>
		void removeComponent(unsigned int entityID);

		template <typename Component>
		Component& getComponent(unsigned int entityID);

		template <typename Component>
		bool hasComponent(unsigned int entityID);

		void removeEntity(unsigned int entityID);

		template <typename... Components>
		View<Components...> view();

	private:
		std::unordered_map<std::type_index, ISparseSet*> m_storage{};
	};


	// Definitions

	AC_Engine::ECS::Registry::~Registry()
	{
		for (auto& setPair : m_storage)
			delete setPair.second;
	}

	template<typename Component, typename ...Args>
	inline Component& Registry::addComponent(unsigned int entityID, Component component, Args& ...args)
	{
		if (!m_storage.contains(std::type_index(typeid(Component))))
		{
			m_storage[std::type_index(typeid(Component))] = new SparseSet<Component>{};
		}

		return static_cast<SparseSet<Component>*>(m_storage[std::type_index(typeid(Component))])->addComponent(entityID, std::forward<Args>(args)...);
	}
	
	template <typename Component>
	void Registry::removeComponent(unsigned int entityID)
	{
		m_storage[std::type_index(typeid(Component))]->removeComponent(entityID);
	}
	
	template <typename Component>
	inline Component& Registry::getComponent(unsigned int entityID)
	{
		SparseSet<Component>* sSet = static_cast<SparseSet<Component>*>(m_storage[std::type_index(typeid(Component))]);
		return sSet->getComponent(entityID);
	}

	template <typename Component>
	inline bool Registry::hasComponent(unsigned int entityID)
	{
		return m_storage[std::type_index(typeid(Component))]->hasComponent(entityID);
	}

	inline void Registry::removeEntity(unsigned int entityID)
	{
		for (auto& setPair : m_storage)
		{
			setPair.second->removeComponent(entityID);
		}
	}

	template<typename ...Components>
	inline View<Components...> Registry::view()
	{
		return View<Components...>(static_cast<SparseSet<Components>*>(m_storage[std::type_index(typeid(Components))])...);
	}
}
