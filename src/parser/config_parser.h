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
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "./common.h"

PARSER_BEGIN
struct RenderObjConfigBase {
	std::string obj_type;
	virtual std::unordered_set<std::string>& GetUniform() = 0;
};

struct RenderObjConfigNaive : public RenderObjConfigBase
{
	std::string type = "naive";
	std::string vertex_shader;
	std::string fragment_shader;
	std::string model_path;
	std::unordered_set<std::string> uniforms;
	std::string projection = "perspective";
	virtual std::unordered_set<std::string>& GetUniform() { return uniforms; };
};

struct RenderObjConfigMulti : public RenderObjConfigBase
{
	struct Drawing {
		std::string vertex_shader;
		std::string fragment_shader;
		std::string model_path;
		std::unordered_set<std::string> uniform;
		std::string projection = "perspective";
	};
	std::unordered_set<std::string> uniforms;
	std::string type = "multi";
	std::vector<Drawing> drawings;
	virtual std::unordered_set<std::string>& GetUniform() { return uniforms; };

};

static const char* config_type_key = "config_type";
static const char* obj_config_key = "object_info";
static const char* renderobj_type_key = "type";
static const char* vertex_shader_key = "vertex_shader";
static const char* fragment_shader_key = "fragment_shader";
static const char* projection_type_key = "projection";
static const char* uniform_key = "uniform";

class ConfigParser {
public:
	ConfigParser(std::vector<std::shared_ptr<RenderObjConfigBase>>& source) : m_obj_configs(source) {};
	void Parse(const std::string& path);
private:
	void ParseNaiveConfig(const rapidjson::Value& obj_config);
	void CheckMemberExist(const rapidjson::Value& json, const char* key);
	void GetJsonString(const rapidjson::Value& json, const char* key, std::string& dest);
	void CheckJsonObject(const rapidjson::Value& json, const char* key);
	void CheckJsonArray(const rapidjson::Value& json, const char* key);

private:
	std::vector<std::shared_ptr<RenderObjConfigBase>>& m_obj_configs;
};

bool GetOFNPath(TCHAR* szFile, bool isSave);
void LoadCameraConfig();
PARSER_END


