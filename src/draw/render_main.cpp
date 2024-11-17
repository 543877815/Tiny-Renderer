#include "render_main.h"
#include "../manager/callback.h"

std::shared_ptr<RenderMain> RenderMain::m_instance = nullptr;
std::shared_ptr<RenderMain> RenderMain::GetInstance()
{
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);
	if (m_instance == nullptr) {
		m_instance = std::make_shared<RenderMain>();
	}
	return m_instance;
}

RenderMain::RenderMain()
{
	m_glfw_instance = GLFWManager::GetInstance(SCR_WIDTH, SCR_HEIGHT);
	m_glfw_instance->SetFrameBufferSizeCallback(FramebufferSizeCallback);
	m_glfw_instance->SetMouseButtonCallback(MouseButtonCallback);
	m_glfw_instance->SetMouseCallback(MouseCallback);
	m_glfw_instance->SetScrollCallback(ScrollCallback);
	m_glfw_instance->SetKeyCallback(KeyCallback);

	m_camera = Camera::GetInstance();
	m_camera->ProcessFramebufferSizeCallback(SCR_WIDTH, SCR_HEIGHT);
	m_window = m_glfw_instance->GetWindow();
	m_render_obj_mgr = RenderObjectManager::GetInstance();
	m_imgui_mgr = ImGuiManager::GetInstance(m_window);
}

void RenderMain::SetupRenderObjs(std::vector<std::string>& config_paths)
{
	m_render_obj_mgr->InitRenderObjs(config_paths);
	m_render_objs = m_render_obj_mgr->GetRenderObjs();
	m_render_obj_configs = m_render_obj_mgr->GetObjConfigs();
	GatherConfigUniform();
}

void RenderMain::PrepareDraw()
{
	float* clear_color = m_imgui_mgr->GetClearColor();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

	float currentFrame = static_cast<float>(glfwGetTime());
	m_delta_time = currentFrame - m_last_frame;
	m_last_frame = currentFrame;
	ProcessInput(m_window, m_delta_time);

	// mvp transform
	//processViewWorld(m_window);
	//processViewCamera(m_window, SCR_WIDTH, SCR_HEIGHT);
	//processModelMatrix(m_window, model);
}

void RenderMain::SetUpDrawUniform(std::unordered_map<std::string, std::any>& draw_uniforms)
{
	for (const auto& uniform : m_render_obj_uniforms) {
		if (m_uniform_setter.contains(uniform)) {
			m_uniform_setter[uniform](draw_uniforms, shared_from_this());
		}
	}
}

void RenderMain::GatherConfigUniform()
{
	m_render_obj_uniforms.clear();
	for (const auto& config_ptr : m_render_obj_configs)
	{
		auto& config_uniform = config_ptr->GetUniform();
		m_render_obj_uniforms.insert(config_uniform.begin(), config_uniform.end());
	}
}

void RenderMain::Draw()
{
	glm::mat4 model = glm::mat4(1.0f);

	std::vector<std::function<void()>> functions;
	std::unordered_map<std::string, std::any> draw_uniforms;
	SetUpDrawUniform(draw_uniforms);
	for (size_t i = 0; i < m_render_objs.size(); i++) {
		auto& render_obj = m_render_objs[i];
		auto& config = m_render_obj_configs[i];
		render_obj->DrawObj(draw_uniforms);
		// ImGUI Callback
		auto callback = [&render_obj]() {
			render_obj->ImGuiCallback();
			};
		functions.emplace_back(callback);
	}

	m_imgui_mgr->Render(functions);

}

void RenderMain::FinishDraw()
{
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void RenderMain::UniformSetter::SetProjectionUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("projection", renderMain->m_camera->GetProjectionMatrix());
}

void RenderMain::UniformSetter::SetViewUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("view", renderMain->m_camera->GetViewMatrix());
}

void RenderMain::UniformSetter::SetModelUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("model", glm::mat4(1.0f));
}

void RenderMain::UniformSetter::SetCamPosUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("cam_pos", renderMain->m_camera->GetPosition());
}

void RenderMain::UniformSetter::SetViewportUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("viewport", glm::vec2(renderMain->m_camera->GetScreenWidth(), renderMain->m_camera->GetScreenHeight()));
}

void RenderMain::UniformSetter::SetFocalUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("focal", glm::vec2(renderMain->m_camera->GetFx(), renderMain->m_camera->GetFy()));
}

void RenderMain::UniformSetter::SetTanFovUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("tan_fov", glm::vec2(renderMain->m_camera->GetWidth() / renderMain->m_camera->GetFx() * 0.5f, renderMain->m_camera->GetHeight() / renderMain->m_camera->GetFy() * 0.5f));
}

void RenderMain::UniformSetter::SetProjParamsUniform(std::unordered_map<std::string, std::any>& uniforms, std::shared_ptr<RenderMain> renderMain)
{
	uniforms.emplace("projParams", glm::vec2(renderMain->m_camera->GetNear(), renderMain->m_camera->GetFar()));
}
