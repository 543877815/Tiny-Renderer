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
	std::string objType;
	std::unordered_set<std::string> uniforms;
	std::unordered_set<std::string>& GetUniform() { return uniforms; };
};

struct RenderObjConfigSimple : public RenderObjConfigBase
{
	std::string type = "simple";
	std::string vertexShader = "";
	std::string fragmentShader = "";
	std::string projection = "perspective";
};

struct RenderObjConfig3DGS : public RenderObjConfigBase
{
	std::string type = "3dgs";
	std::string vertexShader = "";
	std::string fragmentShader = "";
	std::string modelPath = "";
	std::string fboVertexShader = "";
	std::string fboFragmentShader = "";
	std::string projection = "perspective";
};

struct RenderObjConfigAdvanced : public RenderObjConfigBase
{
	struct Drawing {
		std::string vertexShader;
		std::string fragmentShader;
		std::string modelPath;
		std::unordered_set<std::string> uniform;
		std::string projection = "perspective";
	};
	std::unordered_set<std::string> uniforms;
	std::string type = "multi";
	std::vector<Drawing> drawings;
	virtual std::unordered_set<std::string>& GetUniform() { return uniforms; };

};

static const char* configTypeKey = "configType";
static const char* objConfigKey = "objectInfo";
static const char* renderObjTypeKey = "type";
static const char* vertexShaderKey = "vertexShader";
static const char* fragmentShaderKey = "fragmentShader";
static const char* fboVertexShaderKey = "fboVertexShader";
static const char* fboFragmentShaderKey = "fboFragmentShader";
static const char* projectionTypeKey = "projection";
static const char* modelPathKey = "model_path";
static const char* uniformKey = "uniform";

class ConfigParser {
public:
	ConfigParser(std::vector<std::shared_ptr<RenderObjConfigBase>>& source) : m_objConfigs(source) {};
	void Parse(const std::string& path);
private:
	void ParseSimpleConfig(const rapidjson::Value& objConfig);
	void Parse3DGSConfig(const rapidjson::Value& objConfig);
	void CheckMemberExist(const rapidjson::Value& json, const char* key);
	void GetJsonString(const rapidjson::Value& json, const char* key, std::string& dest);
	void CheckJsonObject(const rapidjson::Value& json, const char* key);
	void CheckJsonArray(const rapidjson::Value& json, const char* key);

private:
	std::vector<std::shared_ptr<RenderObjConfigBase>>& m_objConfigs;
};

bool GetOFNPath(TCHAR* szFile, bool isSave);
void LoadCameraConfig();
PARSER_END


