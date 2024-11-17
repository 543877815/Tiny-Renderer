#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <exception>
#include <format>
#include "config_parser.h"

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

class FormatException : public std::exception {
private:
	std::string errorMessage;
public:
	FormatException(const std::string& message) : errorMessage(message) {}
	const char* what() const noexcept override {
		return errorMessage.c_str();
	}
};

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


void ConfigParser::Parse(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open())
		throw std::runtime_error(std::format("Error occured while opening {}", path));
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	ifs.close();
	rapidjson::Document document;
	document.Parse(buffer.str().c_str());
	if (document.HasParseError()) {
		throw std::runtime_error("Document parse error: " + document.GetParseError());
	}
	if (!document.IsArray()) {
		throw std::runtime_error("Document is not an array.");
	}

	auto configs = document.GetArray();
	for (const auto& config : configs)
	{
		if (!config.IsObject())
			throw FormatException("Element of document is not an object.");

		std::string config_type;
		GetJsonString(config, config_type_key, config_type);
		rapidjson::Value object_config_value;
		CheckJsonObject(config, obj_config_key);
		if (config_type == "naive")
		{
			ParseNaiveConfig(config[obj_config_key]);
		}
		else if (config_type == "multi")
		{

		}
		else
		{
			throw std::runtime_error(std::format("The configuration type {} is not implemented yet", config_type));
		}
	}
}

void ConfigParser::ParseNaiveConfig(const rapidjson::Value& obj_config)
{
	if (!obj_config.IsObject()) {
		throw FormatException("Json is not an object.");
	}
	RenderObjConfigNaive config;

	GetJsonString(obj_config, renderobj_type_key, config.obj_type);
	GetJsonString(obj_config, vertex_shader_key, config.vertex_shader);
	GetJsonString(obj_config, fragment_shader_key, config.fragment_shader);
	GetJsonString(obj_config, projection_type_key, config.projection);
	const rapidjson::Value& arr = obj_config[uniform_key];
	CheckJsonArray(obj_config, uniform_key);
	for (const auto& elem : arr.GetArray()) {
		auto uniform = elem.GetString();
		config.uniforms.insert(uniform);
	}
	m_obj_configs.emplace_back(std::make_shared<RenderObjConfigNaive>(config));
}

void ConfigParser::CheckMemberExist(const rapidjson::Value& json, const char* key)
{
	if (!json.HasMember(key))
		throw FormatException(std::format("Json does not contain a member of {}.", key));
}

void ConfigParser::CheckJsonObject(const rapidjson::Value& json, const char* key)
{
	CheckMemberExist(json, key);
	if (!json[key].IsObject())
		throw FormatException(std::format("The value of {} is not an object.", key));
}

void ConfigParser::GetJsonString(const rapidjson::Value& json, const char* key, std::string& dest)
{
	CheckMemberExist(json, key);
	if (!json[key].IsString())
		throw FormatException(std::format("The value of {} is not a string.", key));
	dest = json[key].GetString();
}

void ConfigParser::CheckJsonArray(const rapidjson::Value& json, const char* key)
{
	CheckMemberExist(json, key);
	const rapidjson::Value& arr = json[key];
	if (!arr.IsArray())
		throw FormatException(std::format("The value of {} is not an array.", key));
}

PARSER_END
