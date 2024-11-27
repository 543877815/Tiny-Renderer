#include "rectangle2d_obj.h"

RENDERABLE_BEGIN
Rectangle2DObj::Rectangle2DObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr)
{
	SetUpData();
	auto ConfigPtr = std::static_pointer_cast<Parser::RenderObjConfigSimple>(baseConfigPtr);
	SetUpShader(ConfigPtr->vertexShader, ConfigPtr->fragmentShader);
	SetUpTexture();
	SetUpGLStatus();
}

void Rectangle2DObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	m_shader->Use();
	RenderObjectNaive::Draw();
}

void Rectangle2DObj::SetUpGLStatus()
{
	glDisable(GL_DEPTH_TEST);
}

void Rectangle2DObj::SetUpData()
{
	std::vector<VertexInfo> vertexInfo = std::vector<VertexInfo>{
		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 3, 0}
	};

	std::vector<float>vertices = std::vector<float>{
		0.5f,  0.5f, 0.0f,   // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};

	std::vector<uint32_t> indices = std::vector<uint32_t>{
		3, 1, 0,  // first Triangle
		3, 2, 1   // second Triangle
	};

	SetMesh(&vertices, &vertexInfo, &indices);
}


RENDERABLE_END