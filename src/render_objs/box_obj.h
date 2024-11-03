#pragma once
#include "common.h"

// https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
RENDERABLE_BEGIN
class BoxObj : public RenderObject<float, uint32_t> {
public:
	BoxObj();
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);

private:
	void SetUpGLStatus() {
		glEnable(GL_DEPTH_TEST);
	}

	void SetUpTexture(int num = 0) override;
	void SetUpShader() override;
	void SetUpData() override;
};
RENDERABLE_END