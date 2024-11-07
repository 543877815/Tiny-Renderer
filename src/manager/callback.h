#pragma once
#include "glfw_mgr.h"
#include "../draw/camera.h"

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);


// https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
void ProcessInput(GLFWwindow* window, float deltaTime);
void ProcessModelMatrix(GLFWwindow* window, glm::mat4& modelMatrix);


/**
 * Note: This method is deprecated. Use the ProcessInput() instead
 * Origin from: https://github.com/antimatter15/splat
 */
/* deprecated */void ProcessViewCamera(GLFWwindow* window, const int& SCR_WIDTH, const int& SCR_HEIGHT);
/* deprecated */void ProcessViewWorld(GLFWwindow* window);
/* deprecated */void translate4(glm::mat4& matrix, float x, float y, float z);
/* deprecated */void rotate4(glm::mat4& matrix, float rad, float x, float y, float z);
