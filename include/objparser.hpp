#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include "renderObject.hpp"

namespace AC_Engine
{
    class ObjParser
    {
    public:
        static renderObject parseObjFile(const std::string& filename);
    };
}