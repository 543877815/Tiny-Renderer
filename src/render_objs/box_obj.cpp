#include "box_obj.h"

RENDERABLE_BEGIN
BoxObj::BoxObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr)
{
	SetUpData();
	auto ConfigPtr = std::static_pointer_cast<Parser::RenderObjConfigSimple>(baseConfigPtr);
	SetUpShader(ConfigPtr->vertexShader, ConfigPtr->fragmentShader);
	SetUpTexture(2);
}

void BoxObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	SetUpGLStatus();

	auto projection = std::any_cast<glm::mat4>(uniform.at("projection"));
	auto view = std::any_cast<glm::mat4>(uniform.at("view"));
	auto model = std::any_cast<glm::mat4>(uniform.at("model"));

	m_shader->Use();
	m_shader->SetMat4("projection", projection);
	m_shader->SetMat4("view", view);
	m_shader->SetMat4("model", model);
	for (auto idx : m_textureIdxes) {
		m_textures->BindTexture(idx);
	}
	RenderObjectNaive::Draw();
}

void BoxObj::SetUpGLStatus()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void BoxObj::SetUpTexture(int num)
{
	m_textures = std::make_unique<Texture>(num);
	size_t idx1 = m_textures->GenerateTexture("./textures/awesomeface.png");
	m_textureIdxes.emplace_back(idx1);
	size_t idx2 = m_textures->GenerateTexture("./textures/container.jpg");
	m_textureIdxes.emplace_back(idx2);

	m_shader->Use();
	m_shader->SetInt("texture1", idx1);
	m_shader->SetInt("texture2", idx2);
}

void BoxObj::SetUpData()
{
	std::vector<VertexInfo> vertexInfo = std::vector<VertexInfo>{
		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 5, 0},
		{"aTexCoord", 1, 2, GL_FLOAT, GL_FALSE, 5, 3}
	};

	std::vector<float>vertices = std::vector<float>{
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	};

	SetMesh(&vertices, &vertexInfo);
}
RENDERABLE_END