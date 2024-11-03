#pragma once
#include "common.h"

RENDERABLE_BEGIN
class AxisObj : public RenderObject<float, uint32_t> {
public:
	AxisObj() {
		SetUpData();
		SetUpShader();
	}

	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	~AxisObj() = default;

private:
	void SetUpShader() override {
		m_shader = std::make_unique<Shader>("./shader/axis_vs.glsl", "./shader/axis_fs.glsl");
	}

	void SetUpData() override;
};
RENDERABLE_END
