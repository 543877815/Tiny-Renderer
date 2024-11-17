#include "ply_obj.h"

RENDERABLE_BEGIN
static const float SH_C0 = 0.28209479177387814f;
static const float SH_C1 = 0.4886025119029199f;
static const float SH_C2[] = {
	1.0925484305920792f,
	-1.0925484305920792f,
	0.31539156525252005f,
	-1.0925484305920792f,
	0.5462742152960396f
};
static const float SH_C3[] = {
	-0.5900435899266435f,
	2.890611442640554f,
	-0.4570457994644658f,
	0.3731763325901154f,
	-0.4570457994644658f,
	1.445305721320277f,
	-0.5900435899266435f
};

template<typename T>
void GetSigma(std::vector<T>& scale, std::vector<T>& rot, std::vector<T>& sigma)
{
	std::vector<T> M{
		1.0 - 2.0 * (rot[2] * rot[2] + rot[3] * rot[3]),
		2.0 * (rot[1] * rot[2] + rot[0] * rot[3]),
		2.0 * (rot[1] * rot[3] - rot[0] * rot[2]),

		2.0 * (rot[1] * rot[2] - rot[0] * rot[3]),
		1.0 - 2.0 * (rot[1] * rot[1] + rot[3] * rot[3]),
		2.0 * (rot[2] * rot[3] + rot[0] * rot[1]),

		2.0 * (rot[1] * rot[3] + rot[0] * rot[2]),
		2.0 * (rot[2] * rot[3] - rot[0] * rot[1]),
		1.0 - 2.0 * (rot[1] * rot[1] + rot[2] * rot[2]),
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

void PlyObj::Draw(const vertexUniform& uniform, bool isDirty)
{

}

void PlyObj::SetupAttr()
{
	m_type = MODEL_TYPE::PLY;
	m_vertexCount = m_header.numVertices;
	m_vertexLength = 64;
	m_texwidth = 1024 * 2;
	m_texheight = std::ceil((2.0f * m_vertexCount) / m_texwidth) * 8;
	m_texdata.resize(m_texwidth * m_texheight * 4);
	m_gaussian_texture = std::make_shared<Texture>(1);
}

void PlyObj::LoadHeader(std::ifstream& file)
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
			iss >> m_header.format;
		}
		else if (token == "element") {
			iss >> token;

			if (token == "vertex") {
				iss >> m_header.numVertices;
			}
			else if (token == "face") {
				iss >> m_header.numFaces;
			}
		}
		else if (token == "property") {
			PlyProperty property;
			iss >> property.type >> property.name;

			if (m_header.vertexProperties.size() < static_cast<size_t>(m_header.numVertices)) {
				m_header.vertexProperties.push_back(property);
			}
			else {
				m_header.faceProperties.push_back(property);
			}
			std::pair<size_t, size_t> start_end = { m_header.vertex_offset, 0 };

			if (property.type == "char" || property.type == "int8")
				m_header.vertex_offset += 1;
			else if (property.type == "uchar" || property.type == "uint8")
				m_header.vertex_offset += 1;
			else if (property.type == "short" || property.type == "int16")
				m_header.vertex_offset += 2;
			else if (property.type == "ushort" || property.type == "uint16")
				m_header.vertex_offset += 2;
			else if (property.type == "int" || property.type == "int32")
				m_header.vertex_offset += 4;
			else if (property.type == "uint" || property.type == "uint32")
				m_header.vertex_offset += 4;
			else if (property.type == "float" || property.type == "float32")
				m_header.vertex_offset += 4;
			else if (property.type == "double" || property.type == "float64")
				m_header.vertex_offset += 8;
			else
				std::cout << "Unsupported type " << property.type.c_str() << " for property" << property.name << std::endl;

			start_end.second = m_header.vertex_offset;

			std::string key;
			if (property.name == "x" || property.name == "y" || property.name == "z")
				m_header.UpdateVerticesOffset("position", start_end);
			else if (property.name == "nx" || property.name == "ny" || property.name == "nz")
				m_header.UpdateVerticesOffset("normal", start_end);
			else if (property.name == "opacity")
				m_header.UpdateVerticesOffset("opacity", start_end);
			else if (property.name.find("rot") != std::string::npos)
				m_header.UpdateVerticesOffset("rot", start_end);
			else if (property.name.find("scale") != std::string::npos)
				m_header.UpdateVerticesOffset("scale", start_end);
			else if (property.name.find("f_rest") != std::string::npos || property.name.find("f_dc") != std::string::npos)
				m_header.UpdateVerticesOffset("shs", start_end);
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
	m_vertices.resize(m_header.numVertices);
	const float SH_C0 = 0.28209479177387814f;
	for (auto i = 0; i < m_header.numVertices; i++) {
		//static_assert(sizeof(PlyVertexStorage) == 62 * sizeof(float), "");
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

void PlyObj::GenerateTexData()
{
	for (size_t i = 0; i < m_vertexCount; i++) {
		size_t idx = m_gs_indices[i];
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
		std::copy_n(reinterpret_cast<float*>(&m_vertices[idx].position), 4, reinterpret_cast<float*>(&m_texdata[m_vertexLength * i]));
		std::copy_n(reinterpret_cast<float*>(sigmas.data()), 6, reinterpret_cast<float*>(&m_texdata[m_vertexLength * i + 4]));
		std::copy_n(reinterpret_cast<uint8_t*>(&shs_uint8), 4, reinterpret_cast<uint8_t*>(&m_texdata[m_vertexLength * i + 10]));
		std::copy_n(reinterpret_cast<float*>(&m_vertices[idx].opacity), 1, reinterpret_cast<float*>(&m_texdata[m_vertexLength * i + 11]));
		std::copy_n(reinterpret_cast<float*>(&m_vertices[idx].shs), (std::max)(static_cast<int>(m_header.verticePropertiesOffset["shs"].second / sizeof(float)), 3),
			reinterpret_cast<float*>(&m_texdata[m_vertexLength * i + 12]));
	}
	GenerateTex();
}

void PlyObj::GetSigmaFloat32(glm::vec4& _rotation, glm::vec3& _scale, std::vector<float>& sigmaFloat32)
{
	std::vector<double> rot{
		static_cast<double>(_rotation[0]),
		static_cast<double>(_rotation[1]),
		static_cast<double>(_rotation[2]),
		static_cast<double>(_rotation[3]),
	};

	std::vector<double> scale{
		static_cast<double>(_scale.x),
		static_cast<double>(_scale.y),
		static_cast<double>(_scale.z),
	};

	std::vector<double> sigma(6);
	GetSigma(scale, rot, sigma);

	for (size_t i = 0; i < 6; i++) {
		sigmaFloat32.emplace_back(sigma[i]);
	}
}

void Base3DGSObj::GenerateTex()
{
	Texture::Params m_texParams;
	m_texParams.minFilter = FilterType::Nearest;
	m_texParams.magFilter = FilterType::Nearest;
	m_texParams.sWrap = WrapType::ClampToEdge;
	m_texParams.tWrap = WrapType::ClampToEdge;
	m_gaussian_texture->GenerateTexture(m_texwidth, m_texheight, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, m_texParams, m_texdata.data());
}

RENDERABLE_END

