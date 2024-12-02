#pragma once
#include <memory>

class Texture;

class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	void Bind() const;
	void Unbind() const;
	void AttachColor(std::shared_ptr<Texture> colorTex);
	void AttachDepth(std::shared_ptr<Texture> depthTex);
	void AttachStencil(std::shared_ptr<Texture> stencilTex);

	bool IsComplete() const;

	std::shared_ptr<Texture> GetColorTexture() const { return colorAttachment; }
	std::shared_ptr<Texture> GetDepthTexture() const { return depthAttachment; }
	std::shared_ptr<Texture> GetStencilTexture() const { return stencilAttachment; }

	uint32_t fbo;
	std::shared_ptr<Texture> colorAttachment;
	std::shared_ptr<Texture> depthAttachment;
	std::shared_ptr<Texture> stencilAttachment;
};