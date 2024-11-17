#include "rectangle2d_obj.h"

RENDERABLE_BEGIN
Rectangle2DObj::Rectangle2DObj(std::shared_ptr<Parser::RenderObjConfigBase> base_config_ptr)
{
	SetUpData();
	auto config_ptr = std::static_pointer_cast<Parser::RenderObjConfigNaive>(base_config_ptr);
	SetUpShader(config_ptr->vertex_shader, config_ptr->fragment_shader);
	SetUpTexture();
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
	std::vector<VertexInfo> vertex_info = std::vector<VertexInfo>{
		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 3, 0}
	};

	std::vector<float>vertices = std::vector<float>{
		0.5f,  0.5f, 0.0f,   // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};

	std::vector<uint32_t> indices = std::vector<uint32_t>{
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	};

	SetMesh(&vertices, &vertex_info, &indices);
}


RENDERABLE_END