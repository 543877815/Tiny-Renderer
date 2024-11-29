#include <glad/glad.h>
#include <format>
#include "utils.h"

void limitValueRange(float& value, float lowerLimit, float upperLimit)
{
	float range = upperLimit - lowerLimit;
	while (value > upperLimit) {
		value -= range;
	}
	while (value < lowerLimit) {
		value += range;
	}
}

void GLErrorCheck(const char* message)
{
	GLenum val = glGetError();
	switch (val)
	{
	case GL_INVALID_ENUM:
		std::cout << std::format("GL_INVALID_ENUM : %s\n", message);
		break;
	case GL_INVALID_VALUE:
		std::cout << std::format("GL_INVALID_VALUE : %s\n", message);
		break;
	case GL_INVALID_OPERATION:
		std::cout << std::format("GL_INVALID_OPERATION : %s\n", message);
		break;
#ifndef GL_ES_VERSION_2_0
	case GL_STACK_OVERFLOW:
		std::cout << std::format("GL_STACK_OVERFLOW : %s\n", message);
		break;
	case GL_STACK_UNDERFLOW:
		std::cout << std::format("GL_STACK_UNDERFLOW : %s\n", message);
		break;
#endif
	case GL_OUT_OF_MEMORY:
		std::cout << std::format("GL_OUT_OF_MEMORY : %s\n", message);
		break;
	case GL_NO_ERROR:
		break;
	}
}