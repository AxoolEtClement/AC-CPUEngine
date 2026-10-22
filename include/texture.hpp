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
    static uint32_t sampleTextureBilinear(const Texture& texture, float u, float v)
    {
        if (!(u == u) || !(v == v)) { u = 0.f; v = 0.f; }

        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);
        v = 1.0f - v;

        int x = static_cast<int>(u * (texture.width - 1));
        int y = static_cast<int>(v * (texture.height - 1));

        int   x0 = static_cast<int>(floorf(x));
        int   y0 = static_cast<int>(floorf(y));
        int   x1 = x0 + 1;
        int   y1 = y0 + 1;

        float tx = x - static_cast<float>(x0);
        float ty = y - static_cast<float>(y0);

        if (x0 < 0) x0 = 0; if (x0 >= texture.width)  x0 = texture.width - 1;
        if (y0 < 0) y0 = 0; if (y0 >= texture.height) y0 = texture.height - 1;
        if (x1 < 0) x1 = 0; if (x1 >= texture.width)  x1 = texture.width - 1;
        if (y1 < 0) y1 = 0; if (y1 >= texture.height) y1 = texture.height - 1;

        uint32_t c00 = texture.pixels[y0 * texture.width + x0];
        uint32_t c10 = texture.pixels[y0 * texture.width + x1];
        uint32_t c01 = texture.pixels[y1 * texture.width + x0];
        uint32_t c11 = texture.pixels[y1 * texture.width + x1];

        float w00 = (1.0f - tx) * (1.0f - ty);
        float w10 = tx * (1.0f - ty);
        float w01 = (1.0f - tx) * ty;
        float w11 = tx * ty;

        float b = w00 * ((c00 >> 16) & 0xFF) + w10 * ((c10 >> 16) & 0xFF) + w01 * ((c01 >> 16) & 0xFF) + w11 * ((c11 >> 16) & 0xFF);
        float g = w00 * ((c00 >> 8) & 0xFF) + w10 * ((c10 >> 8) & 0xFF) + w01 * ((c01 >> 8) & 0xFF) + w11 * ((c11 >> 8) & 0xFF);
        float r = w00 * ((c00 >> 0) & 0xFF) + w10 * ((c10 >> 0) & 0xFF) + w01 * ((c01 >> 0) & 0xFF) + w11 * ((c11 >> 0) & 0xFF);

        return (static_cast <uint32_t>(0xFFu) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r);
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