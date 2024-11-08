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
#include "../register/register_config.h"

class RenderMain {
public:
	static std::shared_ptr<RenderMain> GetInstance() {
		static std::mutex mutex;
		std::lock_guard<std::mutex> lock(mutex);
		if (m_instance == nullptr) {
			m_instance = std::make_shared<RenderMain>();
		}
		return m_instance;
	}

	RenderMain() {
		m_glfw_instance = GLFWManager::GetInstance(SCR_WIDTH, SCR_HEIGHT);
		m_glfw_instance->SetFrameBufferSizeCallback(FramebufferSizeCallback);
		m_glfw_instance->SetMouseButtonCallback(MouseButtonCallback);
		m_glfw_instance->SetMouseCallback(MouseCallback);
		m_glfw_instance->SetScrollCallback(ScrollCallback);
		m_glfw_instance->SetKeyCallback(KeyCallback);

		m_camera = Camera::GetInstance();
		m_camera->SetScreen(SCR_WIDTH, SCR_HEIGHT);
		m_window = m_glfw_instance->GetWindow();
		m_render_obj_mgr = RenderObjectManager::GetInstance();
		m_imgui_mgr = ImGuiManager::GetInstance(m_window);



	}

	~RenderMain() {};

	GLFWwindow* GetWindow() {
		return m_window;
	}

	void SetRenderObjs(std::vector<std::string>& configs) {
		m_render_obj_mgr->InitRenderObjs(configs);
		m_render_objs = m_render_obj_mgr->GetRenderObjs();
		m_render_obj_configs = m_render_obj_mgr->GetObjConfigs();
	}

	void PrepareDraw() {
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

	void SetUpUniform(std::unordered_map<std::string, std::any>& uniform, const Registry::RenderObjConfig& config) {
		glm::mat4 model = glm::mat4(1.0f);
		const glm::mat4& view = m_camera->GetViewMatrix();
		const glm::mat4& projection = m_camera->GetProjectionMatrix();
		if (config.uniform.count("projection")) {
			uniform.emplace("projection", projection);
		}
		if (config.uniform.count("view")) {
			uniform.emplace("view", view);
		}
		if (config.uniform.count("model")) {
			uniform.emplace("model", model);
		}
	}

	void Draw() {
		glm::mat4 model = glm::mat4(1.0f);


		std::vector<std::function<void()>> functions;

		for (size_t i = 0; i < m_render_objs.size(); i++) {
			auto& render_obj = m_render_objs[i];
			auto& config = m_render_obj_configs[i];

			// Draw Call
			std::unordered_map<std::string, std::any> uniform;
			SetUpUniform(uniform, config);

			render_obj->DrawObj(uniform);
			// ImGUI Callback
			auto callback = [&render_obj]() {
				render_obj->ImGuiCallback();
				};
			functions.emplace_back(callback);
		}

		m_imgui_mgr->Render(functions);

	}

	void FinishDraw() {
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

private:
	std::shared_ptr<GLFWManager> m_glfw_instance = nullptr;
	std::shared_ptr<Camera> m_camera = nullptr;
	std::shared_ptr<RenderObjectManager> m_render_obj_mgr = nullptr;
	std::shared_ptr< ImGuiManager> m_imgui_mgr = nullptr;
	GLFWwindow* m_window = nullptr;
	std::vector<std::shared_ptr<Renderable::RenderObjectBase>> m_render_objs;
	std::vector<Registry::RenderObjConfig> m_render_obj_configs;
	static std::shared_ptr<RenderMain> m_instance;
	float m_last_frame = 0.0;
	float m_delta_time = 0.0;
};

std::shared_ptr<RenderMain> RenderMain::m_instance = nullptr;