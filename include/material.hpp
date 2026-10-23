#pragma once
#include <string>
#include <array>
#include <vector>
#include <cmath>
#include <memory>
#include "texture.hpp"

struct Color3 { float r = 1, g = 1, b = 1; };

class Material
{
public:

    Material() = default;
    Material(const std::string& name) : name(name) {}
    Material(const std::string& name, const Color3& ambient, const Color3& diffuse, const Color3& specular, const Color3& emissive, float shininess, float opacity)
        : name(name), ambient(ambient), diffuse(diffuse), specular(specular), emissive(emissive), shininess(shininess), opacity(opacity) {}

    std::string name;

    Color3 ambient   { 1.f, 1.f, 1.f };
    Color3 diffuse   { 1.f, 1.f, 1.f };
    Color3 specular  { 1.f, 1.f, 1.f };
    Color3 emissive  { 0.f, 0.f, 0.f };
    float  shininess = 32.f; 
    float  opacity   = 1.f;

    std::shared_ptr<Texture> diffuseMap  = nullptr;
    std::shared_ptr<Texture> specularMap = nullptr;
    std::shared_ptr<Texture> normalMap   = nullptr;

    // LUT pour remplacer pow(x, shininess) par une lecture de tableau
    std::array<float, 256> specLut{};
    void buildSpecLut() 
    {
        for (int i = 0; i < 256; i++)
            specLut[i] = std::pow(i / 255.f, shininess);
    }
};

class MaterialLibrary {
public:
    int add(Material m) { m_materials.push_back(std::move(m)); return (int)m_materials.size() - 1; }
    const Material& get(int id) const { return m_materials[id]; }
private:
    std::vector<Material> m_materials;
};