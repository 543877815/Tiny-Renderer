#pragma once
#include "common.h"
#include "aabb.h"

RENDERABLE_BEGIN
class AABBObj : public RenderObject<glm::vec3, uint32_t> {
public:
	AABBObj();
	AABBObj(std::shared_ptr<AABB> aabb);
	void SetUpVertices();
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	std::shared_ptr<AABB> GetAABB() {
		return m_aabb;
	}
private:
	void SetUpShader() override {
		m_shader = std::make_unique<Shader>("./shader/aabb_vs.glsl", "./shader/aabb_fs.glsl");
	}

	void SetUpData() override;

private:
	std::vector<glm::vec3> m_vertices = std::vector<glm::vec3>(8, glm::vec3(0.0f));
	std::shared_ptr<AABB> m_aabb = std::make_shared<AABB>();
};
RENDERABLE_END