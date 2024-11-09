#pragma once
#include "common.h"
#include "aabb.h"

RENDERABLE_BEGIN
class EllipsoidObj : public RenderObject<glm::vec3, uint32_t> {
public:
	EllipsoidObj(Parser::RenderObjConfig& config);
	std::shared_ptr<AABB> GetAABB();

	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	void ImGuiCallback();
	~EllipsoidObj() = default;

private:
	void SetUpData() override;

private:
	int m_number = 10000;
	glm::vec3 m_scale{ 0.3f, 0.7f, 1.0f };
	glm::vec4 m_rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
	std::shared_ptr<AABB> m_aabb_obj = nullptr;
	struct ImguiParams {
		bool show_aabb = true;
	} m_imgui_params;
};
RENDERABLE_END