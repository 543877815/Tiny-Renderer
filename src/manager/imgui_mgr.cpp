#include "imgui_mgr.h"
#include <glm/gtc/type_ptr.hpp>

std::shared_ptr<ImGuiManager> ImGuiManager::instance = nullptr;

std::shared_ptr<ImGuiManager> ImGuiManager::GetInstance(GLFWwindow* window)
{
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);

	if (instance == nullptr) {
		assert(window, "Window should not be nullptr when first init!");
		instance = std::make_shared<ImGuiManager>(window);
	}
	return instance;
}

void ImGuiManager::CameraController()
{
	static bool isFolded = true;
	if (ImGui::CollapsingHeader("Camera", &isFolded, ImGuiTreeNodeFlags_DefaultOpen))  // default open
	{
		auto camera = Camera::GetInstance();
		if (ImGui::SliderFloat3("Position", glm::value_ptr(camera->GetPosition()), -10.0f, 10.0f))
		{
			camera->UpdateViewMatrix();
		}

		if (ImGui::SliderFloat("Fov", &camera->GetFov(), 15.0f, 150.0f))
		{
			camera->UpdatePerspectiveProjectionMatrix();
		}

		float& fov = camera->GetFov();
		if (ImGui::SliderFloat("Near Plain", &camera->GetNear(), 0.1, 5.0f))
		{
			camera->UpdatePerspectiveProjectionMatrix();
			camera->UpdateOrthogonalProjectionMatrix();
		}
		
		if (ImGui::SliderFloat("Far Plain", &camera->GetFar(), 1000.0f, 2000.f))
		{
			camera->UpdatePerspectiveProjectionMatrix();
			camera->UpdateOrthogonalProjectionMatrix();
		}

		if (ImGui::SliderFloat("Left Plain", &camera->GetLeft(), 1.0f, 1.4f))
		{
			camera->UpdateOrthogonalProjectionMatrix();
		}

		if (ImGui::SliderFloat("Right Plain", &camera->GetRight(), 1.0f, 1.4f))
		{
			camera->UpdateOrthogonalProjectionMatrix();
		}

		if (ImGui::SliderFloat("Top Plain", &camera->GetTop(), 1.0f, 1.4f))
		{
			camera->UpdateOrthogonalProjectionMatrix();
		}

		if (ImGui::SliderFloat("Bottom Plain", &camera->GetBottom(),1.0f, 1.4f))
		{
			camera->UpdateOrthogonalProjectionMatrix();
		}
	}

}

void ImGuiManager::Render(std::vector<std::function<void()>>& func)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("IMGUI CLI");
	ImGui::Text("Hello, ImGui with GLFW and OpenGL!");
	ImGui::SetNextItemWidth(150);
	ImGui::ColorPicker4("Background", m_clear_color, 
		ImGuiColorEditFlags_None
		| ImGuiColorEditFlags_NoInputs 
		| ImGuiColorEditFlags_AlphaBar 
		| ImGuiColorEditFlags_DisplayRGB
		| ImGuiColorEditFlags_NoSidePreview 
		| ImGuiColorEditFlags_NoAlpha);

	CameraController();
	for (auto f : func) {
		f();
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

ImGuiManager::ImGuiManager(GLFWwindow* window) : m_window(window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}


ImGuiManager::~ImGuiManager()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
