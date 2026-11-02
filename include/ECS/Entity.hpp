#pragma once
#include <vector>

namespace AC_Engine::ECS
{
	struct Entity
	{
		unsigned int id;
		unsigned int gen;

		bool operator==(const Entity& other) const;
	};

	class EntityManager
	{
	public:
		EntityManager() = default;
		Entity createEntity();
		void destroyEntity(Entity entity);
		bool isAlive(Entity entity) const;
	private:
		std::vector<unsigned int> m_generations;
		std::vector<unsigned int> m_freeIds;
		unsigned int m_nextId = 0;
	};
}
