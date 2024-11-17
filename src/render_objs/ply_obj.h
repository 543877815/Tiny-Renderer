#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "common.h"
RENDERABLE_BEGIN

enum SORT_ORDER {
	DESCENDING,
	SDCENDING
};



//struct PackPlyHeader {
//	std::string format;
//	int numVertices = 0;
//	int numFaces = 0;
//	int numChunks = 0;
//	std::unordered_map<std::string, int> element2offest;
//	std::unordered_map<std::string, std::vector<PlyProperty>> element2Properties;
//};

struct PlyVertexStorage {
	glm::vec3 position;
	glm::vec3 normal{ 0.0f, 0.0f, 0.0f };
	float shs[3];
	float opacity;
	glm::vec3 scale;
	glm::vec4 rotation;
};

struct PlyVertex3 {
	glm::vec4 position;
	glm::vec3 normal{ 0.0f, 0.0f, 0.0f };
	float shs[3];
	float opacity;
	glm::vec3 scale;
	glm::vec4 rotation;
};

struct PlyVertex2 {
	glm::vec3 position;
	float shs[27];
	float opacity;
	glm::vec4 rotation;
	glm::vec3 scale;
};

struct PlyVertex1 {
	glm::vec3 position;
	float shs[12];
	float opacity;
	glm::vec4 rotation;
	glm::vec3 scale;
};

struct PlyVertex0 {
	glm::vec3 position;
	float shs[3];
	float opacity;
	glm::vec4 rotation;
	glm::vec3 scale;
};

struct SplatVertex {
	glm::vec3 position;
	glm::vec3 scale;
	uint8_t shs[4];  // 0.5 * SH_C0 * v["f_dc"]
	uint8_t rotation[4];
};

struct SplatBuffer {
	uint8_t data[32];
};

enum MODEL_TYPE
{
	SPLAT, PLY,
	PACKED_PLY
};

class Base3DGSObj : public RenderObjectNaive<glm::vec3>
{
protected:
	//template <typename T> void SortIndices(std::vector<T>& vertices);
	void GenerateTex();

protected:
	uint32_t m_vertexCount, m_vertexLength;
	size_t m_texwidth, m_texheight;
	MODEL_TYPE m_type;
	std::vector<uint32_t> m_texdata;
	std::shared_ptr<Shader> m_renderProgram = nullptr;
	std::shared_ptr<Texture> m_gaussian_texture = nullptr;
	std::vector<size_t> m_gs_indices;
	template <typename T> void SortIndices(std::vector<T>& vertices);

};

class PlyObj : public Base3DGSObj {
public:
	struct PlyProperty {
		std::string type;
		std::string name;
	};

	struct PlyHeader {
		std::string format;
		int numVertices = 0;
		int numFaces = 0;
		int vertex_offset = 0;
		std::vector<PlyProperty> vertexProperties;
		std::vector<PlyProperty> faceProperties;
		std::vector<std::string> verticePropertiesOrder;
		std::unordered_map <std::string, std::pair<size_t, size_t>> verticePropertiesOffset;

		void UpdateVerticesOffset(std::string property, std::pair<size_t, size_t>& start_end) {
			if (verticePropertiesOffset.find(property) == verticePropertiesOffset.end()) {
				verticePropertiesOrder.emplace_back(property);
				verticePropertiesOffset[property] = { start_end.first, start_end.second - start_end.first };
			}
			else {
				verticePropertiesOffset[property].second = start_end.second - verticePropertiesOffset[property].first;
			}
		}
	};

	struct vertexUniform {
		int u_texture;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec2 focal;
		glm::vec2 viewport;
		glm::vec3 cam_pos;
		glm::vec2 tan_fov;
		glm::vec2 projParams;
		int sh_deg;
		int showGaussian;
		int showHotspots;
	};

	PlyObj(Parser::RenderObjConfigNaive& config)
	{
		std::ifstream file(config.model_path);
		LoadHeader(file);
		//SetUpShader(config);	
		SetupAttr();
		LoadVertices(file);
		SortIndices(m_vertices);
		GenerateTexData();
	}

	virtual void Draw(const vertexUniform& uniform,/* const ImguiParameters& imguiParams, */bool isDirty);


private:
	void SetupAttr();
	void LoadHeader(std::ifstream& file);
	void LoadVertices(std::ifstream& file);
	void GenerateTexData();
	void GetSigmaFloat32(glm::vec4& _rotation, glm::vec3& _scale, std::vector<float>& sigmaFloat32);
private:
	PlyHeader m_header;
	MODEL_TYPE m_type;
	uint32_t m_vertexCount, m_vertexLength;
	size_t m_texwidth, m_texheight;
	std::vector<uint32_t> m_texdata;
	std::vector<PlyVertex3> m_vertices;

};


struct MinMax {
	float min;
	float max;
};

MinMax calculateMinMax(const std::vector<float>& data) {
	float min, max;
	min = max = data[0];

	for (float item : data) {
		if (item < min) min = item;
		if (item > max) max = item;
	}
	return { min, max };
}

unsigned int Part1By2(unsigned int x) {
	x &= 0x000003ff;
	x = (x ^ (x << 16)) & 0xff0000ff;
	x = (x ^ (x << 8)) & 0x0300f00f;
	x = (x ^ (x << 4)) & 0x030c30c3;
	x = (x ^ (x << 2)) & 0x09249249;   x = (x ^ (x << 2)) & 0x09249249;
	return x;
}

unsigned int encodeMorton3(unsigned int x, unsigned int y, unsigned int z) {
	return (Part1By2(z) << 2) + (Part1By2(y) << 1) + Part1By2(x);
}


template<typename T>
inline void Base3DGSObj::SortIndices(std::vector<T>& vertices)
{
	m_gs_indices.resize(m_vertexCount);
	for (size_t i = 0; i < m_gs_indices.size(); i++) {
		m_gs_indices[i] = i;
	}
	if (1) { // radix sort
		std::vector<float> x;
		std::vector<float> y;
		std::vector<float> z;
		for (int i = 0; i < m_vertexCount; i++) {
			x.push_back(-vertices[i].position[0]);
			y.push_back(-vertices[i].position[1]);
			z.push_back(vertices[i].position[2]);
		}

		MinMax bx = calculateMinMax(x);
		MinMax by = calculateMinMax(y);
		MinMax bz = calculateMinMax(z);

		std::vector<unsigned int> morton(m_gs_indices.size());
		for (size_t i = 0; i < m_gs_indices.size(); ++i) {
			int idx = m_gs_indices[i];
			unsigned int ix = static_cast<unsigned int>(1024 * (x[idx] - bx.min) / (bx.max - bx.min));
			unsigned int iy = static_cast<unsigned int>(1024 * (y[idx] - by.min) / (by.max - by.min));
			unsigned int iz = static_cast<unsigned int>(1024 * (z[idx] - bz.min) / (bz.max - bz.min));
			morton[i] = encodeMorton3(ix, iy, iz);
		}

		std::sort(m_gs_indices.begin(), m_gs_indices.end(), [&morton, &vertices](const int& a, const int& b) {
			return morton[a] < morton[b];
			});
	}
	else { // fast sort
		std::sort(m_gs_indices.begin(), m_gs_indices.end(), [&](const int& a, const int& b) {
			auto& va = vertices[a];
			auto& vb = vertices[b];
			return va.opacity < vb.opacity;
			});
	}
}


RENDERABLE_END

