#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "common.h"
#include "../draw/vertexbuffer.h"
#include "../draw/camera.h"


RENDERABLE_BEGIN
enum SORT_ORDER : uint32_t {
	DESCENDING,
	SDCENDING
};

struct PlyVertexStorage {
	glm::vec3 position;
	glm::vec3 normal{ 0.0f, 0.0f, 0.0f };
	float shs[48];
	float opacity;
	glm::vec3 scale;
	glm::vec4 rotation;
};

struct PlyVertex3 {
	glm::vec4 position;
	glm::vec3 normal{ 0.0f, 0.0f, 0.0f };
	float shs[48];
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

enum MODEL_TYPE : uint32_t
{
	SPLAT,
	PLY
};

class Base3DGSCamera
{
public:
	static std::shared_ptr<Base3DGSCamera> GetInstance();
	float GetFx() { return m_fx; }
	float GetFy() { return m_fy; }
	float GetWidth() { return m_width; }
	float GetHeight() { return m_height; }
	float GetNear() { return m_near; }
	float GetFar() { return m_far; }
	glm::vec2 GetFocal() { return glm::vec2(m_fx, m_fy); }
	glm::vec2 GetTanFov() { return glm::vec2(m_width / m_fx * 0.5f, m_height / m_fy * 0.5f); }
	glm::vec2 GetProjParams() { return glm::vec2(m_near, m_far); }
	const glm::mat4& GetProjection() const { return m_projection; }

private:
	float m_fy = 1040.0f;  // focal_y
	float m_fx = 1040.0f; // focal_x 
	float m_width = 1024.0f;
	float m_height = 768.0f;
	float m_near = 0.1f;
	float m_far = 1000.0f;
	static inline std::shared_ptr<Base3DGSCamera> m_instance = nullptr;
	glm::mat4 m_projection = glm::mat4(1.0f);
	void SetProjection()
	{
		m_projection = glm::mat4{
			(2.0f * m_fx) / m_width, 0.0f, 0.0f,0.0f,
			0.0f, -(2.0f * m_fy) / m_height, 0.0f, 0.0f,
			0.0f, 0.0f, m_far / (m_far - m_near), 1.0f,
			0.0f, 0.0f, -(m_far * m_near) / (m_far - m_near), 0.0f
		};
	}

};

class Base3DGSObj : public RenderObjectBase
{
protected:
	virtual void GenerateTexture();
	virtual void SetUpData();
	virtual void SetUpGLStatus();
	void SetUpAttribute();
	virtual void I_RunSortUpdateDepth() = 0;
	template <typename T> void PresortIndices(std::vector<T>& vertices);
	template <typename T> void RunSortUpdateDepth(std::vector<T>& vertices);
	template <typename T> void RadixSort(std::vector<T>& vertices);
	template <typename T> void QuickSort(std::vector<T>& vertices);

protected:
	uint32_t m_vertexCount = 0, m_vertexLength = 0;
	int m_texture_width = 0, m_textureHeight = 0;
	MODEL_TYPE m_type;
	bool m_useRadixSort = false;
	size_t m_textureIdx = -1;
	SORT_ORDER m_sortOrder = DESCENDING;
	std::vector<uint32_t> m_depthIndex{};
	std::vector<uint32_t> m_textureData{};
	std::vector<int32_t> m_sizeList{};
	std::vector<uint32_t> m_counts{};
	std::vector<uint32_t> m_starts{};
	std::vector<uint32_t> m_indices{};
	std::vector<std::pair<uint32_t, float>> m_index2depth{};
	std::shared_ptr<Shader> m_shader = nullptr;
	std::shared_ptr<Texture> m_gaussian_texture = nullptr;
	std::shared_ptr<VertexArrayObject> m_renderVAO = nullptr;
	std::shared_ptr<VertexBufferObject> m_rectangleVBO = nullptr;
	std::shared_ptr<VertexBufferObject> m_depthIndexVBO = nullptr;

private:
	std::pair<float, float> calculateMinMax(const std::vector<float>& data);
	unsigned int Part1By2(unsigned int x);
	unsigned int encodeMorton3(unsigned int x, unsigned int y, unsigned int z);
};

class PlyObj : public Base3DGSObj {
public:
	struct PlyProperty {
		std::string type;
		std::string name;
	};

	struct PlyHeader {
		std::string format = "";
		int verticeNum = 0;
		int face_number = 0;
		int vertex_offset = 0;
		std::vector<PlyProperty> vertexProperties{};
		std::vector<PlyProperty> faceProperties{};
		std::vector<std::string> verticePropertiesOrder{};
		std::unordered_map <std::string, std::pair<size_t, size_t>> verticePropertiesOffset{};
		void UpdateVerticesOffset(std::string property, std::pair<size_t, size_t>& startEnd);
	};

	PlyObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void SetUpShader(const char* vertexShader, const char* fragmentShader);
	virtual void Draw();
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);

private:
	void I_RunSortUpdateDepth() override;
	void LoadModelHeader(std::ifstream& file, PlyHeader& header);
	void LoadVertices(std::ifstream& file);
	void GenerateTextureData();
	void GetSigmaFloat32(glm::vec4& rotation, glm::vec3& scale, std::vector<float>& sigmaFloat32);
	void SetUpAttribute();

private:
	PlyHeader m_header;
	MODEL_TYPE m_type = MODEL_TYPE::PLY;
	std::vector<PlyVertex3> m_vertices;
};

