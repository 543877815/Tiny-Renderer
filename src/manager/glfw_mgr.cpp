#include "glfw_mgr.h"

void DefaultFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void DefaultMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	std::cout << std::format("Mouse position: ({}, {})", xpos, ypos) << std::endl;
}

void DefaultScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	std::cout << std::format("Mouse scroll: ({}, {})", xoffset, yoffset) << std::endl;
}

void DefaultMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		std::cout << "Left mouse button pressed" << std::endl;
	}
}

void DefaultKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}
}

std::shared_ptr<GLFWManager> GLFWManager::m_instance = nullptr;

std::shared_ptr<GLFWManager> GLFWManager::GetInstance(int screen_width = 0, int screen_height = 0)
{
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);

	if (m_instance == nullptr) {
		assert(screen_width, "screen width should not be zero when first init!");
		assert(screen_height, "screen height should not be zero when first init!");
		m_instance = std::make_shared<GLFWManager>(screen_width, screen_height);
	}
	return m_instance;
}
void GLFWManager::SetFrameBufferSizeCallback(void(*framebuffer_size_callback)(GLFWwindow*, int, int))
{
	assert(shared_from_this() == GetInstance());
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
}

void GLFWManager::SetMouseButtonCallback(void(*mouse_button_callback)(GLFWwindow*, int, int, int))
{
	assert(shared_from_this() == GetInstance());
	glfwSetMouseButtonCallback(m_window, mouse_button_callback);
}

void GLFWManager::SetMouseCallback(void(*mouse_callback)(GLFWwindow*, double, double))
{
	assert(shared_from_this() == GetInstance());
	glfwSetCursorPosCallback(m_window, mouse_callback);
}

void GLFWManager::SetScrollCallback(void(*scroll_callback)(GLFWwindow*, double, double))
{
	assert(shared_from_this() == GetInstance());
	glfwSetScrollCallback(m_window, scroll_callback);
}

void GLFWManager::SetKeyCallback(void(*key_callback)(GLFWwindow*, int, int, int, int))
{
	assert(shared_from_this() == GetInstance());
	glfwSetKeyCallback(m_window, key_callback);
}

GLFWwindow* GLFWManager::GetWindow()
{
	assert(shared_from_this() == GetInstance());
	return m_window;
}

GLFWManager::GLFWManager(int screen_width, int screen_height)
{
	// glfw: initialize and configure
// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
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

	// query limitations
// -----------------
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;
}

GLFWManager::~GLFWManager()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
	m_window = nullptr;
}
