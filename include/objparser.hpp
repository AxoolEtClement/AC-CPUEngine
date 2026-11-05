#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include "rendering/renderData.hpp"

namespace AC_Engine
{
    class ObjParser
    {
    public:
        static Mesh parseObjFile(const std::string& filename);
    };
}