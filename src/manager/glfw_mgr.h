#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <mutex>
#include <iostream>
#include <memory>
#include <format>

void DefaultFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	// 设置视口大小以匹配新窗口尺寸
	glViewport(0, 0, width, height);
}

void DefaultMouseCallback(GLFWwindow* window, double xpos, double ypos) {
	std::cout << std::format("Mouse position: ({}, {})", xpos, ypos) << std::endl;
}

void DefaultScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	std::cout << std::format("Mouse scroll: ({}, {})", xoffset, yoffset) << std::endl;
}

void DefaultMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		std::cout << "Left mouse button pressed" << std::endl;
	}
}

void DefaultKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}
}

class GLFWManager : public std::enable_shared_from_this<GLFWManager> {
public:
	GLFWManager(const GLFWManager&) = delete;
	GLFWManager& operator=(const GLFWManager&) = delete;

	static std::shared_ptr<GLFWManager> GetInstance(int screen_width = 0, int screen_height = 0) {
		static std::mutex mutex;
		std::lock_guard<std::mutex> lock(mutex);

		if (m_instance == nullptr) {
			assert(screen_width, "screen width should not be zero when first init!");
			assert(screen_height, "screen height should not be zero when first init!");
			m_instance = std::make_shared<GLFWManager>(screen_width, screen_height);
		}
		return m_instance;
	}

	void SetFrameBufferSizeCallback(void (*framebuffer_size_callback)(GLFWwindow*, int, int)) {
		assert(shared_from_this() == GetInstance());
		glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	}

	void SetMouseButtonCallback(void (*mouse_button_callback)(GLFWwindow*, int, int, int)) {
		assert(shared_from_this() == GetInstance());
		glfwSetMouseButtonCallback(m_window, mouse_button_callback);
	}

	void SetMouseCallback(void (*mouse_callback)(GLFWwindow*, double, double)) {
		assert(shared_from_this() == GetInstance());
		glfwSetCursorPosCallback(m_window, mouse_callback);
	}

	void SetScrollCallback(void (*scroll_callback)(GLFWwindow*, double, double)) {
		assert(shared_from_this() == GetInstance());
		glfwSetScrollCallback(m_window, scroll_callback);
	}

	void SetKeyCallback(void (*key_callback)(GLFWwindow*, int, int, int, int)) {
		assert(shared_from_this() == GetInstance());
		glfwSetKeyCallback(m_window, key_callback);
	}

	GLFWwindow* GetWindow() {
		assert(shared_from_this() == GetInstance());
		return m_window;
	}


	explicit GLFWManager(int screen_width, int screen_height) {
		// glfw: initialize and configure
	// ------------------------------
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		// glfw window creation
		// --------------------
		m_window = glfwCreateWindow(screen_width, screen_height, "LearnOpenGL", NULL, NULL);
		if (m_window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(m_window);
		glfwSetFramebufferSizeCallback(m_window, DefaultFramebufferSizeCallback);
		glfwSetCursorPosCallback(m_window, DefaultMouseCallback);
		glfwSetScrollCallback(m_window, DefaultScrollCallback);
		glfwSetMouseButtonCallback(m_window, DefaultMouseButtonCallback);
		glfwSetKeyCallback(m_window, DefaultKeyCallback);

		// tell GLFW to capture our mouse
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// tell GLFW to show our mouse
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return;
		}
	}

	~GLFWManager() {
		glfwDestroyWindow(m_window);
		glfwTerminate();
		m_window = nullptr;
	}

private:
	static std::shared_ptr<GLFWManager> m_instance;
	GLFWwindow* m_window = nullptr;
};

std::shared_ptr<GLFWManager> GLFWManager::m_instance = nullptr;
