#pragma once
#include "rendering/renderData.hpp"
#include "objparser.hpp"
#include <unordered_map>
#include <memory>

namespace AC_Engine
{
class MeshManager
{
public:
    MeshManager() = default;
    ~MeshManager() = default;

    void addMesh(const std::string& name, std::string filePath)
    {
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(AC_Engine::ObjParser::parseObjFile(filePath));

        m_meshes[name] = mesh;
    }

    std::shared_ptr<Mesh> getMesh(const std::string& name)
    {
        auto it = m_meshes.find(name);
        if (it != m_meshes.end())
        {
            return it->second;
        }
        return nullptr;
    }
private:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
};
}