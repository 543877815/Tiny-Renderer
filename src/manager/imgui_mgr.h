#pragma once
#include <unordered_map>
#include <iostream>
#include <assert.h>

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <functional>
#include <mutex>
#include <memory>

class ImGuiManager {
public:
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;
	static std::shared_ptr<ImGuiManager> GetInstance(GLFWwindow* window = nullptr);
	float* GetClearColor() { return m_clear_color; }
	void Render(std::vector <std::function<void()>>& func);
	explicit ImGuiManager(GLFWwindow* window);
	~ImGuiManager();

private:
	static std::shared_ptr<ImGuiManager> instance;
	GLFWwindow* m_window = nullptr;
	float m_clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};


