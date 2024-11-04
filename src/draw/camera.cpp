#include "camera.h"
#include "../parser/config.h"
#include <mutex>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

extern const int SCR_WIDTH;
extern const int SCR_HEIGHT;

std::shared_ptr<Camera> Camera::m_instance = nullptr;

void Camera::updateCameraVectors() {
	// calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front_vec = glm::normalize(front);
	// also re-calculate the Right and Up vector
	m_right_vec = glm::normalize(glm::cross(m_front_vec, m_worldUp_vec));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_up_vec = glm::normalize(glm::cross(m_right_vec, m_front_vec));
}

void Camera::ProcessMouseScroll(float yoffset)
{
	m_fov -= (float)yoffset;
	if (m_fov < 1.0f)
		m_fov = 1.0f;
	if (m_fov > 135.0f)
		m_fov = 135.0f;
}

void Camera::RenderController()
{
	static bool isFolded = true;
	if (ImGui::CollapsingHeader("Camera", &isFolded, ImGuiTreeNodeFlags_DefaultOpen))  // default open
	{

		static int selected_option = 0;
		if (ImGui::RadioButton("Perspective Projection", &selected_option, 0))
		{
			m_camera_projection = PERSPECTIVE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Orthogonal Projection", &selected_option, 1))
		{
			m_camera_projection = ORTHOGONAL;
		}

		if (ImGui::SliderFloat3("Position", glm::value_ptr(m_position), -10.0f, 10.0f))
		{
			UpdateViewMatrix();
		}

		if (m_camera_projection == PERSPECTIVE && ImGui::SliderFloat("Fov", &m_fov, 15.0f, 150.0f))
		{
			UpdatePerspectiveProjectionMatrix();
		}

		if (ImGui::SliderFloat("Near Plain", &m_near, 0.1, 5.0f))
		{
			if (m_camera_projection == PERSPECTIVE)
				UpdatePerspectiveProjectionMatrix();
			else
				UpdateOrthogonalProjectionMatrix();
		}

		if (ImGui::SliderFloat("Far Plain", &m_far, 1000.0f, 2000.f))
		{
			if (m_camera_projection == PERSPECTIVE)
				UpdatePerspectiveProjectionMatrix();
			else
				UpdateOrthogonalProjectionMatrix();
		}

		if (m_camera_projection == ORTHOGONAL && ImGui::SliderFloat("Left Plain", &m_left, -1.0f, -1.4f))
		{
			UpdateOrthogonalProjectionMatrix();
		}

		if (m_camera_projection == ORTHOGONAL && ImGui::SliderFloat("Right Plain", &m_right, 1.0f, 1.4f))
		{
			UpdateOrthogonalProjectionMatrix();
		}

		if (m_camera_projection == ORTHOGONAL && ImGui::SliderFloat("Top Plain", &m_top, 1.0f, 1.4f))
		{
			UpdateOrthogonalProjectionMatrix();
		}

		if (m_camera_projection == ORTHOGONAL && ImGui::SliderFloat("Bottom Plain", &m_bottom, -1.0f, -1.4f))
		{
			UpdateOrthogonalProjectionMatrix();
		}
	}
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= m_mouseSensitivity;
	yoffset *= m_mouseSensitivity;

	m_yaw += xoffset;
	m_pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;
	}

	// update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

std::shared_ptr<Camera> Camera::GetInstance()
{
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);

	if (m_instance == nullptr) {
		m_instance = std::make_shared<Camera>();
	}
	return m_instance;
}

Camera::Camera(glm::vec3 position, glm::vec3 up_vec)
{
	updateCameraVectors();
	UpdateViewMatrix();
	UpdateOrthogonalProjectionMatrix();
	UpdatePerspectiveProjectionMatrix();
}

void Camera::UpdateViewMatrix()
{
	m_viewMatrix = glm::lookAt(m_position, m_position + m_front_vec, m_up_vec);
}

void Camera::UpdateOrthogonalProjectionMatrix()
{
	m_orthogonalProjectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
}

void Camera::UpdatePerspectiveProjectionMatrix()
{
	m_prospectiveProjectionMatrix = glm::perspective(glm::radians(m_fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, m_near, m_far);
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	float velocity = m_movementSpeed * deltaTime;
	if (direction == FORWARD)
		m_position += m_front_vec * velocity;
	if (direction == BACKWARD)
		m_position -= m_front_vec * velocity;
	if (direction == LEFT)
		m_position -= m_right_vec * velocity;
	if (direction == RIGHT)
		m_position += m_right_vec * velocity;
	if (direction == UP)
		m_position += m_up_vec * velocity;
	if (direction == DOWN)
		m_position -= m_up_vec * velocity;
	if (direction == YAW_DONW || direction == YAW_UP) {
		if (direction == YAW_DONW) {
			m_yaw -= 0.001f;
		}
		if (direction == YAW_UP) {
			m_yaw += 0.001f;
		}

	}

	m_viewMatrix = glm::lookAt(m_position, m_position + m_front_vec, m_up_vec);
}