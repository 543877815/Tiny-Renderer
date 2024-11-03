#pragma once
#include "glfw_mgr.h"
#include "../draw/camera.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto camera = Camera::GetInstance();
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}
}

// https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
void processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
			camera.ProcessKeyboard(UP, deltaTime);
		}
		else {
			camera.ProcessKeyboard(FORWARD, deltaTime);
		}
	}

	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
			camera.ProcessKeyboard(DOWN, deltaTime);
		}
		else {
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		camera.ProcessKeyboard(YAW_UP, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		camera.ProcessKeyboard(YAW_DONW, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		camera.ProcessKeyboard(PITCH_LEFT, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		camera.ProcessKeyboard(PITCH_RIGHT, deltaTime);
	}

}

void processModelMatrix(GLFWwindow* window, glm::mat4& modelMatrix) {
	// modelMatrix
	glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f);
	float rotationSpeed = 0.1f;
	float angle = glm::radians(0.0f);

	auto set_rotation = [&](float _angle, glm::vec3 _axis) {
		angle = _angle;
		axis = _axis;
		};

	if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS) {
		set_rotation(glm::radians(-rotationSpeed), glm::vec3(1, 0, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS) {
		set_rotation(glm::radians(rotationSpeed), glm::vec3(1, 0, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS) {
		set_rotation(glm::radians(-rotationSpeed), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS) {
		set_rotation(glm::radians(rotationSpeed), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS) {
		set_rotation(glm::radians(-rotationSpeed), glm::vec3(0, 0, 1));
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS) {
		set_rotation(glm::radians(rotationSpeed), glm::vec3(0, 0, 1));
	}

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
	modelMatrix *= rotationMatrix;
}

void translate4(glm::mat4& matrix, float x, float y, float z) {
	matrix = glm::translate(matrix, glm::vec3(x, y, z));
}

void rotate4(glm::mat4& matrix, float rad, float x, float y, float z) {
	matrix = glm::rotate(matrix, glm::radians(rad), glm::vec3(x, y, z));
}

void processViewCamera(GLFWwindow* window, const int& SCR_WIDTH, const int& SCR_HEIGHT) {
	auto camera = Camera::GetInstance();
	float transitionSpeed = 0.001f;
	float rotationSpeed = 0.005f;
	static glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	static glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	static glm::vec3 Right = glm::normalize(glm::cross(Front, WorldUp));
	static glm::vec3 Up = glm::normalize(glm::cross(Right, Front));
	static float yaw = 0.0f; // 初始方位角
	static float pitch = 0.0f; // 初始极角

	// KEY
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
	bool KeyJ = glfwGetKey(window, GLFW_KEY_J);
	bool KeyK = glfwGetKey(window, GLFW_KEY_K);
	bool KeyL = glfwGetKey(window, GLFW_KEY_L);
	bool KeyI = glfwGetKey(window, GLFW_KEY_I);
	bool shift = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);

	glm::mat4 inverseMatrix = glm::inverse(camera->GetViewMatrix());
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (shift) {
			translate4(inverseMatrix, 0, -transitionSpeed, 0);
		}
		else {
			translate4(inverseMatrix, 0, 0, transitionSpeed);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (shift) {
			translate4(inverseMatrix, 0, transitionSpeed, 0);
		}
		else {
			translate4(inverseMatrix, 0, 0, -transitionSpeed);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		translate4(inverseMatrix, -transitionSpeed, 0, 0);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		translate4(inverseMatrix, transitionSpeed, 0, 0);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) rotate4(inverseMatrix, rotationSpeed, 0, 1, 0);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) rotate4(inverseMatrix, -rotationSpeed, 0, 1, 0);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) rotate4(inverseMatrix, rotationSpeed, 0, 0, 1);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) rotate4(inverseMatrix, -rotationSpeed, 0, 0, 1);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) rotate4(inverseMatrix, rotationSpeed, 1, 0, 0);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) rotate4(inverseMatrix, -rotationSpeed, 1, 0, 0);

	if (KeyJ || KeyK || KeyL || KeyI) {
		int d = 4;
		float dx = 20.0f / static_cast<float>(SCR_WIDTH);
		float dy = 20.0f / static_cast<float>(SCR_HEIGHT);
		translate4(inverseMatrix, 0, 0, d);
		if (KeyJ) rotate4(inverseMatrix, -dx, 0, 1, 0);
		if (KeyL) rotate4(inverseMatrix, dx, 0, 1, 0);
		if (KeyI) rotate4(inverseMatrix, dy, 1, 0, 0);
		if (KeyK) rotate4(inverseMatrix, -dy, 1, 0, 0);
		translate4(inverseMatrix, 0, 0, -d);
	}

	/****************** Mouse ************************/
	static bool isDragging = true;
	static double dx = 0;
	static double dy = 0;
	static double lastX = 0;
	static double lastY = 0;
	//ImVec2 windowPos = ImGui::GetWindowPos();
	//ImVec2 windowSize = ImGui::GetWindowSize();

	//ImVec2 rectMin = windowPos;
	//ImVec2 rectMax = ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y);
	//if (!ImGui::IsMouseHoveringRect(rectMin, rectMax) && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
	if (!isDragging) {
		isDragging = true;
		glfwGetCursorPos(window, &lastX, &lastY);
	}
	//}
	else if (isDragging) {
		isDragging = false;
		lastX = 0;
		lastY = 0;
	}

	if (isDragging) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		dx = (5 * (xpos - lastX)) / SCR_WIDTH;
		dy = (5 * (ypos - lastY)) / SCR_HEIGHT;
		int d = 4;
		translate4(inverseMatrix, 0, 0, d);
		rotate4(inverseMatrix, dx, 0, 1, 0);
		rotate4(inverseMatrix, -dy, 1, 0, 1);
		translate4(inverseMatrix, 0, 0, -d);

		lastX = xpos;
		lastY = ypos;
	}
	/********************************************/

	camera->SetViewMatrix(glm::inverse(inverseMatrix));

}

