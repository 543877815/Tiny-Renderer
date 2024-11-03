#include "sphere_obj.h"

RENDERABLE_BEGIN
SphereObj::SphereObj()
{
	SetUpData();
	SetUpShader();
	SetUpAABB();
}

void SphereObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	m_shader->Use();
	auto projection = std::any_cast<glm::mat4>(uniform.at("projection"));
	auto view = std::any_cast<glm::mat4>(uniform.at("view"));
	auto model = std::any_cast<glm::mat4>(uniform.at("model"));
	m_shader->SetMat4("projection", projection);
	m_shader->SetMat4("view", view);
	m_shader->SetMat4("model", model);
	RenderObject::Draw();

	if (m_imgui_params.show_aabb) {
		m_aabb_obj->DrawObj(uniform);
	}
}

void SphereObj::ImGuiCallback()
{
	bool changed = false;
	changed |= ImGui::SliderInt("number", &m_number, 0, 100000);
	if (changed |= ImGui::SliderFloat("radius", &m_radius, 0.0, 5.0)) {
		SetUpAABB();
	}

	ImGui::Checkbox("show aabb", &m_imgui_params.show_aabb);
	if (changed) SetUpData();
}

void SphereObj::SetUpShader()
{
	m_shader = std::make_unique<Shader>("./shader/sphere_vs.glsl", "./shader/sphere_fs.glsl");
}

void SphereObj::SetUpData()
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 generator(seed);
	auto m_uniform01 = std::uniform_real_distribution<double>(0.0, 1.0);
	std::vector<glm::vec3> vertices;

	for (int i = 0; i < m_number; i++) {
		float theta = 2 * M_PI * m_uniform01(generator);
		float phi = acos(1 - 2 * m_uniform01(generator));
		float x = sin(phi) * cos(theta) * m_radius;
		float y = sin(phi) * sin(theta) * m_radius;
		float z = cos(phi) * m_radius;
		vertices.emplace_back(glm::vec3(x, y, z));
	}

	std::vector<VertexInfo> vertex_info = std::vector<VertexInfo>{
		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 1, 0},
	};

	SetMesh(&vertices, &vertex_info);
	SetPrimitive(GL_POINTS);
}

void SphereObj::SetUpAABB()
{
	if (!m_aabb_obj) m_aabb_obj = std::make_shared<AABBObj>();
	m_aabb_obj->GetAABB()->reset();
	m_aabb_obj->GetAABB()->expand({ m_radius, m_radius, m_radius });
	m_aabb_obj->GetAABB()->expand({ -m_radius, -m_radius, -m_radius });
}

RENDERABLE_END