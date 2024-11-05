#pragma once
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "../register/register_render_obj.h"
#include "../register/register_config.h"
#include <format>
#include <unordered_set>
class RenderObjectManager {
public:

	RenderObjectManager();
	RenderObjectManager(const RenderObjectManager&) = delete;
	RenderObjectManager& operator=(const RenderObjectManager&) = delete;
	std::vector<Registry::RenderObjConfig>& GetObjConfigs();
	Registry::RenderObjConfig& GetObjConfig(size_t idx);
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>>& GetRenderObjs();
	static std::shared_ptr<RenderObjectManager> GetInstance();
	void InitRenderObjs(std::vector<std::string>& configs);
	void InitRenderObj(const std::string& config);

private:
	void ParseCameraConfig(const std::string& path);

private:
	static std::shared_ptr<RenderObjectManager> instance;
	std::vector<Registry::RenderObjConfig> obj_configs;
	std::unordered_map<std::string, Registry::CreateRenderObjFuncPtr> register_render_obj;
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>> render_objs;
};

