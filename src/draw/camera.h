#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>




// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum CameraKeyboard : uint32_t {
		CAMERA_ROTATE_LEFT,
		CAMERA_ROTATE_RIGHT,
		CAMERA_ROTATE_UP,
		CAMERA_ROTATE_DOWN,
		CAMERA_ROTATE_CLOCKWISE,
		CAMERA_ROTATE_ANTICLOCKWISE,
		SCREEN_MOVE_LEFT,
		SCREEN_MOVE_RIGHT,
		SCREEN_MOVE_UP,
		SCREEN_MOVE_DOWN,
		SCREEN_MOVE_FORWARD,
		SCREEN_MOVE_BACKWARD
	};

	enum CameraMouseButton : uint32_t {
		MOUSE_BUTTON_RIGHT,
		MOUSE_BUTTON_LEFT
	};

	enum CameraMouseAction : uint32_t{
		MOUSE_RELEASE,
		MOUSE_PRESS
	};

	enum CameraMods : uint32_t {
		NONE,
		MOUSE_MOD_SHIFT,
		MOUSE_MOD_CONTROL,
		MOUSE_MOD_ALT,
		MOUSE_MOD_SUPER
	};

	enum CameraProjMethod : uint32_t {
		PERSPECTIVE,
		ORTHOGONAL
	};
	static std::shared_ptr<Camera> GetInstance();
	Camera() : Camera(m_position, m_up_vec) {}
	// constructor with vectors
	Camera(glm::vec3 position, glm::vec3 up_vec);
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ) :
		Camera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ)) {}

	void SetViewMatrix(glm::mat4 mat) { m_viewMat = mat; }
	const glm::mat4& GetViewMat() const { return m_viewMat; }
	const glm::mat4& GetOrthoProjMat() const { return m_orthoProjMat; }
	const glm::mat4& GetPerspProjMat() const { return m_prospProjMat; }
	const glm::mat4& GetProjMat() const { return m_cameraProjMethod == PERSPECTIVE ? m_prospProjMat : m_orthoProjMat; }
	const glm::vec3& GetPosition() const { return m_position; }
	float GetFov() const { return m_fov; }
	float GetNear() const { return m_near; }
	float GetFar() const { return m_far; }
	float GetLeft() const { return m_left; }
	float GetRight() const { return m_right; }
	float GetTop() const { return m_top; }
	float GetBottom() const { return m_bottom; }
	int GetScreenWidth()const { return m_screenWidth; }
	int GetScreenHeight()const { return m_screenHeight; }
	CameraProjMethod GetCameraProj() const { return m_cameraProjMethod; }
	void UpdateViewMatrix();
	void UpdateOrthogonalProjectionMatrix();
	void UpdatePerspectiveProjectionMatrix();
	void SetPosition(glm::vec3 position) { m_position = position; }
	void ProcessFramebufferSizeCallback(int width, int height);
	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(CameraKeyboard direction, float deltaTime);
	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseCallback(float xoffset, float yoffset);
	// processes input received from a mouse input system. Expects the button and action	.
	void ProcessMouseButtonCallback(CameraMouseButton button, CameraMouseAction action, CameraMods mods = CameraMods::NONE);
	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessScrollCallback(float xoffset, float yoffset);
	void RenderController();

private:
	void UpdateViewMatrix(glm::quat& quaternion);
	void UpdateCameraRotationSphere(const glm::vec3& axis, float angle);
	//void InitializeCameraVectors(); // calculates the front vector from the Camera's (updated) Euler Angles
	void translate4(glm::mat4& matrix, float x, float y, float z);
private:
	// camera Attributes
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_frontVec = -m_position;
	glm::vec3 m_worldUpVec = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_rightVec = glm::normalize(glm::cross(m_frontVec, m_worldUpVec));
	glm::vec3 m_up_vec = glm::normalize(glm::cross(m_rightVec, m_frontVec));
	bool m_sphericalSurfaceRotation = true;
	// euler Angles
	float m_yaw = -90.0f;
	float m_pitch = 0.0f;
	float m_roll = 0.0f;
	// camera options
	float m_rotationSpeed = 2.0f;
	float m_transitionSpeed = 2.0f;
	float m_movementSpeed = 2.5f;
	float m_mouseSensitivity = 0.1f;
	// projection relative
	float m_fov = 45.0f;
	float m_near = 0.1f;
	float m_far = 1000.0f;
	float m_left = -1.2f;
	float m_right = 1.2f;
	float m_bottom = -1.2f;
	float m_top = 1.2f;
	// speed
	float m_viewTransitionSpeed = 0.001f;
	float m_viewRotationSpeed = 0.005f;
	float m_modelTransitionSpeed = 0.001f;
	float m_modelRotationSpeed = 0.005f;
	// screen
	int m_screenWidth = 800;
	int m_screenHeight = 600;
	float m_screenAspectRate = (float)m_screenWidth / (float)m_screenHeight;
	// camera matrix
	glm::mat4 m_viewMat;
	glm::mat4 m_prospProjMat;
	glm::mat4 m_orthoProjMat;
	CameraProjMethod m_cameraProjMethod = CameraProjMethod::PERSPECTIVE;
	// mouse
	float m_mouse_last_pos_x = 0.0f;
	float m_mouse_last_pos_y = 0.0f;
	float m_mouse_pressed = false;
	// singleton
	static std::shared_ptr<Camera> m_instance;
	// imgui
	bool m_isFolded = true;
};

