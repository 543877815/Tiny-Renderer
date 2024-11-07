#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <mutex>
#include <iostream>
#include <memory>
#include <format>

void DefaultFramebufferSizeCallback(GLFWwindow* window, int width, int height);
void DefaultMouseCallback(GLFWwindow* window, double xpos, double ypos);
void DefaultScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void DefaultMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void DefaultKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

class GLFWManager : public std::enable_shared_from_this<GLFWManager> {
public:
	GLFWManager(const GLFWManager&) = delete;
	GLFWManager& operator=(const GLFWManager&) = delete;
	static std::shared_ptr<GLFWManager> GetInstance(int screen_width, int screen_height);

	void SetFrameBufferSizeCallback(void (*framebuffer_size_callback)(GLFWwindow*, int, int));
	void SetMouseButtonCallback(void (*mouse_button_callback)(GLFWwindow*, int, int, int));
	void SetMouseCallback(void (*mouse_callback)(GLFWwindow*, double, double));
	void SetScrollCallback(void (*scroll_callback)(GLFWwindow*, double, double));
	void SetKeyCallback(void (*key_callback)(GLFWwindow*, int, int, int, int));
	GLFWwindow* GetWindow();
	explicit GLFWManager(int screen_width, int screen_height);
	~GLFWManager();

private:
	static std::shared_ptr<GLFWManager> m_instance;
	GLFWwindow* m_window = nullptr;
};

