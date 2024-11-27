#pragma once
#include "common.h"

RENDERABLE_BEGIN
class AxisObj : public RenderObjectNaive<float, uint32_t> {
public:
	AxisObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);

	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	~AxisObj() = default;

private:
	void SetUpData() override;
};
RENDERABLE_END
