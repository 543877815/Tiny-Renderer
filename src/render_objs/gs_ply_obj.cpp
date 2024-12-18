#include "./gs_ply_obj.h"

#include <mutex>
RENDERABLE_BEGIN
constexpr const float SH_C0 = 0.28209479177387814f;
constexpr const float SH_C1 = 0.4886025119029199f;
constexpr const float SH_C2[] = {
	1.0925484305920792f,
	-1.0925484305920792f,
	0.31539156525252005f,
	-1.0925484305920792f,
	0.5462742152960396f
};
constexpr const float SH_C3[] = {
	-0.5900435899266435f,
	2.890611442640554f,
	-0.4570457994644658f,
	0.3731763325901154f,
	-0.4570457994644658f,
	1.445305721320277f,
	-0.5900435899266435f
};

std::pair<float, float> Base3DGSObj::calculateMinMax(const std::vector<float>& data)
{
	float min, max;
	min = max = data[0];

	for (float item : data) {
		if (item < min) min = item;
		if (item > max) max = item;
	}
	return { min, max };
}

unsigned int Base3DGSObj::Part1By2(unsigned int x)
{
	x &= 0x000003ff;
	x = (x ^ (x << 16)) & 0xff0000ff;
	x = (x ^ (x << 8)) & 0x0300f00f;
	x = (x ^ (x << 4)) & 0x030c30c3;
	x = (x ^ (x << 2)) & 0x09249249;
	x = (x ^ (x << 2)) & 0x09249249;
	return x;
}

unsigned int Base3DGSObj::encodeMorton3(unsigned int x, unsigned int y, unsigned int z)
{
	return (Part1By2(z) << 2) + (Part1By2(y) << 1) + Part1By2(x);
}


std::shared_ptr<Base3DGSCamera> Base3DGSCamera::GetInstance()
{
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);

	if (m_instance == nullptr) {
		m_instance = std::make_shared<Base3DGSCamera>();
		m_instance->SetProjection();
	}
	return m_instance;
}

template<typename T>
void GetSigma(std::vector<T>& scale, std::vector<T>& rotation, std::vector<T>& sigma)
{
	std::vector<T> M{
		1.0 - 2.0 * (rotation[2] * rotation[2] + rotation[3] * rotation[3]),
		2.0 * (rotation[1] * rotation[2] + rotation[0] * rotation[3]),
		2.0 * (rotation[1] * rotation[3] - rotation[0] * rotation[2]),

		2.0 * (rotation[1] * rotation[2] - rotation[0] * rotation[3]),
		1.0 - 2.0 * (rotation[1] * rotation[1] + rotation[3] * rotation[3]),
		2.0 * (rotation[2] * rotation[3] + rotation[0] * rotation[1]),

		2.0 * (rotation[1] * rotation[3] + rotation[0] * rotation[2]),
		2.0 * (rotation[2] * rotation[3] - rotation[0] * rotation[1]),
		1.0 - 2.0 * (rotation[1] * rotation[1] + rotation[2] * rotation[2]),
	};

	for (size_t i = 0; i < M.size(); i++) {
		M[i] *= scale[std::floor(i / 3)];
	}

	sigma[0] = M[0] * M[0] + M[3] * M[3] + M[6] * M[6];
	sigma[1] = M[0] * M[1] + M[3] * M[4] + M[6] * M[7];
	sigma[2] = M[0] * M[2] + M[3] * M[5] + M[6] * M[8];
	sigma[3] = M[1] * M[1] + M[4] * M[4] + M[7] * M[7];
	sigma[4] = M[1] * M[2] + M[4] * M[5] + M[7] * M[8];
	sigma[5] = M[2] * M[2] + M[5] * M[5] + M[8] * M[8];
}

void GSPlyObj::PlyHeader::UpdateVerticesOffset(std::string property, std::pair<size_t, size_t>& start_end)
{
	if (verticePropertiesOffset.find(property) == verticePropertiesOffset.end()) {
		verticePropertiesOrder.emplace_back(property);
		verticePropertiesOffset[property] = { start_end.first, start_end.second - start_end.first };
	}
	else {
		verticePropertiesOffset[property].second = start_end.second - verticePropertiesOffset[property].first;
	}
}

