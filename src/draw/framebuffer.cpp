#include "./framebuffer.h"
#include "./texture.h"
#include <glad/glad.h>


FrameBuffer::FrameBuffer()
{
	glGenFramebuffers(1, &fbo);
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &fbo);
	fbo = 0;
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void FrameBuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::AttachColor(std::shared_ptr<Texture> colorTex)
{
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex->GetTexture(0), 0);
	colorAttachment = colorTex;
	Unbind();

}

void FrameBuffer::AttachDepth(std::shared_ptr<Texture> depthTex)
{
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex->GetTexture(0), 0);
	depthAttachment = depthTex;
	Unbind();
}

void FrameBuffer::AttachStencil(std::shared_ptr<Texture> stencilTex)
{
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTex->GetTexture(0), 0);
	stencilAttachment = stencilTex;
	Unbind();
}

bool FrameBuffer::IsComplete() const
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}