template<typename T>
inline void Base3DGSObj::PresortIndices(std::vector<T>& vertices)
{
	for (size_t i = 0; i < m_indices.size(); i++) {
		m_indices[i] = i;
	}
	std::vector<float> x;
	std::vector<float> y;
	std::vector<float> z;
	for (int i = 0; i < m_vertexCount; i++) {
		x.push_back(-vertices[i].position[0]);
		y.push_back(-vertices[i].position[1]);
		z.push_back(vertices[i].position[2]);
	}

	auto bx = calculateMinMax(x);
	auto by = calculateMinMax(y);
	auto bz = calculateMinMax(z);

	std::vector<unsigned int> morton(m_indices.size());
	for (size_t i = 0; i < m_indices.size(); ++i) {
		int idx = m_indices[i];
		unsigned int ix = static_cast<unsigned int>(1024 * (x[idx] - bx.first) / (bx.second - bx.first));
		unsigned int iy = static_cast<unsigned int>(1024 * (y[idx] - by.first) / (by.second - by.first));
		unsigned int iz = static_cast<unsigned int>(1024 * (z[idx] - bz.first) / (bz.second - bz.first));
		morton[i] = encodeMorton3(ix, iy, iz);
	}

	std::sort(m_indices.begin(), m_indices.end(), [&morton, &vertices](const int& a, const int& b) {
		return morton[a] < morton[b];
		});
}

template<typename T>
inline void Base3DGSObj::RunSortUpdateDepth(std::vector<T>& vertices)
{
	if (m_useRadixSort) {
		RadixSort(vertices);
	}
	else {
		QuickSort(vertices);
	}
	m_depthIndexVBO->Update(m_depthIndex);
}

template <typename T>
inline void Base3DGSObj::RadixSort(std::vector<T>& vertices)
{
	auto instance = Camera::GetInstance();
	auto modelViewProjMatrix = instance->GetProjectionMatrix() * instance->GetViewMatrix();
	int32_t maxDepth = 0x80000000;
	int32_t minDepth = 0x7fffffff;
	m_sizeList.resize(m_vertexCount);
	std::memset(m_sizeList.data(), 0, sizeof(uint32_t) * m_sizeList.size());

	for (size_t i = 0; i < m_vertexCount; i++) {
		size_t idx = (m_type == SPLAT) ? i : m_indices[i];
		auto& pos = vertices[idx].position;
		float depth_f = (modelViewProjMatrix[0][2] * pos.x +
			modelViewProjMatrix[1][2] * pos.y +
			modelViewProjMatrix[2][2] * pos.z) * 4096.0;

		m_sizeList[i] = static_cast<int32_t>(depth_f);
		maxDepth = (std::max)(maxDepth, m_sizeList[i]);
		minDepth = (std::min)(minDepth, m_sizeList[i]);
	}

	uint32_t sortBit = 256 * 256;
	float depthInv = static_cast<float>(sortBit) / static_cast<float>(maxDepth - minDepth);

	size_t count = sortBit + 1; // +1 防止越界
	if (m_counts.empty()) m_counts.resize(count);
	std::memset(m_counts.data(), 0, sizeof(uint32_t) * m_counts.size());

	for (size_t i = 0; i < m_vertexCount; i++) {
		if (m_sortOrder == DESCENDING) {
			m_sizeList[i] = static_cast<int32_t>((maxDepth - m_sizeList[i]) * depthInv); // 降序
		}
		else {
			m_sizeList[i] = static_cast<int32_t>((m_sizeList[i] - minDepth) * depthInv); // 升序
		}
		m_counts[m_sizeList[i]]++;
	}

	if (m_starts.empty()) m_starts.resize(count);
	std::memset(m_starts.data(), 0, sizeof(uint32_t) * m_starts.size());
	for (size_t i = 1; i < sortBit; i++) {
		m_starts[i] = m_starts[i - 1] + m_counts[i - 1];
	}

	std::memset(m_depthIndex.data(), 0, sizeof(uint32_t) * m_depthIndex.size());
	for (size_t i = 0; i < m_vertexCount; i++) {
		m_depthIndex[m_starts[m_sizeList[i]]++] = i;
	}

	std::cout << m_depthIndex[0] << " " << m_depthIndex[1] << " " << m_depthIndex[2] << std::endl;
};



template <typename T>
inline void Base3DGSObj::QuickSort(std::vector<T>& m_vertices)
{
	auto instance = Camera::GetInstance();
	auto modelViewProjMatrix = instance->GetProjectionMatrix() * instance->GetViewMatrix();

	m_index2depth.resize(m_vertexCount);
	std::memset(m_index2depth.data(), 0, sizeof(float) * m_index2depth.size());

	for (size_t i = 0; i < m_vertexCount; i++) {
		size_t idx = (m_type == SPLAT) ? i : m_indices[i];

		m_index2depth[i] = std::make_pair(i,
			modelViewProjMatrix[0][2] * m_vertices[idx].position.x +
			modelViewProjMatrix[1][2] * m_vertices[idx].position.y +
			modelViewProjMatrix[2][2] * m_vertices[idx].position.z);
	}

	if (m_sortOrder == DESCENDING) {
		std::sort(m_index2depth.begin(), m_index2depth.end(),
			[&](std::pair<uint32_t, float>& a, std::pair<uint32_t, float>& b) {
				return a.second > b.second;
			});
	}
	else {
		std::sort(m_index2depth.begin(), m_index2depth.end(),
			[&](std::pair<uint32_t, float>& a, std::pair<uint32_t, float>& b) {
				return a.second < b.second;
			});
	}

	std::memset(m_depthIndex.data(), 0, sizeof(uint32_t) * m_depthIndex.size());
	for (size_t i = 0; i < m_vertexCount; i++) {
		auto [_idx, _depth] = m_index2depth[i];
		m_depthIndex[i] = _idx;
	}
};

RENDERABLE_END

