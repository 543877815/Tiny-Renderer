#include "map_obj.h"
#include <math.h>
RENDERABLE_BEGIN
MapObj::MapObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr)
{
	SetUpData();
	SetUpGLStatus();
	auto ConfigPtr = std::static_pointer_cast<Parser::RenderObjConfigSimple>(baseConfigPtr);
	SetUpShader(ConfigPtr->vertexShader, ConfigPtr->fragmentShader);
	SetUpTexture(1);
}

#define PI 3.1415926535f
void MapObj::UV2XYZ(const Point& uv, Point& xyz) {
	float lon = uv.x * 180.0f;
	float lat = uv.y * 90.0f;

	float radLon = (lon - 90.0f) * PI / 180.0f;
	float radLat = lat * PI / 180.0f;

	const float earthPerimeter = 20037508.34f;
	const float earthRadius = 6378137.0f;

	xyz.x = cos(radLat) * cos(radLon);
	xyz.y = sin(radLat);
	xyz.z = cos(radLat) * sin(radLon);

	if (abs(lat) < 85) {
		float WebMercatorX = earthRadius * radLon;
		float WebMercatorY = earthRadius * log(tan((PI / 4.0f) + (radLat / 2.0f)));

		xyz.u = (WebMercatorX / earthPerimeter + 1.0f) / 2.0f;
		xyz.v = (WebMercatorY / earthPerimeter + 1.0f) / 2.0f;
	}
	else {
		xyz.u = uv.u;
		xyz.v = uv.v;
	}
}


void MapObj::SetUpData()
{
	uint32_t numVerticesPerRow = m_gridWidth + 1;
	float gridUnitWidth = (m_gridRight - m_gridLeft) / m_gridWidth;
	float gridUnitHeight = (m_gridTop - m_gridBottom) / m_gridHeight;
	float uvUnitWidth = 1.0f / m_gridWidth;
	float uvUnitHeight = 1.0f / m_gridHeight;

	std::vector<Point> m_vertices;
	std::vector<uint32_t> indices;

	for (uint32_t i = 0; i <= static_cast<uint32_t>(m_gridWidth); i++)
	{
		for (uint32_t j = 0; j <= static_cast<uint32_t>(m_gridHeight); j++)
		{
			Point start{ .x = m_gridLeft + i * gridUnitWidth,
						 .y = m_gridBottom + j * gridUnitHeight,
						 .u = uvUnitWidth * i,
						 .v = uvUnitHeight * j };

			Point end;
			UV2XYZ(start, end);

			m_vertices.emplace_back(start * (1 - m_transform_scale) + end * m_transform_scale);

			if (i != m_gridWidth && j != m_gridHeight) {
				indices.emplace_back(i + j * numVerticesPerRow);
				indices.emplace_back(i + 1 + (j + 1) * numVerticesPerRow);
				indices.emplace_back(i + 1 + j * numVerticesPerRow);

				indices.emplace_back(i + (j + 1) * numVerticesPerRow);
				indices.emplace_back(i + 1 + (j + 1) * numVerticesPerRow);
				indices.emplace_back(i + j * numVerticesPerRow);
			}
		}
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	SetMesh(&m_vertices, &m_vertexInfo, &indices);
}

void MapObj::SetUpGLStatus()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void MapObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
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

void MapObj::SetUpTexture(int num)
{
	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	GLint maxLayers = 0;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxLayers);

	m_textures = std::make_unique<Texture>(num);
	size_t idx1 = m_textures->GenerateTexture("D:/jupyter_notebook/cg_practise/httplib/output/0/0/0.png");
	m_textureIdxes.emplace_back(idx1);

	m_shader->Use();
	m_shader->SetInt("texture1", static_cast<int>(idx1));
}


void MapObj::ImGuiCallback()
{
	bool is_change = false;

	static int selection = 2;
	if (ImGui::CollapsingHeader("Rasterization Mode")) {
		is_change |= ImGui::RadioButton("point", &selection, 0);
		ImGui::SameLine();
		is_change |= ImGui::RadioButton("line", &selection, 1);
		ImGui::SameLine();
		is_change |= ImGui::RadioButton("fill", &selection, 2);
	}
	if (selection == 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	else if (selection == 1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (selection == 2) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	is_change |= ImGui::SliderInt("grid_width", &m_gridWidth, 2, 50);
	is_change |= ImGui::SliderInt("grid_height", &m_gridHeight, 2, 50);
	is_change |= ImGui::SliderFloat("grid_left", &m_gridLeft, -1.0f, 0.0f);
	is_change |= ImGui::SliderFloat("grid_bottom", &m_gridBottom, -1.0f, 0.0f);
	is_change |= ImGui::SliderFloat("grid_right", &m_gridRight, 0.0f, 1.0f);
	is_change |= ImGui::SliderFloat("grid_top", &m_gridTop, 0.0f, 1.0f);
	is_change |= ImGui::SliderFloat("transform_scale", &m_transform_scale, 0.0f, 1.0f);
	if (is_change) SetUpData();
}


RENDERABLE_END

