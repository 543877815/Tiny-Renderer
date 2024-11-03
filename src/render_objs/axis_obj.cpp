#include "axis_obj.h"


RENDERABLE_BEGIN
void AxisObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	auto projection = std::any_cast<glm::mat4>(uniform.at("projection"));
	auto view = std::any_cast<glm::mat4>(uniform.at("view"));
	auto model = std::any_cast<glm::mat4>(uniform.at("model"));

	m_shader->Use();
	m_shader->SetMat4("projection", projection);
	m_shader->SetMat4("view", view);
	m_shader->SetMat4("model", model);
	RenderObject::Draw();
}

void AxisObj::SetUpData()
{
	std::vector<VertexInfo> vertex_info = std::vector<VertexInfo>{
		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 6, 0},
		{"aColor", 1, 3, GL_FLOAT, GL_FALSE, 6, 3}
	};

	std::vector<float>vertices = std::vector<float>{
		-100.0, 0.0,    0.0,   1.0, 0.0, 0.0,
		 100.0, 0.0 ,   0.0,   1.0, 0.0, 0.0,
		 0.0,   100.0,  0.0,   0.0, 1.0, 0.0,
		 0.0,  -100.0,  0.0,   0.0, 1.0, 0.0,
		 0.0,   0.0,    100.0, 0.0, 0.0, 1.0,
		 0.0,   0.0,   -100.0, 0.0, 0.0, 1.0,
	};

	std::vector<uint32_t> indices = std::vector<uint32_t>{
		0, 1,
		2, 3,
		4, 5
	};

	SetMesh(&vertices, &vertex_info, &indices);
	SetPrimitive(GL_LINES);
}
RENDERABLE_END