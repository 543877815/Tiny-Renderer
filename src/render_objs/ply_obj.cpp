#include "ply_obj.h"
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
	x = (x ^ (x << 2)) & 0x09249249;   x = (x ^ (x << 2)) & 0x09249249;
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

void PlyObj::PlyHeader::UpdateVerticesOffset(std::string property, std::pair<size_t, size_t>& start_end)
{
	if (verticePropertiesOffset.find(property) == verticePropertiesOffset.end()) {
		verticePropertiesOrder.emplace_back(property);
		verticePropertiesOffset[property] = { start_end.first, start_end.second - start_end.first };
	}
	else {
		verticePropertiesOffset[property].second = start_end.second - verticePropertiesOffset[property].first;
	}
}

PlyObj::PlyObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr)
{
	auto configPtr = std::static_pointer_cast<Parser::RenderObjConfig3DGS>(baseConfigPtr);
	SetUpShader(configPtr->vertexShader.c_str(), configPtr->fragmentShader.c_str());
	std::ifstream file(configPtr->modelPath, std::ios::binary);
	LoadModelHeader(file, m_header);
	SetUpAttribute();
	LoadVertices(file);
	PresortIndices(m_vertices);
	GenerateTextureData();
	GenerateTexture();
	SetUpData();
}

void PlyObj::SetUpShader(const char* vertex_shader, const char* fragmentShader)
{
	m_shader = std::make_unique<Shader>(vertex_shader, fragmentShader);
}

void PlyObj::Draw()
{
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, m_vertexCount);
}

void PlyObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	auto projection = std::any_cast<glm::mat4>(uniform.at("projection"));
	auto view = std::any_cast<glm::mat4>(uniform.at("view"));
	auto model = std::any_cast<glm::mat4>(uniform.at("model"));
	auto focal = std::any_cast<glm::vec2>(uniform.at("focal"));
	auto viewport = std::any_cast<glm::vec2>(uniform.at("viewport"));
	auto camPos = std::any_cast<glm::vec3>(uniform.at("camPos"));
	auto tanFov = std::any_cast<glm::vec2>(uniform.at("tanFov"));
	auto projParams = std::any_cast<glm::vec2>(uniform.at("projParams"));

	SetUpGLStatus();

	m_shader->Use();
	m_renderVAO->Bind();
	RunSortUpdateDepth(m_vertices);
	m_gaussian_texture->BindTexture(0);
	m_shader->SetInt("u_texture", m_textureIdx);
	m_shader->SetMat4("projection", projection);
	m_shader->SetMat4("view", view);
	m_shader->SetMat4("model", model);
	m_shader->SetVec2("focal", focal);
	m_shader->SetVec2("viewport", viewport);
	m_shader->SetVec3("camPos", camPos);
	m_shader->SetVec2("tanFov", tanFov);
	m_shader->SetVec2("projParams", projParams);
	m_shader->SetInt("sphericalHarmonicsDegree", 3);
	Draw();
	m_renderVAO->Unbind();

}

void PlyObj::I_RunSortUpdateDepth()
{
	RunSortUpdateDepth(m_vertices);
}

void PlyObj::LoadModelHeader(std::ifstream& file, PlyHeader& header)
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

void PlyObj::LoadVertices(std::ifstream& file)
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

void PlyObj::GenerateTextureData()
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

void PlyObj::GetSigmaFloat32(glm::vec4& rotation, glm::vec3& scale, std::vector<float>& sigmaFloat32)
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

void PlyObj::SetUpAttribute()
{
	m_vertexCount = m_header.verticeNum;
	m_vertices.resize(m_vertexCount);
	Base3DGSObj::SetUpAttribute();
}

void Base3DGSObj::GenerateTexture()
{
	Texture::Params m_texture_parameters;
	m_texture_parameters.minFilter = FilterType::Nearest;
	m_texture_parameters.magFilter = FilterType::Nearest;
	m_texture_parameters.sWrap = WrapType::ClampToEdge;
	m_texture_parameters.tWrap = WrapType::ClampToEdge;
	m_textureIdx = m_gaussian_texture->GenerateTexture(m_texture_width, m_textureHeight,
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
	m_vertexLength = 64;
	m_texture_width = 1024 * 2;
	m_textureHeight = std::ceil((2.0f * m_vertexCount) / m_texture_width) * 8;
	m_textureData.resize(m_texture_width * m_textureHeight * 4);
	m_gaussian_texture = std::make_shared<Texture>(1);
	m_depthIndex.resize(m_vertexCount);
	m_indices.resize(m_vertexCount);
	m_sizeList.resize(m_vertexCount);
}


RENDERABLE_END

