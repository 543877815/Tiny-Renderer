#pragma once

#include "common.h"
// https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.2.hello_triangle_indexed/hello_triangle_indexed.cpp
RENDERABLE_BEGIN
class Rectangle2DObj : public RenderObjectNaive<float, uint32_t> {
public:
	Rectangle2DObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);

private:
	void SetUpGLStatus();
	void SetUpTexture(int num = 0) override {}
	void SetUpData() override;
};

RENDERABLE_END

