#pragma once
#include <unordered_map>
#include <vector>

namespace AC_Engine::ECS
{
	
	class ISparseSet
	{
		~ISparseSet() = default;
		virtual void addComponent(unsigned int entityID) = 0;
		virtual void removeComponent(unsigned int entityID) = 0;
		virtual bool hasComponent(unsigned int entityID) = 0;
	};

	template <typename T>
	class SparseSet : ISparseSet
	{
		void addComponent(unsigned int entityID);
		void removeComponent(unsigned int entityID);
		bool hasComponent(unsigned int entityID);
		T& getComponent(unsigned int entityID);

	private:
		std::vector<T> m_components{};
		std::unordered_map<unsigned int, T> m_indexes{};
	};

	// Definitions

	template<typename T>
	inline void SparseSet<T>::addComponent(unsigned int entityID)
	{
		if (!m_indexes.contains(entityID))
		{
			m_components.emplace_back();
			m_indexes[entityId] = m_components.size - 1;
		}
	}

	template<typename T>
	inline void SparseSet<T>::removeComponent(unsigned int entityID)
	{
		if (m_indexes.contains(entityID))
		{
			m_components[m_indexes[entityID]] = m_components.end();
			m_components.pop_back();
			m_indexes.erase(entityID);
		}
	}

	template<typename T>
	inline bool SparseSet<T>::hasComponent(unsigned int entityID)
	{
		return m_indexes.contains(entityID);
	}

	template<typename T>
	inline T& SparseSet<T>::getComponent(unsigned int entityID)
	{
		if (m_indexes.contains(entityID))
		{
			return m_components[m_indexes[entityID]];
		}
		return nullptr;
	}

}
