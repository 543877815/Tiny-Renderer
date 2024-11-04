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
		m_glfw_instance->SetFrameBufferSizeCallback(framebufferSizeCallback);
		m_glfw_instance->SetMouseButtonCallback(mouseButtonCallback);
		m_glfw_instance->SetMouseCallback(mouseCallback);
		m_glfw_instance->SetScrollCallback(scrollCallback);
		m_glfw_instance->SetKeyCallback(keyCallback);

		m_camera = Camera::GetInstance();
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
		float deltaTime = currentFrame - m_last_frame;
		m_last_frame = currentFrame;
	}

	void SetUpUniform(std::unordered_map<std::string, std::any>& uniform, const RenderObjectManager::RenderObjConfig& config) {
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4& view = m_camera->GetViewMatrix();


		if (config.uniform.count("projection")) {
			if (config.projection == "orthogonal") {
				uniform.emplace("projection", m_camera->GetOrthogonalProjectionMatrix());
			}
			else {
				uniform.emplace("projection", m_camera->GetPerspectiveProjectionMatrix());
			}
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

		// mvp transform
		//processInput(window, camera, deltaTime);
		//processViewWorld(window, camera);

		processViewCamera(m_window, SCR_WIDTH, SCR_HEIGHT);
		processModelMatrix(m_window, model);
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
	std::vector<std::shared_ptr<renderable::RenderObjectBase>> m_render_objs;
	std::vector<RenderObjectManager::RenderObjConfig> m_render_obj_configs;
	static std::shared_ptr<RenderMain> m_instance;
	float m_last_frame = 0;
};

std::shared_ptr<RenderMain> RenderMain::m_instance = nullptr;