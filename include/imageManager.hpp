#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "rendering/texture.hpp"

namespace AC_Engine
{
class ImageManager {
public:
    ImageManager() = default;
    void addImage(const std::string& path, std::string name = "") 
    {
        if (m_images.find(path) != m_images.end()) {
            return;
        }
        Texture texture = TextureLoader::loadTexture(path);
        if (texture.pixels == nullptr) {
            throw std::runtime_error("Failed to load image: " + path);
        }
        m_images[path] = std::make_shared<Texture>(texture);
    }
    std::shared_ptr<Texture> getImage(const std::string& path) {
        auto it = m_images.find(path);
        if (it != m_images.end()) {
            return it->second;
        }
        addImage(path);
        return m_images[path];
    }
private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_images;
};
}