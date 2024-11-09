#include "config_parser.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

PARSER_BEGIN
bool GetOFNPath(TCHAR* szFile, bool isSave)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = TEXT("Text Files\0*.TXT\0All Files\0*.*\0");
	ofn.lpstrFile = szFile;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = TEXT("txt\0json");
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	if (isSave) {
		return GetSaveFileName(&ofn);
	}
	else {
		return GetOpenFileName(&ofn);
	}
}

void LoadCameraConfig()
{
	TCHAR szFile[MAX_PATH] = { 0 };
	if (GetOFNPath(szFile, false)) {
		std::ifstream file;
		file.open(szFile, std::ifstream::in);
		std::wstring ws(szFile);
		if (file.fail()) {
			std::wcout << "Unable to open file: " << szFile << std::endl;
			return;
		}

		file.close();
	}
}

void ParseRenderObjConfig(const std::string& path, std::vector<Parser::RenderObjConfig>& obj_configs)
{
	std::ifstream ifs(path);
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	rapidjson::Document document;
	document.Parse(buffer.str().c_str());
	if (document.HasParseError()) {
		std::cerr << "JSON parse error: " << document.GetParseError() << std::endl;
		return;
	}

	if (!document.IsArray()) {
		std::cerr << "JSON is not an array." << std::endl;
		return;
	}

	for (const auto& elem : document.GetArray()) {
		Parser::RenderObjConfig config;
		if (!elem.IsObject()) {
			std::cerr << "Element is not an object." << std::endl;
			continue;
		}

		const rapidjson::Value& obj_info = elem;

		config.obj_type = obj_info["object"].GetString();
		config.vertex_shader = obj_info["vertex_shader"].GetString();
		config.fragment_shader = obj_info["fragment_shader"].GetString();
		if (obj_info.HasMember("projection") && obj_info["projection"].IsString()) {
			config.projection = obj_info["projection"].GetString();
		}
		else {
			config.projection = "perspective";
		}

		const rapidjson::Value& uniform_info = obj_info["uniform"];
		if (uniform_info.IsArray()) {
			for (const auto& uniform : uniform_info.GetArray()) {
				config.uniform.insert(uniform.GetString());
			}
		}
		else {
			std::cerr << "Uniform is not an array." << std::endl;
		}

		obj_configs.emplace_back(config);
	}
}

PARSER_END
