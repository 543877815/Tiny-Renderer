#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <unordered_set>
#include "./common.h"

PARSER_BEGIN
struct RenderObjConfig {
	std::string obj_type;
	std::string vertex_shader;
	std::string fragment_shader;
	std::unordered_set<std::string> uniform;
	std::string projection;
};
bool GetOFNPath(TCHAR* szFile, bool isSave);
void LoadCameraConfig();
void ParseRenderObjConfig(const std::string& path, std::vector<Parser::RenderObjConfig>& obj_configs);
PARSER_END


