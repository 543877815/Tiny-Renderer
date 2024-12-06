#include "render_main.h"
#include "../manager/callback.h"
#include "../render_objs/gs_ply_obj.h"
#include "../register/register_uniform_setter.h"

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
	m_glfwInstance = GLFWManager::GetInstance(SCR_WIDTH, SCR_HEIGHT);
	m_glfwInstance->SetFrameBufferSizeCallback(FramebufferSizeCallback);
	m_glfwInstance->SetMouseButtonCallback(MouseButtonCallback);
	m_glfwInstance->SetMouseCallback(MouseCallback);
	m_glfwInstance->SetScrollCallback(ScrollCallback);
	m_glfwInstance->SetKeyCallback(KeyCallback);

	m_camera = Camera::GetInstance();
	m_camera->ProcessFramebufferSizeCallback(SCR_WIDTH, SCR_HEIGHT);
	m_window = m_glfwInstance->GetWindow();
	m_renderObjMgr = RenderObjectManager::GetInstance();
	m_imguiMgr = ImGuiManager::GetInstance(m_window);
	m_uniformSetter = std::make_unique<Registry::UniformSetter>();
	//m_fbo = std::make_shared<Renderable::GSFrameBufferObj>("./shader/gs_fbo_vs.glsl", "./shader/gs_fbo_fs.glsl"); // to be packed
}

void RenderMain::SetupRenderObjs(std::vector<std::string>& configPaths)
{
	m_renderObjMgr->InitRenderObjs(configPaths);
	m_renderObjs = m_renderObjMgr->GetRenderObjs();
	m_renderObjConfigs = m_renderObjMgr->GetObjConfigs();
	GatherConfigUniform();
}

void RenderMain::PrepareDraw()
{
	float* clearColor = m_imguiMgr->GetClearColor();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

	float currentFrame = static_cast<float>(glfwGetTime());
	m_delta_time = currentFrame - m_lastFrame;
	m_lastFrame = currentFrame;
	ProcessInput(m_window, m_delta_time);
	SetUpDrawUniform();
	// mvp transform
	//processViewWorld(m_window);
	//processViewCamera(m_window, SCR_WIDTH, SCR_HEIGHT);
	//processModelMatrix(m_window, model);
}

void RenderMain::SetUpDrawUniform()
{
	for (const auto& uniform : m_renderObjUniforms) {
		if (m_uniformSetter->Contain(uniform)) {
			m_uniformSetter->GetFunc(uniform)(m_drawUniforms);
		}
	}
}

void RenderMain::GatherConfigUniform()
{
	m_renderObjUniforms.clear();
	for (const auto& ConfigPtr : m_renderObjConfigs)
	{
		auto& configUniform = ConfigPtr->GetUniform();
		m_renderObjUniforms.insert(configUniform.begin(), configUniform.end());
	}
}

void RenderMain::Draw()
{
	glm::mat4 model = glm::mat4(1.0f);

	std::vector<std::function<void()>> functions;
	for (size_t i = 0; i < m_renderObjs.size(); i++) {
		auto& renderObj = m_renderObjs[i];
		auto& config = m_renderObjConfigs[i];
		renderObj->DrawObj(m_drawUniforms);
		// ImGUI Callback
		auto callback = [&renderObj]() {
			renderObj->ImGuiCallback();
			};
		functions.emplace_back(callback);
	}
	m_imguiMgr->Render(functions);
}

void RenderMain::FinishDraw()
{
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}


