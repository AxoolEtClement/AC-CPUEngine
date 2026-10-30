#pragma once
#include <unordered_map>
#include <vector>

namespace AC_Engine::ECS
{
	class ISparseSet
	{
	public:
		~ISparseSet() = default;
		virtual void removeComponent(unsigned int entityID) = 0;
		virtual bool hasComponent(unsigned int entityID) = 0;
		virtual unsigned int getEntityAt(unsigned int index) = 0;
		virtual size_t size() = 0;
	};

	template <typename Component>
	class SparseSet : public ISparseSet
	{
	public:
		void removeComponent(unsigned int entityID);
		bool hasComponent(unsigned int entityID);
		unsigned int getEntityAt(unsigned int denseIndex);
		size_t size() override;
		
		template<typename... Args>
		Component& addComponent(unsigned int entityID, Args... args);
		Component& getComponent(unsigned int entityID);

	private:
		std::vector<Component> m_components{};
		std::vector<unsigned int> m_dense{};
		std::unordered_map<unsigned int, size_t> m_sparse{};
	};


	// Definitions

	template<typename Component>
	inline void SparseSet<Component>::removeComponent(unsigned int entityID)
	{
		if (m_sparse.contains(entityID))
		{
			size_t indexToRemove = m_sparse[entityID];
			size_t lastIndex = m_dense.size() - 1;
			if (indexToRemove != lastIndex)
			{
				unsigned int lastEntityID = m_dense[lastIndex];
				m_dense[indexToRemove] = lastEntityID;
				m_sparse[lastEntityID] = indexToRemove;
				m_components[indexToRemove] = std::move(m_components[lastIndex]);
			}
			m_dense.pop_back();
			m_components.pop_back();
			m_sparse.erase(entityID);
		}
	}

	template<typename Component>
	inline bool SparseSet<Component>::hasComponent(unsigned int entityID)
	{
		return m_indexes.contains(entityID);
	}

	template<typename Component>
	inline unsigned int SparseSet<Component>::getEntityAt(unsigned int denseIndex)
	{
		return m_dense[denseIndex];
	}

	template<typename Component>
	inline size_t SparseSet<Component>::size()
	{
		return m_dense.size();
	}

	template<typename Component>
	inline Component& SparseSet<Component>::getComponent(unsigned int entityID)
	{
		if (m_indexes.contains(entityID))
		{
			return m_components[m_sparse[entityID]];
		}
		return nullptr;
	}

	
	template<typename Component>
	template<typename ...Args>
	inline Component& SparseSet<Component>::addComponent(unsigned int entityID, Args ...args)
	{
		if (!m_sparse.contains(entityID))
		{
			size_t newIndex = m_dense.size();
			m_dense.push_back(entityID);
			m_components.emplace_back(Component{ std::forward<Args>(args)... });
			m_sparse[entityID] = newIndex;
		}
		return m_components[m_sparse[entityID]];
	}

}