GSPlyObj::GSPlyObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr)
{
	auto configPtr = std::static_pointer_cast<Parser::RenderObjConfig3DGS>(baseConfigPtr);
	SetUpShader(configPtr->vertexShader.c_str(), configPtr->fragmentShader.c_str());
	SetUpFbo(configPtr->fboVertexShader.c_str(), configPtr->fboFragmentShader.c_str());
	std::ifstream file(configPtr->modelPath, std::ios::binary);
	LoadModelHeader(file, m_header);
	SetUpAttribute();
	LoadVertices(file);
	PresortIndices(m_vertices);
	GenerateTextureData();
	GenerateTexture();
	SetUpData();
}

void GSPlyObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	if (m_fbo)
		m_fbo->PrepareDraw();
	{
		glm::mat4 projection = std::any_cast<glm::mat4>(uniform.at("perspective_projection"));
		glm::mat4 view = std::any_cast<glm::mat4>(uniform.at("view"));
		glm::mat4 model = std::any_cast<glm::mat4>(uniform.at("model"));
		glm::vec2 focal = std::any_cast<glm::vec2>(uniform.at("focal"));
		glm::vec2 viewport = std::any_cast<glm::vec2>(uniform.at("viewport"));
		glm::vec3 camPos = std::any_cast<glm::vec3>(uniform.at("camPos"));
		glm::vec2 tanFov = std::any_cast<glm::vec2>(uniform.at("tanFov"));
		glm::vec2 nearFar = std::any_cast<glm::vec2>(uniform.at("nearFar"));
		SetUpGLStatus();
		RunSortUpdateDepth();
		m_shader->Use();
		m_renderVAO->Bind();
		m_gaussian_texture->BindTexture(m_textureIdx);
		m_shader->SetMat4("projection", projection);
		m_shader->SetMat4("view", view);
		m_shader->SetMat4("model", model);
		m_shader->SetVec2("focal", focal);
		m_shader->SetVec2("viewport", viewport);
		m_shader->SetVec3("camPos", camPos);
		m_shader->SetVec2("tanFov", tanFov);
		m_shader->SetVec2("nearFar", nearFar);
		m_shader->SetInt("u_texture", m_textureIdx);
		m_shader->SetInt("sphericalHarmonicsDegree", m_sphericalHarmonicsDegree);
		m_shader->SetInt("showGaussian", 3);
		Draw();
		m_renderVAO->Unbind();
	}
	if (m_fbo)
		m_fbo->DrawObj(uniform);
}

