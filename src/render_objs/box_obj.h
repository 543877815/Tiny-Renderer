#pragma once
#include "common.h"

// https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
RENDERABLE_BEGIN
class BoxObj : public RenderObjectNaive<float, uint32_t> {
public:
	BoxObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);

private:
	void SetUpGLStatus();
	void SetUpTexture(int num = 0) override;
	void SetUpData() override;

private:
	std::vector<size_t> m_textureIdxes{};
};
RENDERABLE_END