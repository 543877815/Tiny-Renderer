#pragma once
#include "common.h"

RENDERABLE_BEGIN
class AxisObj : public RenderObject<float, uint32_t> {
public:
	AxisObj(Parser::RenderObjConfig& config) {
		SetUpData();
		SetUpShader(config);
	}

	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	~AxisObj() = default;

private:
	void SetUpData() override;
};
RENDERABLE_END