void GSPlyObj::ImGuiCallback()
{
	ImGui::SliderInt("SphericalHarmonicsDegree", &m_sphericalHarmonicsDegree, 1, 3);
	{
		static int selected_option = 0;
		ImGui::Text("Sorting Method");
		bool isChanged = false;
		if (ImGui::RadioButton("Countint Sort (CPU)", &selected_option, 0))
		{
			m_sorter = std::make_shared<CountingSortCPU<PlyVertex3>>(m_vertexCount, m_sortOrder);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Quick Sort (CPU)", &selected_option, 1))
		{
			m_sorter = std::make_shared<QuickSortCPU<PlyVertex3>>(m_vertexCount, m_sortOrder);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Radix Sort (CPU)", &selected_option, 2))
		{
			m_sorter = std::make_shared<RadixSortCPU<PlyVertex3>>(m_vertexCount, m_sortOrder);
		}
		if (ImGui::RadioButton("Single Pass Radix Sort (GPU)", &selected_option, 3))
		{
			m_sorter = std::make_shared<SinglePassRadixSortGPU<PlyVertex3>>(m_vertexCount, m_sortOrder);
		}
		if (ImGui::RadioButton("Multiple Pass Radix Sort (GPU)", &selected_option, 4))
		{
			m_sorter = std::make_shared<MultiPassRadixSortGPU<PlyVertex3>>(m_vertexCount, m_sortOrder);
		}
		m_sortMethod = static_cast<SORT_METHOD>(selected_option);
	}
	Base3DGSObj::ImGuiCallback();
	m_fbo->ImGuiCallback();
}

void GSPlyObj::RunSortUpdateDepth()
{
	m_sorter->Sort(m_vertices, m_indices, m_depthIndex, m_depthIndexVBO);
}

void GSPlyObj::LoadModelHeader(std::ifstream& file, PlyHeader& header)
{
	std::string line;
	bool headerEnd = false;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string token;
		std::cout << line << std::endl;
		iss >> token;
		if (token == "ply") {
			// PLY format indicator
		}
		else if (token == "format") {
			iss >> header.format;
		}
		else if (token == "element") {
			iss >> token;

			if (token == "vertex") {
				iss >> header.verticeNum;
			}
			else if (token == "face") {
				iss >> header.face_number;
			}
		}
		else if (token == "property") {
			PlyProperty property;
			iss >> property.type >> property.name;

			if (header.vertexProperties.size() < static_cast<size_t>(header.verticeNum)) {
				header.vertexProperties.push_back(property);
			}
			else {
				header.faceProperties.push_back(property);
			}
			std::pair<size_t, size_t> start_end = { header.vertex_offset, 0 };

			if (property.type == "char" || property.type == "int8")
				header.vertex_offset += 1;
			else if (property.type == "uchar" || property.type == "uint8")
				header.vertex_offset += 1;
			else if (property.type == "short" || property.type == "int16")
				header.vertex_offset += 2;
			else if (property.type == "ushort" || property.type == "uint16")
				header.vertex_offset += 2;
			else if (property.type == "int" || property.type == "int32")
				header.vertex_offset += 4;
			else if (property.type == "uint" || property.type == "uint32")
				header.vertex_offset += 4;
			else if (property.type == "float" || property.type == "float32")
				header.vertex_offset += 4;
			else if (property.type == "double" || property.type == "float64")
				header.vertex_offset += 8;
			else
				std::cout << "Unsupported type " << property.type.c_str() << " for property" << property.name << std::endl;

			start_end.second = header.vertex_offset;

			std::string key;
			if (property.name == "x" || property.name == "y" || property.name == "z")
				header.UpdateVerticesOffset("position", start_end);
			else if (property.name == "nx" || property.name == "ny" || property.name == "nz")
				header.UpdateVerticesOffset("normal", start_end);
			else if (property.name == "opacity")
				header.UpdateVerticesOffset("opacity", start_end);
			else if (property.name.find("rot") != std::string::npos)
				header.UpdateVerticesOffset("rot", start_end);
			else if (property.name.find("scale") != std::string::npos)
				header.UpdateVerticesOffset("scale", start_end);
			else if (property.name.find("f_rest") != std::string::npos || property.name.find("f_dc") != std::string::npos)
				header.UpdateVerticesOffset("shs", start_end);
		}
		else if (token == "end_header") {
			headerEnd = true;
			break;
		}
	}
	if (!headerEnd) {
		throw std::runtime_error("Could not find end of header");
	}
}

void GSPlyObj::LoadVertices(std::ifstream& file)
{
	static_assert(sizeof(PlyVertexStorage) == 62 * sizeof(float), "");
	for (auto i = 0; i < m_header.verticeNum; i++) {
		assert(file.is_open());
		assert(!file.eof());
		PlyVertexStorage vertexBuffer;
		memset(&vertexBuffer, 0, sizeof(vertexBuffer));

		for (auto property : m_header.verticePropertiesOrder) {
			if (m_header.verticePropertiesOffset.find(property) != m_header.verticePropertiesOffset.end()) {
				if (property == "position")
					file.read(reinterpret_cast<char*>(&vertexBuffer.position), m_header.verticePropertiesOffset[property].second);
				else if (property == "normal")
					file.read(reinterpret_cast<char*>(&vertexBuffer.normal), m_header.verticePropertiesOffset[property].second);
				else if (property == "shs")
					file.read(reinterpret_cast<char*>(&vertexBuffer.shs), m_header.verticePropertiesOffset[property].second);
				else if (property == "opacity")
					file.read(reinterpret_cast<char*>(&vertexBuffer.opacity), m_header.verticePropertiesOffset[property].second);
				else if (property == "scale")
					file.read(reinterpret_cast<char*>(&vertexBuffer.scale), m_header.verticePropertiesOffset[property].second);
				else if (property == "rot")
					file.read(reinterpret_cast<char*>(&vertexBuffer.rotation), m_header.verticePropertiesOffset[property].second);
			}
		}

		m_vertices[i].position = glm::vec4(vertexBuffer.position, 1.0f);
		m_vertices[i].scale = glm::exp(vertexBuffer.scale);
		m_vertices[i].opacity = 1.0f / (1.0f + std::exp(-vertexBuffer.opacity)); // sigmoid
		m_vertices[i].rotation = glm::normalize(vertexBuffer.rotation);
		m_vertices[i].shs[0] = vertexBuffer.shs[0] * SH_C0;
		m_vertices[i].shs[1] = vertexBuffer.shs[1] * SH_C0;
		m_vertices[i].shs[2] = vertexBuffer.shs[2] * SH_C0;

		auto SH_N = m_header.verticePropertiesOffset["shs"].second / sizeof(float) / 3;
		for (auto j = 1; j < SH_N; j++) {
			m_vertices[i].shs[j * 3 + 0] = vertexBuffer.shs[(j - 1) + 3];
			m_vertices[i].shs[j * 3 + 1] = vertexBuffer.shs[(j - 1) + SH_N + 2];
			m_vertices[i].shs[j * 3 + 2] = vertexBuffer.shs[(j - 1) + SH_N * 2 + 1];
		}

		assert(vertexBuffer.normal.x == 0.0f);
		assert(vertexBuffer.normal.y == 0.0f);
		assert(vertexBuffer.normal.z == 0.0f);
	}
	file.close();
}

void GSPlyObj::GenerateTextureData()
{
	for (size_t i = 0; i < m_vertexCount; i++) {
		size_t idx = m_indices[i];
		glm::vec3 pos = m_vertices[idx].position;
		glm::vec3* sh = reinterpret_cast<glm::vec3*>(&m_vertices[idx].shs);
		glm::vec3 result = SH_C0 * sh[0];
		result += 0.5f;
		result = (glm::max)(result, glm::vec3(0.0f));
		result = (glm::min)(result, glm::vec3(1.0f));
		glm::vec4 shs = glm::vec4(result, m_vertices[idx].opacity) * 255.0f;
		uint8_t shs_uint8[4]{ static_cast<uint8_t>(shs.x), static_cast<uint8_t>(shs.y), static_cast<uint8_t>(shs.z), static_cast<uint8_t>(shs.w) };
		std::vector<float> sigmas;
		GetSigmaFloat32(m_vertices[idx].rotation, m_vertices[idx].scale, sigmas);
		// 0: posx, 1: posy, 2: posz, 3: 1, 4: cov1, 5: cov2, 6: cov3, 7: cov4, 8: cov5, 9: cov6, 10: RGBA(lp), 11: opacity(hp), 12-60: shs
		std::copy_n(reinterpret_cast<float*>(&m_vertices[idx].position), 4, reinterpret_cast<float*>(&m_textureData[m_vertexLength * i]));
		std::copy_n(reinterpret_cast<float*>(sigmas.data()), 6, reinterpret_cast<float*>(&m_textureData[m_vertexLength * i + 4]));
		std::copy_n(reinterpret_cast<uint8_t*>(&shs_uint8), 4, reinterpret_cast<uint8_t*>(&m_textureData[m_vertexLength * i + 10]));
		std::copy_n(reinterpret_cast<float*>(&m_vertices[idx].opacity), 1, reinterpret_cast<float*>(&m_textureData[m_vertexLength * i + 11]));
		std::copy_n(reinterpret_cast<float*>(&m_vertices[idx].shs), (std::max)(static_cast<int>(m_header.verticePropertiesOffset["shs"].second / sizeof(float)), 3),
			reinterpret_cast<float*>(&m_textureData[m_vertexLength * i + 12]));
	}
}

void GSPlyObj::GetSigmaFloat32(glm::vec4& rotation, glm::vec3& scale, std::vector<float>& sigmaFloat32)
{
	std::vector<double> rotation_d{
		static_cast<double>(rotation[0]),
		static_cast<double>(rotation[1]),
		static_cast<double>(rotation[2]),
		static_cast<double>(rotation[3]),
	};

	std::vector<double> scale_d{
		static_cast<double>(scale.x),
		static_cast<double>(scale.y),
		static_cast<double>(scale.z),
	};

	std::vector<double> sigma(6);
	GetSigma(scale_d, rotation_d, sigma);

	for (size_t i = 0; i < 6; i++) {
		sigmaFloat32.emplace_back(sigma[i]);
	}
}

void GSPlyObj::SetUpAttribute()
{
	m_vertexCount = m_header.verticeNum;
	m_vertexLength = 64;
	m_textureHeight = std::ceil((2.0f * m_vertexCount) / m_textureWidth) * 8;
	m_textureData.resize(m_textureWidth * m_textureHeight * 4);
	m_vertices.resize(m_vertexCount);
	m_sorter = std::make_shared<CountingSortCPU<PlyVertex3>>(m_vertexCount, m_sortOrder);
	Base3DGSObj::SetUpAttribute();
}

void GSPlyObj::SetUpFbo(const char* vertexShader, const char* fragmentShader)
{
	m_fbo = std::make_shared<GSFrameBufferObj>(vertexShader, fragmentShader);
}

void Base3DGSObj::SetUpShader(const char* vertexShader, const char* fragmentShader)
{
	m_shader = std::make_shared<Shader>(vertexShader, fragmentShader);
}

void Base3DGSObj::GenerateTexture()
{
	Texture::Params m_texture_parameters;
	m_texture_parameters.minFilter = FilterType::Nearest;
	m_texture_parameters.magFilter = FilterType::Nearest;
	m_texture_parameters.sWrap = WrapType::ClampToEdge;
	m_texture_parameters.tWrap = WrapType::ClampToEdge;
	m_textureIdx = m_gaussian_texture->GenerateTexture(m_textureWidth, m_textureHeight,
		GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, m_texture_parameters, m_textureData.data());
}

void Base3DGSObj::SetUpData()
{
	m_renderVAO = std::make_shared<VertexArrayObject>();
	float position_scale = 1.0f;
	std::vector<glm::vec2> vertices{
			glm::vec2(-position_scale, -position_scale),
			glm::vec2(position_scale, -position_scale),
			glm::vec2(position_scale,  position_scale),
			glm::vec2(-position_scale,  position_scale)
	};
	m_rectangleVBO = std::make_shared<VertexBufferObject>(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
	m_renderVAO->SetAttribBuffer(0, m_rectangleVBO);
	m_depthIndexVBO = std::make_shared<VertexBufferObject>(GL_ARRAY_BUFFER, m_depthIndex, GL_DYNAMIC_DRAW); // int
	m_depthIndexVBO->SetOffset(1);
	m_renderVAO->SetAttribBuffer(1, m_depthIndexVBO);
	m_renderVAO->Bind();
	glVertexAttribDivisor(m_shader->GetAttribLocation("index"), 1);
	m_renderVAO->Unbind();
}

void Base3DGSObj::Draw()
{
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, m_vertexCount);
}

void Base3DGSObj::SetUpGLStatus()
{
	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void Base3DGSObj::SetUpAttribute()
{
	m_gaussian_texture = std::make_shared<Texture>(1);
	m_depthIndex.resize(m_vertexCount);
	m_indices.resize(m_vertexCount);
	for (uint32_t i = 0; i < m_indices.size(); i++) {
		m_indices[i] = i;
	}
	m_textureData.resize(m_textureWidth * m_textureHeight * 4);
}

void Base3DGSObj::ImGuiCallback()
{
	static bool isFolded = true;
	if (ImGui::CollapsingHeader("Base3DGSObj", &isFolded, ImGuiTreeNodeFlags_DefaultOpen))  // default open
	{
		static int maxVertexCount = m_vertexCount;
		int vertexCount = static_cast<int>(m_vertexCount);
		if (ImGui::SliderInt("vertexCount", &vertexCount, 0, maxVertexCount))
		{
			m_vertexCount = static_cast<uint32_t>(vertexCount);
		}
	}
}

GSSplatObj::GSSplatObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr)
{
	auto configPtr = std::static_pointer_cast<Parser::RenderObjConfig3DGS>(baseConfigPtr);
	SetUpShader(configPtr->vertexShader.c_str(), configPtr->fragmentShader.c_str());
	std::ifstream file(configPtr->modelPath, std::ios::binary);
	GetVertexCount(file);
	SetUpAttribute();
	LoadVertices(file);
	GenerateTexture();
	SetUpData();
}


void GSSplatObj::LoadVertices(std::ifstream& file)
{
	file.seekg(0, std::ios::beg);
	m_vertices.resize(m_vertexCount);
	for (size_t i = 0; i < m_vertexCount; i++) {
		assert(file.is_open(), "");
		file.read(reinterpret_cast<char*>(&m_vertices[i]), sizeof(SplatVertex));
		std::vector<uint32_t> sigmasHalf2x16;
		GetSigmaHalf2x16(m_vertices[i], sigmasHalf2x16);
		// 0: posx, 1: posy, 2: posz, 3: 0, 4: cov12, 5: cov34, 6: cov56, 7: RGBA(lp)
		std::copy_n(reinterpret_cast<float*>(&m_vertices[i].position), 3, reinterpret_cast<float*>(&m_textureData[m_vertexLength * i]));
		std::copy_n(reinterpret_cast<uint32_t*>(sigmasHalf2x16.data()), 3, reinterpret_cast<uint32_t*>(&m_textureData[m_vertexLength * i + 4]));
		std::copy_n(reinterpret_cast<uint8_t*>(&m_vertices[i].shs), 4, reinterpret_cast<uint8_t*>(&m_textureData[m_vertexLength * i + 7]));
	}
}


void GSSplatObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	glm::mat4 projection = std::any_cast<glm::mat4>(uniform.at("perspective_projection"));
	glm::mat4 view = std::any_cast<glm::mat4>(uniform.at("view"));
	glm::mat4 model = std::any_cast<glm::mat4>(uniform.at("model"));
	glm::vec2 focal = std::any_cast<glm::vec2>(uniform.at("focal"));
	glm::vec2 viewport = std::any_cast<glm::vec2>(uniform.at("viewport"));
	glm::vec2 nearFar = std::any_cast<glm::vec2>(uniform.at("nearFar"));
	SetUpGLStatus();
	RunSortUpdateDepth();
	m_shader->Use();
	m_renderVAO->Bind();
	m_gaussian_texture->BindTexture(m_textureIdx);
	m_shader->SetMat4("projection", projection);
	m_shader->SetMat4("view", view);
	m_shader->SetMat4("model", model);
	m_shader->SetVec2("focal", focal);
	m_shader->SetVec2("viewport", viewport);
	m_shader->SetVec2("nearFar", nearFar);
	m_shader->SetInt("u_texture", m_textureIdx);
	Draw();
	m_renderVAO->Unbind();
}

