#include "./gs_framebuffer_obj.h"


RENDERABLE_BEGIN
void GSFrameBufferObj::UpdateFBO()
{
	auto instance = Camera::GetInstance();
	int screen_width = instance->GetScreenWidth();
	int screen_height = instance->GetScreenHeight();
	if (screen_width != m_fboSize.x || screen_height != m_fboSize.y)
	{
		SetFBOSize({ screen_width, screen_height });
		SetUpFBOColorTex();
	}
}

GSFrameBufferObj::GSFrameBufferObj(const std::string& vertexShader, const std::string& fragmentShader)
{
	SetUpData();
	SetUpShader(vertexShader, fragmentShader);
	SetUpTexture(1);
	SetUpFBOParams();
	SetUpFBOColorTex();
}

void GSFrameBufferObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	SetUpGLStatus();
	UpdateFBO();
	m_shader->Use();
	m_textures->BindTexture(m_textureIdx);
	m_shader->SetVec4("color", glm::vec4(1.0f));
	m_shader->SetInt("colorTexture", m_textureIdx);
	RenderObjectNaive::Draw();
}

void GSFrameBufferObj::SetUpGLStatus()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glm::vec4 clearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
}

void GSFrameBufferObj::ImGuiCallback()
{
	static int selected_option = 0;
	ImGui::Text("Framebuffer precision");
	bool isChanged = false;
	isChanged |= ImGui::RadioButton("Uint8", &selected_option, 0);
	ImGui::SameLine();
	isChanged |= ImGui::RadioButton("Fp16", &selected_option, 1);
	ImGui::SameLine();
	isChanged |= ImGui::RadioButton("Fp32", &selected_option, 2);
	if (isChanged)
	{
		m_precision = static_cast<PRECISION>(selected_option);
		SetUpFBOColorTex();
	}
}

void GSFrameBufferObj::SetUpData()
{
	std::vector<float> vertices{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
	};
	std::vector<uint32_t> indices = {
		0, 1, 2,
		0, 2, 3
	};
	std::vector<VertexInfo> vertexInfo = {
		{"position", 0, 3, GL_FLOAT, GL_FALSE, 5, 0},
		{"uv", 1, 2, GL_FLOAT, GL_FALSE, 5, 3}
	};

	SetMesh(&vertices, &vertexInfo, &indices);
	SetPrimitive(GL_TRIANGLES);
}

void GSFrameBufferObj::SetUpFBOColorTex()
{
	auto instance = Camera::GetInstance();
	int screen_width = instance->GetScreenWidth();
	int screen_height = instance->GetScreenHeight();
	if (m_precision == UINT8)
	{
		if (!IsFBOCreated())
		{
			m_textureIdx = m_textures->GenerateTexture(screen_width, screen_height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, m_texParams, nullptr);
		}
		else
		{
			m_textures->UpdateTexture(m_textureIdx, screen_width, screen_height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, m_texParams, nullptr);
		}
	}
	else if (m_precision == PRECISION::FP16)
	{
		if (!IsFBOCreated())
		{
			m_textureIdx = m_textures->GenerateTexture(screen_width, screen_height, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, m_texParams, nullptr);
		}
		else
		{
			m_textures->UpdateTexture(m_textureIdx, screen_width, screen_height, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, m_texParams, nullptr);
		}
	}
	else if (m_precision == PRECISION::FP32)
	{
		if (!IsFBOCreated())
		{
			m_textureIdx = m_textures->GenerateTexture(screen_width, screen_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, m_texParams, nullptr);
		}
		else
		{
			m_textures->UpdateTexture(m_textureIdx, screen_width, screen_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, m_texParams, nullptr);
		}
	}
	m_fbo->AttachColor(m_textures);
	SetFBOSize({ screen_width, screen_height });
}

void GSFrameBufferObj::SetUpFBOParams()
{
	m_fbo = std::make_shared<FrameBuffer>();
	m_texParams.minFilter = FilterType::Nearest;
	m_texParams.magFilter = FilterType::Nearest;
	m_texParams.sWrap = WrapType::ClampToEdge;
	m_texParams.tWrap = WrapType::ClampToEdge;
}
RENDERABLE_END