#pragma once
#include "./common.h"
#include <string>
#include <vector>
REGISTER_BEGIN
enum Operator : uint32_t
{
	NEXT,
	PREVIOUS,
	CURRENT
};

class RegisterConfigPath {
public:
	RegisterConfigPath() {}
	static std::vector<std::string> GetConfigPath(const std::vector<size_t> indices);
	static std::vector<std::string> GetConfigPath(Operator op);

private:
	static inline size_t m_currentIdx = 0;
	static inline std::vector<std::string> m_configPaths{
		"./config/gs_ply.json",
		"./config/gs_splat.json",
		"./config/box.json",
		"./config/ellipsoid.json",
		"./config/map.json",
		"./config/rectangle2d.json",
		"./config/sphere.json"
	};
};

REGISTER_END