#define STB_TRUETYPE_IMPLEMENTATION
#include "font.hpp"
#include <fstream>
#include <iterator>
#include <vector>

bool AC_Engine::Font::Load(const std::string& fontPath, int fontSize)
{
    std::ifstream file(fontPath, std::ios::binary);
    if (!file) return false;

    std::vector<unsigned char> ttf((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());

    m_fontPath = fontPath;
    m_atlas.assign(ATLAS_W * ATLAS_H, 0);

    int res = stbtt_BakeFontBitmap(ttf.data(), 0, static_cast<float>(fontSize),
                                   m_atlas.data(), ATLAS_W, ATLAS_H,
                                   32, 96, m_chars);
    return res > 0;
}