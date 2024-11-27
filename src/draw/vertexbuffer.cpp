/*
	Copyright (c) 2024 Anthony J. Thibault
	This software is licensed under the MIT License. See LICENSE for more details.
*/

#include "vertexbuffer.h"

#include <cassert>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

VertexBufferObject::VertexBufferObject(int targetIn, const std::vector<float>& data, unsigned int flags)
{
	target = targetIn;
	glGenBuffers(1, &obj);
	Bind();
	glBufferData(target, sizeof(float) * data.size(), (void*)data.data(), flags);
	Unbind();
	elementSize = 1;
	type = GL_FLOAT;
	numElements = (int)data.size();
}

VertexBufferObject::VertexBufferObject(int targetIn, const std::vector<glm::vec2>& data, unsigned int flags)
{
	target = targetIn;
	glGenBuffers(1, &obj);
	Bind();
	glBufferData(target, sizeof(glm::vec2) * data.size(), (void*)data.data(), flags);
	Unbind();
	elementSize = 2;
	type = GL_FLOAT;
	numElements = (int)data.size();
}

VertexBufferObject::VertexBufferObject(int targetIn, const std::vector<glm::vec3>& data, unsigned int flags)
{
	target = targetIn;
	glGenBuffers(1, &obj);
	Bind();
	glBufferData(target, sizeof(glm::vec3) * data.size(), (void*)data.data(), flags);
	Unbind();
	elementSize = 3;
	type = GL_FLOAT;
	numElements = (int)data.size();
}

VertexBufferObject::VertexBufferObject(int targetIn, const std::vector<glm::vec4>& data, unsigned int flags)
{
	target = targetIn;
	glGenBuffers(1, &obj);
	Bind();
	glBufferData(target, sizeof(glm::vec4) * data.size(), (void*)data.data(), flags);
	Unbind();
	elementSize = 4;
	type = GL_FLOAT;
	numElements = (int)data.size();
}

VertexBufferObject::VertexBufferObject(int targetIn, const std::vector<uint32_t>& data, unsigned int flags)
{
	target = targetIn;
	glGenBuffers(1, &obj);
	Bind();
	glBufferData(target, sizeof(uint32_t) * data.size(), (void*)data.data(), flags);
	Unbind();
	elementSize = 1;
	type = GL_INT;
	numElements = (int)data.size();
}

VertexBufferObject::~VertexBufferObject()
{
	glDeleteBuffers(1, &obj);
}

void VertexBufferObject::Bind() const
{
	glBindBuffer(target, obj);
}

void VertexBufferObject::Unbind() const
{
	glBindBuffer(target, 0);
}

void VertexBufferObject::Update(const std::vector<float>& data)
{
	Bind();
	glBufferSubData(target, 0, sizeof(float) * data.size(), (void*)data.data());
	Unbind();
}

void VertexBufferObject::Update(const std::vector<glm::vec2>& data)
{
	Bind();
	glBufferSubData(target, 0, sizeof(glm::vec2) * data.size(), (void*)data.data());
	Unbind();
}

void VertexBufferObject::Update(const std::vector<glm::vec3>& data)
{
	Bind();
	glBufferSubData(target, 0, sizeof(glm::vec3) * data.size(), (void*)data.data());
	Unbind();
}

void VertexBufferObject::Update(const std::vector<glm::vec4>& data)
{
	Bind();
	glBufferSubData(target, 0, sizeof(glm::vec4) * data.size(), (void*)data.data());
	Unbind();
}

void VertexBufferObject::Update(const std::vector<uint32_t>& data)
{
	Bind();
	glBufferSubData(target, 0, sizeof(uint32_t) * data.size(), (void*)data.data());
	Unbind();
}

void VertexBufferObject::Read(std::vector<uint32_t>& data)
{
	Bind();
	size_t bufferSize = sizeof(uint32_t) * data.size();
	assert(bufferSize == (elementSize * sizeof(uint32_t) * numElements));
	//void* rawBuffer = glMapBuffer(target, GL_READ_ONLY);
	void* rawBuffer = glMapBufferRange(target, 0, bufferSize, GL_MAP_READ_BIT);
	if (rawBuffer)
	{
		memcpy((void*)data.data(), rawBuffer, bufferSize);
	}
	glUnmapBuffer(target);
	Unbind();
}

VertexArrayObject::VertexArrayObject()
{
	glGenVertexArrays(1, &obj);
}

VertexArrayObject::~VertexArrayObject()
{
	glDeleteVertexArrays(1, &obj);
}

void VertexArrayObject::Bind() const
{
	glBindVertexArray(obj);
}

void VertexArrayObject::Unbind() const
{
	glBindVertexArray(0);
}

void VertexArrayObject::SetAttribBuffer(int loc, std::shared_ptr<VertexBufferObject> attribBuffer)
{
	assert(attribBuffer->target == GL_ARRAY_BUFFER);
	Bind();
	attribBuffer->Bind();
	glVertexAttribPointer(loc, attribBuffer->elementSize, attribBuffer->type, attribBuffer->normalized,
		attribBuffer->offset * sizeof(attribBuffer->type), (void*)(attribBuffer->stride * sizeof(attribBuffer->type)));
	glEnableVertexAttribArray(loc);
	attribBuffer->Unbind();
	attribBufferVec.push_back(attribBuffer);
	Unbind();
}

void VertexArrayObject::SetElementBuffer(std::shared_ptr<VertexBufferObject> elementBufferIn)
{
	assert(elementBufferIn->target == GL_ELEMENT_ARRAY_BUFFER);
	elementBuffer = elementBufferIn;

	Bind();
	elementBufferIn->Bind();
	Unbind();
}

void VertexArrayObject::DrawElements(int mode) const
{
	Bind();
	glDrawElements((GLenum)mode, elementBuffer->numElements, GL_UNSIGNED_INT, nullptr);
	Unbind();
}

