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

class RenderMain : public std::enable_shared_from_this<RenderMain> {
public:
	static std::shared_ptr<RenderMain> GetInstance();
	RenderMain();
	~RenderMain() {};
	GLFWwindow* GetWindow() { return m_window; }
	void SetupRenderObjs(std::vector<std::string>& config_paths);
	void PrepareDraw();
	void Draw();
	void FinishDraw();

private:
	class UniformSetter
	{
	public:
		static void SetProjectionUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
		static void SetViewUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
		static void SetModelUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
		static void SetCamPosUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
		static void SetViewportUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
		static void SetFocalUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
		static void SetTanFovUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
		static void SetProjParamsUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain);
	};

	void SetUpDrawUniform(std::unordered_map<std::string, std::any>& draw_uniforms);
	void GatherConfigUniform();

private:
	std::shared_ptr<GLFWManager> m_glfw_instance = nullptr;
	std::shared_ptr<Camera> m_camera = nullptr;
	std::shared_ptr<RenderObjectManager> m_render_obj_mgr = nullptr;
	std::shared_ptr<ImGuiManager> m_imgui_mgr = nullptr;
	GLFWwindow* m_window = nullptr;
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>> m_render_objs;
	std::vector<std::shared_ptr<Parser::RenderObjConfigBase>> m_render_obj_configs;
	std::unordered_set<std::string> m_render_obj_uniforms;
	static std::shared_ptr<RenderMain> m_instance;
	float m_last_frame = 0.0;
	float m_delta_time = 0.0;
	using SetterFuncPtr = std::function<void(std::unordered_map<std::string, std::any>&, const std::shared_ptr<RenderMain>)>;
	std::unordered_map<std::string, SetterFuncPtr> m_uniform_setter = {
		{"projection", UniformSetter::SetProjectionUniform},
		{"view", UniformSetter::SetViewUniform},
		{"model", UniformSetter::SetModelUniform},
		{"cam_pos", UniformSetter::SetCamPosUniform},
		{"viewport", UniformSetter::SetViewportUniform},
		{"focal", UniformSetter::SetFocalUniform},
		{"tan_fov", UniformSetter::SetTanFovUniform},
		{"projParams", UniformSetter::SetProjParamsUniform}
	};
};