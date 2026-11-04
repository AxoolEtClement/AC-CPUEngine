#pragma once
#include <memory>
#include "renderData.hpp"

namespace AC_Engine
{
	struct MeshComponent
	{
		std::shared_ptr<Mesh> mesh{};
	};
}