void GSSplatObj::ImGuiCallback()
{
	{
		static int selected_option = 0;
		ImGui::Text("Sorting Method");
		bool isChanged = false;
		if (ImGui::RadioButton("Countint Sort (CPU)", &selected_option, 0))
		{
			m_sorter = std::make_shared<CountingSortCPU<SplatVertex>>(m_vertexCount, m_sortOrder);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Quick Sort (CPU)", &selected_option, 1))
		{
			m_sorter = std::make_shared<QuickSortCPU<SplatVertex>>(m_vertexCount, m_sortOrder);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Radix Sort (CPU)", &selected_option, 2))
		{
			m_sorter = std::make_shared<RadixSortCPU<SplatVertex>>(m_vertexCount, m_sortOrder);
		}
		if (ImGui::RadioButton("Single Pass Radix Sort (GPU)", &selected_option, 3))
		{
			m_sorter = std::make_shared<SinglePassRadixSortGPU<SplatVertex>>(m_vertexCount, m_sortOrder);
		}
		if (ImGui::RadioButton("Multiple Pass Radix Sort (GPU)", &selected_option, 4))
		{
			m_sorter = std::make_shared<MultiPassRadixSortGPU<SplatVertex>>(m_vertexCount, m_sortOrder);
		}
		m_sortMethod = static_cast<SORT_METHOD>(selected_option);
	}
}

void GSSplatObj::RunSortUpdateDepth()
{
	m_sorter->Sort(m_vertices, m_indices, m_depthIndex, m_depthIndexVBO);
}

void GSSplatObj::GetSigmaHalf2x16(SplatVertex& vertexBuffer, std::vector<uint32_t>& sigmasHalf2x16)
{
	std::vector<double> rot{
		(static_cast<double>(vertexBuffer.rotation[0]) - 128.0) / 128.0,
		(static_cast<double>(vertexBuffer.rotation[1]) - 128.0) / 128.0,
		(static_cast<double>(vertexBuffer.rotation[2]) - 128.0) / 128.0,
		(static_cast<double>(vertexBuffer.rotation[3]) - 128.0) / 128.0,
	};

	std::vector<double> scale{
		static_cast<double>(vertexBuffer.scale.x),
		static_cast<double>(vertexBuffer.scale.y),
		static_cast<double>(vertexBuffer.scale.z),
	};

	std::vector<double> sigma(6);
	GetSigma(scale, rot, sigma);

	for (size_t i = 0; i < 3; i++) {
		sigmasHalf2x16.emplace_back(packHalf2x16(sigma[i * 2], sigma[i * 2 + 1]));
	}
}


uint32_t GSSplatObj::floatToHalf(float f)
{
	FloatIntUnion fiu;
	fiu.f = f;
	uint32_t i = fiu.ui;
	uint32_t sign = (i >> 31) & 0x0001;
	uint32_t exp = (i >> 23) & 0x00ff;
	uint32_t frac = i & 0x007fffff;

	int newExp;
	if (exp == 0) {
		newExp = 0;
	}
	else if (exp < 113) {
		newExp = 0;
		frac |= 0x00800000;
		frac = frac >> (113 - exp);
		if (frac & 0x01000000) {
			newExp = 1;
			frac = 0;
		}
	}
	else if (exp < 142) {
		newExp = exp - 112;
	}
	else {
		newExp = 31;
		frac = 0;
	}
	return (sign << 15) | (newExp << 10) | (frac >> 13);
}

uint32_t GSSplatObj::packHalf2x16(float x, float y)
{
	return (floatToHalf(x) | (floatToHalf(y) << 16));
}

void GSSplatObj::SetUpAttribute()
{
	m_vertexLength = 8;
	m_textureHeight = std::ceil((2.0f * m_vertexCount) / m_textureWidth);
	m_vertices.resize(m_vertexCount);
	m_sorter = std::make_shared<CountingSortCPU<SplatVertex>>(m_vertexCount, m_sortOrder);
	Base3DGSObj::SetUpAttribute();
}

void GSSplatObj::GetVertexCount(std::ifstream& file)
{
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	size_t rowLength = 3 * 4 + 3 * 4 + 4 + 4;
	m_vertexCount = size / rowLength;
}
RENDERABLE_END


