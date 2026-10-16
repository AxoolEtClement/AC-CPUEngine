#include "objparser.hpp"

renderObject AC_Engine::ObjParser::parseObjFile(const std::string& filename)
{
    renderObject obj;
    std::ifstream file(filename);
    if(!file.is_open())
    {
        std::cerr << "Error: Could not open OBJ file: " << filename << std::endl;
        return obj;
    }
    std::string line;
    while (std::getline(file, line))
    {
        switch (line[0])
        {
            case 'v':
                if (line[1] == ' ')
                {
                    obj.vertices.push_back({simd::Vec3f{std::stof(line.substr(2, line.find(' ', 2) - 2)),
                                                       std::stof(line.substr(line.find(' ', 2) + 1, line.find(' ', line.find(' ', 2) + 1) - line.find(' ', 2) - 1)),
                                                       std::stof(line.substr(line.find_last_of(' ') + 1))}});
                }
                break;
            case 'f':
                {
                    std::array<size_t, 3> face{};
                    size_t start = 2;
                    for (int i = 0; i < 3; ++i)
                    {
                        size_t end = line.find(' ', start);
                        face[i] = std::stoul(line.substr(start, end - start)) - 1;
                        start = end + 1;
                    }
                    obj.faces.push_back(face);
                }
                break;
            default:
                break;
        }
        
    }
    return obj;
}