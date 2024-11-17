#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>




// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum CameraKeyboard {
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

	enum CameraMouseButton {
		MOUSE_BUTTON_RIGHT,
		MOUSE_BUTTON_LEFT
	};

	enum CameraMouseAction {
		MOUSE_RELEASE,
		MOUSE_PRESS
	};

	enum CameraMods {
		NONE,
		MOUSE_MOD_SHIFT,
		MOUSE_MOD_CONTROL,
		MOUSE_MOD_ALT,
		MOUSE_MOD_SUPER
	};

	enum CameraProjection {
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

	void SetViewMatrix(glm::mat4 mat) { m_viewMatrix = mat; }
	const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
	const glm::mat4& GetOrthogonalProjectionMatrix() const { return m_orthogonalProjectionMatrix; }
	const glm::mat4& GetPerspectiveProjectionMatrix() const { return m_prospectiveProjectionMatrix; }
	const glm::mat4& GetProjectionMatrix() const { return m_camera_projection == PERSPECTIVE ? m_prospectiveProjectionMatrix : m_orthogonalProjectionMatrix; }
	const glm::vec3& GetPosition() const { return m_position; }
	float GetFov() const { return m_fov; }
	float GetNear() const { return m_near; }
	float GetFar() const { return m_far; }
	float GetLeft() const { return m_left; }
	float GetRight() const { return m_right; }
	float GetTop() const { return m_top; }
	float GetBottom() const { return m_bottom; }
	float GetFx() const { return m_fx; }
	float GetFy() const { return m_fy; }
	float GetWidth() const { return m_width; }
	float GetHeight() const { return m_height; }
	int GetScreenWidth()const { return m_screen_width; }
	int GetScreenHeight()const { return m_screen_height; }
	CameraProjection GetCameraProjection() const { return m_camera_projection; }
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
	void InitializeCameraVectors(); // calculates the front vector from the Camera's (updated) Euler Angles
	void translate4(glm::mat4& matrix, float x, float y, float z);
private:
	// camera Attributes
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_front_vec = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_up_vec = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_right_vec = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_worldUp_vec = glm::vec3(0.0f, 1.0f, 0.0f);
	bool m_spherical_surface_rotation = true;
	// euler Angles
	float m_yaw = -90.0f;
	float m_pitch = 0.0f;
	float m_roll = 0.0f;
	// camera options
	float m_rotation_speed = 2.0f;
	float m_transition_speed = 2.0f;
	float m_movementSpeed = 2.5f;
	float m_mouseSensitivity = 0.1f;;
	// projection relative
	float m_fov = 45.0f;
	float m_near = 0.1f;
	float m_far = 100.0f;
	float m_left = -1.2f;
	float m_right = 1.2f;
	float m_bottom = -1.2f;
	float m_top = 1.2f;
	// speed
	float m_view_transition_speed = 0.001f;
	float m_view_rotation_speed = 0.005f;
	float m_model_transition_speed = 0.001f;
	float m_model_rotation_speed = 0.005f;
	// screen
	int m_screen_width = 800;
	int m_screen_height = 600;
	float m_screen_aspect_rate = (float)m_screen_width / (float)m_screen_height;
	// 3dgs
	float m_fy = 1164.660128748450f;  // focal_y
	float m_fx = 1159.5880733038064f; // focal_x 
	float m_width = 1959.0f;
	float m_height = 1090.0f;
	// camera matrix
	glm::mat4 m_viewMatrix;
	glm::mat4 m_prospectiveProjectionMatrix;
	glm::mat4 m_orthogonalProjectionMatrix;
	CameraProjection m_camera_projection = CameraProjection::PERSPECTIVE;
	// mouse
	float m_mouse_last_pos_x = 0.0f;
	float m_mouse_last_pos_y = 0.0f;
	float m_mouse_pressed = false;
	// singleton
	static std::shared_ptr<Camera> m_instance;
};

