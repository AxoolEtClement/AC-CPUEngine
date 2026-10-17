#pragma once
#include <stb_image.h>
#include <string>
#include <cstdint>
#include <stdexcept>


struct Texture
{
    uint32_t *pixels;
    int width;
    int height;
};

class TextureLoader
{
    public:
        static Texture loadTexture(const std::string& filePath)
        {
            Texture texture{};
            int channels;
            texture.pixels = reinterpret_cast<uint32_t*>(stbi_load(filePath.c_str(), &texture.width, &texture.height, &channels, 4));
            if (!texture.pixels)
            {
                throw std::runtime_error("Failed to load texture: " + filePath);
            }
            return texture;
        }
        static void freeTexture(Texture& texture)
        {
            if (texture.pixels)
            {
                stbi_image_free(texture.pixels);
                texture.pixels = nullptr;
            }
        }
        static uint32_t sampleTexture(const Texture& texture, float u, float v)
        {
            if (!(u == u) || !(v == v)) { u = 0.f; v = 0.f; }

            u = std::clamp(u, 0.0f, 1.0f);        
            v = std::clamp(v, 0.0f, 1.0f);
            v = 1.0f - v;                                       

            int x = static_cast<int>(u * (texture.width  - 1));
            int y = static_cast<int>(v * (texture.height - 1));

            return texture.pixels[y * texture.width + x];
        }
        static uint32_t color_scale(uint32_t color, float scale)
        {
            // Clamp the scale to [0, 1]
            scale = std::clamp(scale, 0.0f, 1.0f);
            // AABBGGRR
            uint8_t a = (color >> 24) & 0xFF;
            uint8_t b = static_cast<uint8_t>(((color >> 16) & 0xFF) * scale);
            uint8_t g = static_cast<uint8_t>(((color >> 8) & 0xFF) * scale);
            uint8_t r = static_cast<uint8_t>((color & 0xFF) * scale);

            return (a << 24) | (b << 16) | (g << 8) | r;
        }
};