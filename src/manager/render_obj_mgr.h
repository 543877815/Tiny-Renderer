#pragma once
#include <memory>

#include <format>
#include <unordered_set>
#include "../register/register_render_obj.h"
class RenderObjectManager {
public:

	RenderObjectManager();
	RenderObjectManager(const RenderObjectManager&) = delete;
	RenderObjectManager& operator=(const RenderObjectManager&) = delete;
	std::vector<std::shared_ptr<Parser::RenderObjConfigBase>>& GetObjConfigs();
	std::shared_ptr<Parser::RenderObjConfigBase>& GetObjConfig(size_t idx);
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>>& GetRenderObjs();
	static std::shared_ptr<RenderObjectManager> GetInstance();
	void InitRenderObjs(std::vector<std::string>& config_paths);
	void ResetRenderObjs();

private:
	static std::shared_ptr<RenderObjectManager> m_instance;

	std::vector<std::shared_ptr<Parser::RenderObjConfigBase>> m_obj_configs;
	std::unordered_map<std::string, Registry::CreateRenderObjFuncPtr> m_register_render_obj;
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>> m_render_objs;
};

