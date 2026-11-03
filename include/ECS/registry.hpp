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
	public:
		~Registry();
		
		template <typename Component, typename ...Args>
		Component& addComponent(unsigned int entityID, Args&&... args);

		template <typename Component>
		void removeComponent(unsigned int entityID);

		template <typename Component>
		Component& getComponent(unsigned int entityID);

		template <typename Component>
		bool hasComponent(unsigned int entityID);

		Entity createEntity();
		void removeEntity(Entity entity);

		template <typename... Components>
		View<Components...> view();

	private:
		std::unordered_map<std::type_index, ISparseSet*> m_storage{};
		EntityManager m_entityManager{};
	};


	// Definitions

	inline Registry::~Registry()
	{
		for (auto& setPair : m_storage)
			delete setPair.second;
	}

	template<typename Component, typename ...Args>
	inline Component& Registry::addComponent(unsigned int entityID, Args&& ...args)
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
		auto typeIndex = std::type_index(typeid(Component));
		if (!m_storage.contains(typeIndex))
			return;
		m_storage[typeIndex]->removeComponent(entityID);
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
		auto typeIndex = std::type_index(typeid(Component));
		if (!m_storage.contains(typeIndex))
			return false;
		
		return m_storage[typeIndex]->hasComponent(entityID);
	}

	inline Entity Registry::createEntity()
	{
		return m_entityManager.createEntity();
	}

	inline void Registry::removeEntity(Entity entity)
	{
		unsigned int entityID = entity.id;
		for (auto& setPair : m_storage)
		{
			if (setPair.second->hasComponent(entityID))
				setPair.second->removeComponent(entityID);
		}
		m_entityManager.destroyEntity(entity);
	}

	template<typename ...Components>
	inline View<Components...> Registry::view()
	{
		return View<Components...>(static_cast<SparseSet<Components>*>(m_storage[std::type_index(typeid(Components))])...);
	}
}
