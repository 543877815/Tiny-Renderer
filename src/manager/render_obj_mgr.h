#pragma once
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "../register/register_render_obj.h"
#include <format>
#include <unordered_set>
class RenderObjectManager {
public:
	struct RenderObjConfig {
		std::string obj_type;
		std::string vertex_shader;
		std::string fragment_shader;
		std::unordered_set<std::string> uniform;
		std::string projection;
	};

	RenderObjectManager();
	RenderObjectManager(const RenderObjectManager&) = delete;
	RenderObjectManager& operator=(const RenderObjectManager&) = delete;
	std::vector<RenderObjConfig>& GetObjConfigs();
	RenderObjConfig& GetObjConfig(size_t idx);
	std::vector<std::shared_ptr<renderable::RenderObjectBase>>& GetRenderObjs();
	static std::shared_ptr<RenderObjectManager> GetInstance();
	void InitRenderObjs(std::vector<std::string>& configs);
	void InitRenderObj(const std::string& config);

private:
	void ParseCameraConfig(const std::string& path);

private:
	static std::shared_ptr<RenderObjectManager> instance;
	std::vector<RenderObjConfig> obj_configs;
	std::unordered_map<std::string, std::function<std::shared_ptr<renderable::RenderObjectBase>()>> register_render_obj;
	std::vector<std::shared_ptr<renderable::RenderObjectBase>> render_objs;
};

