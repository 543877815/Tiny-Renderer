#pragma once
#include <random>
#include <cmath>
#include <chrono>

#include "common.h"
#include "aabb_obj.h"

//glm::mat4 GetProjectionMatrix(float fx, float fy, int width, int height, float znear, float zfar)
//{
//	glm::mat4 projectionMatrix{
//		(2.0f * fx) / width, 0, 0, 0,
//		0, -(2.0f * fy) / height, 0, 0,
//		0, 0, zfar / (zfar - znear), 1.0f,
//		0, 0, -(zfar * znear) / (zfar - znear), 0
//	};
//	return std::move(projectionMatrix);
//}

//glm::mat4 GetViewMatrix()

RENDERABLE_BEGIN
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

class SphereObj : public RenderObjectNaive<glm::vec3, uint32_t> {

public:
	SphereObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	virtual void ImGuiCallback();
	~SphereObj() = default;

private:
	void SetUpData() override;
	void SetUpAABB();

private:
	int m_number = 10000;
	float m_radius = 1.0;
	std::shared_ptr<AABBObj> m_aabbObj = nullptr;

	struct ImguiParams {
		bool showAABB = true;
	} m_imguiParams;

};


RENDERABLE_END
