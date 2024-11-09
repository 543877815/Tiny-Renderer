#include <mutex>
#include "render_obj_mgr.h"
#include "../parser/config_parser.h"

std::shared_ptr<RenderObjectManager> RenderObjectManager::m_instance = nullptr;
RenderObjectManager::RenderObjectManager()
{
	m_register_render_obj = Registry::RenderObjectFactory::GetRegisterRenderObj();
}

std::vector<Parser::RenderObjConfig>& RenderObjectManager::GetObjConfigs()
{
	return m_obj_configs;
}

Parser::RenderObjConfig& RenderObjectManager::GetObjConfig(size_t idx)
{
	return m_obj_configs[idx];
}

std::vector<std::shared_ptr<Renderable::RenderObjectBase>>& RenderObjectManager::GetRenderObjs()
{
	return m_render_objs;
}

std::shared_ptr<RenderObjectManager> RenderObjectManager::GetInstance()
{
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);
	if (m_instance == nullptr) {
		m_instance = std::make_shared<RenderObjectManager>();
	}
	return m_instance;
}

void RenderObjectManager::InitRenderObjs(std::vector<std::string>& config_paths) {
	for (auto path : config_paths) {
		Parser::ParseRenderObjConfig(path, m_obj_configs);
	}
	for (size_t i = 0; i < m_obj_configs.size(); i++) {
		auto& obj_config = m_obj_configs[i];
		auto& obj_type = obj_config.obj_type;
		if (m_register_render_obj.count(obj_type)) {
			m_render_objs.emplace_back(m_register_render_obj[obj_type](obj_config));
		}
		else {
			std::cerr << std::format("Error occur: the object: {} is not regisered yet.", obj_type) << std::endl;
		}
	}
}

