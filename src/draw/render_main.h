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
#include "../register/register_uniform_setter.h"

class RenderMain : public std::enable_shared_from_this<RenderMain> {
public:
	static std::shared_ptr<RenderMain> GetInstance();
	RenderMain();
	~RenderMain() {};
	GLFWwindow* GetWindow() { return m_window; }
	void SetupRenderObjs(std::vector<std::string>& configPaths);
	void PrepareDraw();
	void Draw();
	void FinishDraw();

private:

	void SetUpDrawUniform(std::unordered_map<std::string, std::any>& drawUniforms);
	void GatherConfigUniform();

private:
	std::shared_ptr<GLFWManager> m_glfwInstance = nullptr;
	std::shared_ptr<Camera> m_camera = nullptr;
	std::shared_ptr<RenderObjectManager> m_renderObjMgr = nullptr;
	std::shared_ptr<ImGuiManager> m_imguiMgr = nullptr;
	std::unique_ptr<Registry::UniformSetter> m_uniformSetter = nullptr;
	GLFWwindow* m_window = nullptr;
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>> m_renderObjs;
	std::vector<std::shared_ptr<Parser::RenderObjConfigBase>> m_renderObjConfigs;
	std::unordered_set<std::string> m_renderObjUniforms;
	static std::shared_ptr<RenderMain> m_instance;
	float m_lastFrame = 0.0;
	float m_delta_time = 0.0;

};