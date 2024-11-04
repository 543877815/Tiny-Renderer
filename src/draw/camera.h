#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>


// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	YAW_DONW,
	YAW_UP,
	PITCH_LEFT,
	PITCH_RIGHT
};

enum CameraProjection {
	PERSPECTIVE,
	ORTHOGONAL
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
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
	CameraProjection GetCameraProjection() const { return m_camera_projection; }
	void UpdateViewMatrix();
	void UpdateOrthogonalProjectionMatrix();
	void UpdatePerspectiveProjectionMatrix();
	void SetPosition(glm::vec3 position) { m_position = position; }
	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(CameraMovement direction, float deltaTime);
	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset);
	void RenderController();

private:
	void updateCameraVectors(); // calculates the front vector from the Camera's (updated) Euler Angles

private:
	// camera Attributes
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_front_vec = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_up_vec = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_right_vec = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_worldUp_vec = glm::vec3(0.0f, 1.0f, 0.0f);
	// euler Angles
	float m_yaw = -90.0f;
	float m_pitch = 0.0f;
	// camera options
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
	// camera matrix
	glm::mat4 m_viewMatrix;
	glm::mat4 m_prospectiveProjectionMatrix;
	glm::mat4 m_orthogonalProjectionMatrix;
	CameraProjection m_camera_projection = CameraProjection::PERSPECTIVE;
	// singleton
	static std::shared_ptr<Camera> m_instance;
};

