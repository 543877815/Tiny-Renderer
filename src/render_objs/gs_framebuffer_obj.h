#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "common.h"
#include "../draw/vertexbuffer.h"
#include "../draw/camera.h"
#include "../draw/framebuffer.h"
#include "../draw/camera.h"
RENDERABLE_BEGIN
class GSFrameBufferObj :public RenderObjectNaive<float, uint32_t> {
public:
	enum PRECISION : uint32_t {
		UINT8,
		FP16,
		FP32
	};

	GSFrameBufferObj(const std::string& vertexShader, const std::string& fragmentShader);

	GSFrameBufferObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr)
	{
		auto ConfigPtr = std::static_pointer_cast<Parser::RenderObjConfigSimple>(baseConfigPtr);
		GSFrameBufferObj(ConfigPtr->vertexShader, ConfigPtr->fragmentShader);
	}
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	glm::ivec2& GetFboSize() { return m_fboSize; }
	void BindFBO() { m_fbo->Bind(); }
	void ImGuiCallback();
private:
	void SetUpTexture(int num) override { m_textures = std::make_unique<Texture>(num); }
	void SetUpData() override;
	void SetUpFBOColorTex();
	void SetUpFBOParams();
	void SetUpGLStatus();
	void UpdateFBO();
	bool IsFBOCreated() { return m_fboSize.x != 0 && m_fboSize.y != 0; }
	void SetFBOSize(glm::ivec2 size) { m_fboSize = size; }

private:
	std::shared_ptr<FrameBuffer> m_fbo = nullptr;
	glm::ivec2 m_fboSize = { 0, 0 };
	int m_textureIdx = -1;
	Texture::Params m_texParams;
	PRECISION m_precision = PRECISION::UINT8;
};

RENDERABLE_END