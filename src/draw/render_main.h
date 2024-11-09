#pragma once
#include <vector>
#include <memory>
#include <mutex>

#include "../manager/glfw_mgr.h"
#include "../manager/callback.h"
#include "../manager/imgui_mgr.h"
#include "../manager/render_obj_mgr.h"
#include "../parser/config.h"
#include "../draw/camera.h"

class RenderMain {
public:
	static std::shared_ptr<RenderMain> GetInstance();
	RenderMain();
	~RenderMain() {};
	GLFWwindow* GetWindow() { return m_window; }
	void SetRenderObjs(std::vector<std::string>& config_paths);
	void PrepareDraw();
	void Draw();
	void FinishDraw();

public:
	void SetUpUniform(std::unordered_map<std::string, std::any>& uniform, const Parser::RenderObjConfig& config);

private:
	std::shared_ptr<GLFWManager> m_glfw_instance = nullptr;
	std::shared_ptr<Camera> m_camera = nullptr;
	std::shared_ptr<RenderObjectManager> m_render_obj_mgr = nullptr;
	std::shared_ptr< ImGuiManager> m_imgui_mgr = nullptr;
	GLFWwindow* m_window = nullptr;
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>> m_render_objs;
	std::vector<Parser::RenderObjConfig> m_render_obj_configs;
	static std::shared_ptr<RenderMain> m_instance;
	float m_last_frame = 0.0;
	float m_delta_time = 0.0;
};