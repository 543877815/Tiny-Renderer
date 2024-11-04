#pragma once
#include <string>
#include <unordered_set>
#include "./common.h"

REGISTER_BEGIN
struct RenderObjConfig {
	std::string obj_type;
	std::string vertex_shader;
	std::string fragment_shader;
	std::unordered_set<std::string> uniform;
	std::string projection;
};

REGISTER_END
