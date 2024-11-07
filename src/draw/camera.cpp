#include "camera.h"
#include "../parser/config.h"
#include "../utils/utils.h"
#include <mutex>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

std::shared_ptr<Camera> Camera::m_instance = nullptr;

// 将四元数转换为旋转矩阵
//glm::mat3 quaternionToRotationMatrix(const glm::quat& q) {
//	glm::mat3 rotationMatrix;
//	rotationMatrix[0][0] = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
//	rotationMatrix[0][1] = 2 * q.x * q.y - 2 * q.w * q.z;
//	rotationMatrix[0][2] = 2 * q.x * q.z + 2 * q.w * q.y;
//	rotationMatrix[1][0] = 2 * q.x * q.y + 2 * q.w * q.z;
//	rotationMatrix[1][1] = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
//	rotationMatrix[1][2] = 2 * q.y * q.z - 2 * q.w * q.x;
//	rotationMatrix[2][0] = 2 * q.x * q.z - 2 * q.w * q.y;
//	rotationMatrix[2][1] = 2 * q.y * q.z + 2 * q.w * q.x;
//	rotationMatrix[2][2] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
//	return rotationMatrix;
//}
//
//// 从旋转矩阵中提取Yaw角度
//float extractYaw(const glm::mat3& rotationMatrix) {
//	return std::atan2(rotationMatrix[1][0], rotationMatrix[0][0]);
//}
//
//// 从旋转矩阵中提取Pitch角度
//float extractPitch(const glm::mat3& rotationMatrix) {
//	return std::atan2(-rotationMatrix[2][0],
//		std::sqrt(rotationMatrix[2][1] * rotationMatrix[2][1] + rotationMatrix[2][2] * rotationMatrix[2][2]));
//}

glm::vec3 rotateVector(const glm::vec3& vector, const glm::quat& rotationQuaternion)
{
	// 将物体的位置转化为四元数形式
	glm::quat positionQuaternion(0.0f, vector.x, vector.y, vector.z);
	/// 用物体的旋转四元数旋转位置四元数
	glm::quat rotatedQuaternion = rotationQuaternion * positionQuaternion * glm::inverse(rotationQuaternion);
	/// 返回旋转后的向量
	return glm::vec3(rotatedQuaternion.x, rotatedQuaternion.y, rotatedQuaternion.z);
}

const double PI = 3.14159265358979323846;
const double rad2degree = 180.0f / PI;
void Camera::UpdateCameraRotationSphere(const glm::vec3& axis, float angle)
{
	// 将轴向量归一化
	glm::vec3 unitAxis = glm::normalize(axis);
	// 创建表示旋转的四元数
	glm::qua rotationQuaternion = glm::angleAxis(angle, unitAxis);
	// 将物体的位置转化为四元数形式
	if (m_facing_origin)
		m_position = rotateVector(m_position, rotationQuaternion);
	// 更新相机的朝向向量（假设front向量初始化为(0,0,-1)）
	m_front_vec = rotateVector(m_front_vec, rotationQuaternion);
	// 更新相机的上向量，确保它始终与朝向向量垂直
	m_up_vec = rotateVector(m_up_vec, rotationQuaternion);
	// 从四元数转欧拉角
#ifdef GLM_ENABLE_EXPERIMENTAL
	glm::vec3 eulerAngles = glm::eulerAngles(rotationQuaternion);
#else
#error "Not implement error."
#endif // !GLM_ENABLE_EXPERIMENTAL
	// 获取pitch角（绕X轴旋转角度）
	m_pitch += eulerAngles.x * rad2degree;
	// 获取yaw角（绕Y轴旋转角度）
	m_yaw += eulerAngles.y * rad2degree;
	// 获取roll角（绕Z轴旋转角度）
	m_roll += eulerAngles.z * rad2degree;
	limitValueRange(m_yaw, -180.0f, 180.0f);
	limitValueRange(m_pitch, -180.0f, 180.0f);
}

