#include "ECS/Entity.hpp"

namespace AC_Engine::ECS
{
	bool Entity::operator==(const Entity& other) const
	{
		return id == other.id && gen == other.gen;
	}

	Entity EntityManager::createEntity()
	{
		unsigned int id;
		if (!m_freeIds.empty())
		{
			id = m_freeIds.back();
			m_freeIds.pop_back();
		}
		else
		{
			id = m_nextId++;
			m_generations.push_back(0);
		}

		return Entity{ id, m_generations[id] };
	}

	void EntityManager::destroyEntity(Entity entity)
	{
		if (isAlive(entity))
		{
			m_generations[entity.id]++;
			m_freeIds.push_back(entity.id);
		}
	}

	bool EntityManager::isAlive(Entity entity) const
	{
		if (entity.id >= m_generations.size())
		{
			return false;
		}

		return m_generations[entity.id] == entity.gen;
	}
}