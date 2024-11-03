#pragma once
#include "common.h"

RENDERABLE_BEGIN
static struct Vertices {
	glm::vec3 position;
	glm::vec3 normal;
};

static struct Material {
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float shininess;
};

static struct Light {
	float position[3];
	float diffuse[3];
	float ambient[3];
	float specular[3];
};


class LearningOpenGLMaterial : public RenderObject<Vertices, uint32_t> {
public:
	LearningOpenGLMaterial();

	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	void ImGuiCallback();
	~LearningOpenGLMaterial() = default;

private:
	void SetUpData() override;
	void SetUpShader() override;

private:
	int m_number = 10000;
	glm::vec3 m_scale{ 0.3f, 0.7f, 1.0f };
	glm::vec4 m_rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
	std::unique_ptr<Shader> m_lighting_shader = nullptr;
	Material m_material{
		 { 1.0f, 0.5f, 0.31f },
		 { 1.0f, 0.5f, 0.31f },
		 { 0.5f, 0.5f, 0.5f },
		 32.0f
	};
	Light m_light{
		 { 1.2f, 1.0f, 2.0f },
		 { 1.0f, 0.35f, 0.65f },
		 { 0.2f, 0.07f, 0.13f },
		 { 1.0f, 1.0f, 1.0f }
	};

};
RENDERABLE_END