void Camera::UpdateCameraVectors() {
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

void Camera::translate4(glm::mat4& matrix, float x, float y, float z)
{
	matrix = glm::translate(matrix, glm::vec3(x, y, z));
	m_position.x = matrix[3][0];
	m_position.y = matrix[3][1];
	m_position.z = matrix[3][2];
	m_facing_origin = false;
	UpdateViewMatrix();
}

void Camera::ProcessMouseScroll(float yoffset)
{
	m_fov -= (float)yoffset;
	if (m_fov < 1.0f)
		m_fov = 1.0f;
	if (m_fov > 135.0f)
		m_fov = 135.0f;
	UpdatePerspectiveProjectionMatrix();
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
		ImGui::SameLine();
		if (ImGui::Checkbox("Facing Origin", &m_facing_origin))
		{
			m_front_vec = -m_position;
			UpdateViewMatrix();
		}
		if (!m_facing_origin)
		{
			if (ImGui::SliderFloat3("Front Vector", glm::value_ptr(m_front_vec), -1.0f, 1.0f))
			{
				UpdateViewMatrix();
			}
		}

		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderFloat("Pitch", &m_pitch, -90.0f, 90.0f))
		{
			glm::quat quaternion = glm::quat(glm::vec3(m_pitch, m_yaw, m_roll));
			if (m_facing_origin)
				m_position = rotateVector(m_position, quaternion);
			// 更新相机的朝向向量（假设front向量初始化为(0,0,-1)）
			m_front_vec = rotateVector(m_front_vec, quaternion);
			// 更新相机的上向量，确保它始终与朝向向量垂直
			m_up_vec = rotateVector(m_up_vec, quaternion);
			UpdateViewMatrix();
		}
		ImGui::SameLine();
		if (ImGui::SliderFloat("Yaw", &m_yaw, -180.0f, 180.0f));
		ImGui::SameLine();
		if (ImGui::SliderFloat("Roll", &m_roll, -180.0f, 180.0f));
		ImGui::SameLine();
		ImGui::SliderFloat("Rotation Speed", &m_rotation_speed, 1.0f, 5.0f);
		ImGui::PopItemWidth();


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
	UpdateCameraVectors();
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
	UpdateCameraVectors();
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
	m_prospectiveProjectionMatrix = glm::perspective(glm::radians(m_fov), m_screen_aspect_rate, m_near, m_far);
}

void Camera::SetScreen(int width, int height)
{
	m_screen_height = height;
	m_screen_width = width;
	m_screen_aspect_rate = (float)m_screen_width / (float)m_screen_height;
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	glm::mat4 inverseViewMatrix = glm::inverse(m_viewMatrix);
	if (direction == SCREEN_MOVE_FORWARD)
		translate4(inverseViewMatrix, 0.0f, 0.0f, -m_transition_speed * deltaTime);
	else if (direction == SCREEN_MOVE_BACKWARD)
		translate4(inverseViewMatrix, 0.0f, 0.0f, m_transition_speed * deltaTime);
	else if (direction == SCREEN_MOVE_UP)
		translate4(inverseViewMatrix, 0.0f, m_transition_speed * deltaTime, 0.0f);
	else if (direction == SCREEN_MOVE_DOWN)
		translate4(inverseViewMatrix, 0.0f, -m_transition_speed * deltaTime, 0.0f);
	else if (direction == SCREEN_MOVE_LEFT)
		translate4(inverseViewMatrix, -m_transition_speed * deltaTime, 0.0f, 0.0f);
	else if (direction == SCREEN_MOVE_RIGHT)
		translate4(inverseViewMatrix, m_transition_speed * deltaTime, 0.0f, 0.0f);

	if (direction == CAMERA_ROTATE_LEFT)
		UpdateCameraRotationSphere(glm::vec3(0.0f, 1.0f, 0.0f), m_rotation_speed * deltaTime);
	else if (direction == CAMERA_ROTATE_RIGHT)
		UpdateCameraRotationSphere(glm::vec3(0.0f, 1.0f, 0.0f), -m_rotation_speed * deltaTime);
	else if (direction == CAMERA_ROTATE_CLOCKWISE)
		UpdateCameraRotationSphere(m_front_vec, m_rotation_speed * deltaTime);
	else if (direction == CAMERA_ROTATE_ANTICLOCKWISE)
		UpdateCameraRotationSphere(m_front_vec, -m_rotation_speed * deltaTime);
	else if (direction == CAMERA_ROTATE_UP || direction == CAMERA_ROTATE_DOWN)
	{
		glm::vec3 axis = glm::normalize(glm::cross(-m_position, m_up_vec));
		float upAngle = glm::acos(glm::dot(m_front_vec, m_worldUp_vec) / (glm::length(m_front_vec) * glm::length(m_worldUp_vec)));
		//float temp = 0.05f;
		if (direction == CAMERA_ROTATE_UP)
			UpdateCameraRotationSphere(axis, m_rotation_speed * deltaTime);
		if (direction == CAMERA_ROTATE_DOWN)
			UpdateCameraRotationSphere(axis, -m_rotation_speed * deltaTime);
	}

	UpdateViewMatrix();
}