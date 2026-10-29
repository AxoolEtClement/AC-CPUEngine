#pragma once
#include <unordered_map>
#include <ECS/sparseSet.hpp>
#include <ECS/view.hpp>
#include <typeindex>


namespace AC_Engine::ECS
{
	class Registry
	{
		template <typename Component, typename ...Args>
		void addComponent(unsigned int entityID, Component component, Args&... args);
		
		template <typename Component>
		Component& getComponent(unsigned int entityID);

		template <typename... Components>
		View<Components...> view();

	private:
		std::unordered_map<std::type_index, ISparseSet*> m_storage{};
	};


	// Definitions

	template<typename Component, typename ...Args>
	inline void Registry::addComponent(unsigned int entityID, Component component, Args& ...args)
	{
		if (!m_storage.contains(std::type_index(typeid(Component))))
		{
			m_storage[std::type_index(typeid(Component))] = new SparseSet<Component>{};
		}

		m_storage[std::type_index(typeid(Component))]->addComponent(entityID);
	}

	template <typename Component>
	inline Component& Registry::getComponent(unsigned int entityID)
	{
		SparseSet<Component>* sSet = static_cast<SparseSet<Component>*>(m_storage[std::type_index(typeid(Component))]);
		sSet->getComponent(entityID);
		
	}

	template<typename ...Components>
	inline View<Components...> Registry::view()
	{
		return View<Components...>(static_cast<SparseSet<Components>*>(m_storage[std::type_index(typeid(Components))])...);
	}
}
