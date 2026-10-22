#include "objparser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

renderObject AC_Engine::ObjParser::parseObjFile(const std::string& filename)
{
    renderObject obj;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open OBJ file: " << filename << std::endl;
        return obj;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.size() < 2) continue;   // évite line[1] hors limites sur une ligne vide

        switch (line[0])
        {
            case 'v':
                if (line[1] == ' ')                      // "v x y z"
                {
                    std::istringstream iss(line.substr(2));
                    float x, y, z;
                    iss >> x >> y >> z;
                    obj.vertices.push_back({ simd::Vec4{ x, y, z, 1.0f }, { 0.f, 0.f } });
                }
                else if (line[1] == 't')                 // "vt u v"
                {
                    std::istringstream iss(line.substr(3));
                    float u, v;
                    iss >> u >> v;
                    obj.uvs.push_back({ u, v });         // stocké dans la liste, pas dans un sommet
                }
                break;                                   // "vn" ignoré

            case 'f':
                {
                    std::istringstream iss(line.substr(2));
                    std::vector<size_t> pos;
                    std::vector<int> tex;
                    std::string token;

                    while (iss >> token)                 // formats : v, v/vt, v/vt/vn, v//vn
                    {
                        size_t s1 = token.find('/');
                        pos.push_back(std::stoul(token.substr(0, s1)) - 1);

                        int ti = -1;
                        if (s1 != std::string::npos)
                        {
                            size_t s2 = token.find('/', s1 + 1);
                            std::string t = token.substr(s1 + 1, s2 == std::string::npos ? std::string::npos : s2 - s1 - 1);
                            if (!t.empty()) ti = std::stoi(t) - 1;
                        }
                        tex.push_back(ti);
                    }

                    // triangulation en éventail (gère aussi les quads)
                    for (size_t i = 1; i + 1 < pos.size(); ++i)
                    {
                        obj.faces.push_back({ pos[0], pos[i], pos[i + 1] });
                        obj.face_uvs.push_back(tex[0]);
                        obj.face_uvs.push_back(tex[i]);
                        obj.face_uvs.push_back(tex[i + 1]);
                    }
                }
                break;

            default:
                break;
        }
    }
    return obj;
}