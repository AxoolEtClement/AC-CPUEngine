#pragma once
#include <stb_truetype.h>
#include <string>
#include <vector>


namespace AC_Engine
{
    class Font
    {
    public:
        Font() = default;
        ~Font() = default;
        bool Load(const std::string& fontPath, int fontSize);
        stbtt_bakedchar m_chars[96] = {};
        std::vector<unsigned char> m_atlas;
        static constexpr int ATLAS_W = 512, ATLAS_H = 512;
    private:
        std::string m_fontPath;
    };
}