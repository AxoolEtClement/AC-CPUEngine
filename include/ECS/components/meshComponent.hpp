#pragma once
#include <memory>
#include "renderData.hpp"

struct MeshComponent
{
	std::shared_ptr<Mesh> mesh{};
};