// https://github.com/antimatter15/splat
void processViewWorld(GLFWwindow* window, Camera& camera)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	glm::vec3 translateVector(0.0f);
	float translateSpeed = 0.002f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
			translateVector = glm::vec3(0.0f, -translateSpeed, 0.0f);
		}
		else {
			translateVector = glm::vec3(0.0f, 0.0f, translateSpeed);
		}
	}

	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
			translateVector = glm::vec3(0.0f, translateSpeed, 0.0f);
		}
		else {
			translateVector = glm::vec3(0.0f, 0.0f, -translateSpeed);
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		translateVector = glm::vec3(translateSpeed, 0.0f, 0.0f);

	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		translateVector = glm::vec3(-translateSpeed, 0.0f, 0.0f);
	}

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translateVector);
	camera.SetPosition(camera.GetPosition() + translateVector);
	camera.SetViewMatrix(camera.GetViewMatrix() * translationMatrix);

	glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f);
	float rotationSpeed = 0.1f;
	float angle = glm::radians(0.0f);

	auto set_rotation = [&](float _angle, glm::vec3 _axis) {
		angle = _angle;
		axis = _axis;
		};

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		set_rotation(glm::radians(-rotationSpeed), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		set_rotation(glm::radians(rotationSpeed), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		set_rotation(glm::radians(rotationSpeed), glm::vec3(0, 0, 1));
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		set_rotation(glm::radians(-rotationSpeed), glm::vec3(0, 0, 1));
	}
	else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		set_rotation(glm::radians(rotationSpeed), glm::vec3(1, 0, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		set_rotation(glm::radians(-rotationSpeed), glm::vec3(1, 0, 0));
	}

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
	camera.SetViewMatrix(camera.GetViewMatrix() * rotationMatrix);
}
