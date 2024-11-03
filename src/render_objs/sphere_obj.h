#pragma once
#include <random>
#include <cmath>
#include <chrono>

#include "common.h"
#include "aabb_obj.h"

RENDERABLE_BEGIN
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SphereObj : public RenderObject<glm::vec3, uint32_t> {

public:
	SphereObj();
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	virtual void ImGuiCallback();
	~SphereObj() = default;

private:
	void SetUpShader() override;
	void SetUpData() override;
	void SetUpAABB();

private:
	int m_number = 10000;
	float m_radius = 1.0;
	std::shared_ptr<AABBObj> m_aabb_obj = nullptr;

	struct ImguiParams {
		bool show_aabb = true;
	} m_imgui_params;

};


RENDERABLE_END
