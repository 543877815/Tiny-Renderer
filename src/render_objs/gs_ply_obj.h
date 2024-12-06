#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cfloat>
#include "common.h"
#include "../draw/vertexbuffer.h"
#include "../draw/camera.h"

RENDERABLE_BEGIN
enum SORT_ORDER : uint32_t
{
	DESCENDING,
	SDCENDING
};

enum SORT_METHOD : uint32_t
{
	COUNTING_SORT,
	QUICK_SORT,
	RADIX_SORT
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

template <typename T>
class BaseSorter {
public:
	BaseSorter() {}
	BaseSorter(uint32_t vertexCount, SORT_ORDER sortOrder) :m_vertexCount(vertexCount), m_sortOrder(sortOrder) {}
	virtual void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex) = 0;

protected:
	uint32_t m_vertexCount = 0;
	SORT_ORDER m_sortOrder = DESCENDING;
};

template <typename T>
class QuickSortCPU : public BaseSorter<T> {
public:
	QuickSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder)
	{
		this->m_vertexCount = vertexCount;
		this->m_sortOrder = sortOrder;
		m_index2depth.resize(vertexCount);
	}

	void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex) override
	{
		auto instance = Camera::GetInstance();
		auto modelViewProjMatrix = instance->GetProjMat() * instance->GetViewMat();

		m_index2depth.resize(this->m_vertexCount);
		std::memset(m_index2depth.data(), 0, sizeof(float) * m_index2depth.size());

		for (size_t i = 0; i < this->m_vertexCount; i++) {
			size_t idx = indices[i];

			m_index2depth[i] = std::make_pair(i,
				modelViewProjMatrix[0][2] * vertices[idx].position.x +
				modelViewProjMatrix[1][2] * vertices[idx].position.y +
				modelViewProjMatrix[2][2] * vertices[idx].position.z);
		}

		if (this->m_sortOrder == DESCENDING) {
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

		for (size_t i = 0; i < this->m_vertexCount; i++) {
			auto [idx, _] = m_index2depth[i];
			depthIndex[i] = idx;
		}
	}
private:
	std::vector<std::pair<uint32_t, float>> m_index2depth{};
};

template <typename T>
class RadixSortCPU : public BaseSorter<T> {
public:
	RadixSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder)
	{
		this->m_vertexCount = vertexCount;
		this->m_sortOrder = sortOrder;
		m_totalIter = (m_bit + m_bitPerIter - 1) / m_bitPerIter;
		m_bucketSize = 1 << m_bitPerIter;
		m_index2depth1.resize(vertexCount);
		m_index2depth2.resize(vertexCount);
		m_presum.resize(m_bucketSize);
		m_summation.resize(m_bucketSize);
		m_histogram.resize(m_bucketSize);
	}

	void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex) override
	{
		auto instance = Camera::GetInstance();
		float plane_far = instance->GetFar();
		float half_far = plane_far / 2.0f;
		auto modelViewProjMatrix = instance->GetProjMat() * instance->GetViewMat();
		const uint32_t MAX_DEPTH = UINT32_MAX;
		for (size_t i = 0; i < this->m_vertexCount; i++) {
			size_t idx = indices[i];
			auto& pos = vertices[idx].position;
			float depth = (modelViewProjMatrix[0][2] * pos.x +
				modelViewProjMatrix[1][2] * pos.y +
				modelViewProjMatrix[2][2] * pos.z);
			m_index2depth1[i] = { i, MAX_DEPTH - static_cast<uint32_t>((depth - half_far) / plane_far * MAX_DEPTH) };
		}

		for (size_t iter = 0; iter < m_totalIter; iter++)
		{
			std::fill(m_histogram.begin(), m_histogram.end(), std::vector<std::pair<uint32_t, uint32_t>>{});
			std::fill(m_presum.begin(), m_presum.end(), 0);
			std::fill(m_summation.begin(), m_summation.end(), 0);
			auto& source = (iter % 2 == 0) ? m_index2depth1 : m_index2depth2;
			auto& dest = (iter % 2 == 0) ? m_index2depth2 : m_index2depth1;
			uint32_t shift = iter * m_bitPerIter;

			// histogram
			for (auto elem : source)
			{
				auto& [_, depth] = elem;
				uint32_t bucketId = (depth >> shift) & (m_bucketSize - 1);
				m_histogram[bucketId].emplace_back(elem);
			}

			// presum
			for (uint32_t id = 0; id < m_bucketSize; id++)
			{
				if (id == 0)
					m_presum[id] = m_histogram[id].size();
				else
					m_presum[id] = m_presum[id - 1] + m_histogram[id].size();
				m_summation[id] = m_histogram[id].size();
			}

			// sort
			for (auto elem : source)
			{
				auto& [_, depth] = elem;
				uint32_t bucketId = (depth >> shift) & (m_bucketSize - 1);
				dest[m_presum[bucketId] - 1] = m_histogram[bucketId][m_summation[bucketId] - 1];
				m_presum[bucketId]--;
				m_summation[bucketId]--;
			}
		}

		auto& finalBuffer = (m_totalIter % 2 == 0) ? m_index2depth1 : m_index2depth2;
		for (uint32_t i = 0; i < this->m_vertexCount; i++)
		{
			auto [idx, _] = finalBuffer[i];
			depthIndex[i] = idx;
		}
	}

private:
	const uint32_t m_bit = 32;
	const uint32_t m_bitPerIter = 8;
	uint32_t m_bucketSize = 0;
	uint32_t m_totalIter = 0;
	std::vector<std::pair<uint32_t, uint32_t>> m_index2depth1{};
	std::vector<std::pair<uint32_t, uint32_t>> m_index2depth2{};
	std::vector<float> m_depth{};
	std::vector<std::vector<std::pair<uint32_t, uint32_t>>> m_histogram{};
	std::vector<uint32_t> m_presum{};
	std::vector<uint32_t> m_summation{};
};

template <typename T>
class CountingSortCPU : public BaseSorter<T> {
public:
	CountingSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder)
	{
		this->m_vertexCount = vertexCount;
		this->m_sortOrder = sortOrder;
		m_sizeList.resize(vertexCount);
		std::memset(m_sizeList.data(), 0, sizeof(int32_t) * vertexCount);
		m_depth.resize(vertexCount);
		std::memset(m_depth.data(), 0, sizeof(float) * vertexCount);
	}

	void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex)override
	{
		auto instance = Camera::GetInstance();
		auto modelViewProjMatrix = instance->GetProjMat() * instance->GetViewMat();
		float maxDepth = FLT_MIN;
		float minDepth = FLT_MAX;

		std::memset(m_sizeList.data(), 0, sizeof(uint32_t) * m_sizeList.size());
		for (uint32_t i = 0; i < this->m_vertexCount; i++) {
			size_t idx = indices[i];
			auto& pos = vertices[idx].position;
			m_depth[i] = (modelViewProjMatrix[0][2] * pos.x +
				modelViewProjMatrix[1][2] * pos.y +
				modelViewProjMatrix[2][2] * pos.z);
			maxDepth = (std::max)(maxDepth, m_depth[i]);
			minDepth = (std::min)(minDepth, m_depth[i]);
		}

		uint32_t sortBit = 256 * 256;
		uint32_t count = sortBit + 1; // +1 防止越界

		if (m_counts.empty()) m_counts.resize(count);
		std::memset(m_counts.data(), 0, sizeof(uint32_t) * m_counts.size());
		for (uint32_t i = 0; i < this->m_vertexCount; i++)
		{
			m_sizeList[i] = (maxDepth - m_depth[i]) / (maxDepth - minDepth + 0.01) * sortBit;
			m_counts[m_sizeList[i]]++;
		}

		if (m_starts.empty()) m_starts.resize(count);
		std::memset(m_starts.data(), 0, sizeof(uint32_t) * m_starts.size());
		for (uint32_t i = 1; i < sortBit; i++)
		{
			m_starts[i] = m_starts[i - 1] + m_counts[i - 1];
		}

		for (uint32_t i = 0; i < this->m_vertexCount; i++)
		{
			if (this->m_sortOrder == DESCENDING)
				depthIndex[m_starts[m_sizeList[i]]++] = i;
			else
				depthIndex[m_starts[m_sizeList[i]]++] = this->m_vertexCount - i - 1;
		}
	}
private:
	std::vector<float> m_depth{};
	std::vector<int32_t> m_sizeList{};
	std::vector<uint32_t> m_counts{};
	std::vector<uint32_t> m_starts{};
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
	glm::vec2 GetNearFar() { return glm::vec2(m_near, m_far); }
	const glm::mat4& GetProjection() const { return m_projection; }
private:
	float m_fy = 1164.66f;  // focal_y
	float m_fx = 1159.58f; // focal_x 
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
	template <typename T> void PresortIndices(std::vector<T>& vertices);
	void ImGuiCallback() override;

protected:
	uint32_t m_vertexCount = 0, m_vertexLength = 0;
	int m_texture_width = 0, m_textureHeight = 0;
	MODEL_TYPE m_type;
	SORT_METHOD m_sortMethod = COUNTING_SORT;
	int m_textureIdx = -1;
	SORT_ORDER m_sortOrder = DESCENDING;
	std::vector<uint32_t> m_depthIndex{};
	std::vector<uint32_t> m_textureData{};
	std::vector<uint32_t> m_indices{};
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

class GSPlyObj : public Base3DGSObj {
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

	GSPlyObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void SetUpShader(const char* vertexShader, const char* fragmentShader);
	virtual void Draw();
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	void ImGuiCallback() override;
	void RunSortUpdateDepth();
private:
	void LoadModelHeader(std::ifstream& file, PlyHeader& header);
	void LoadVertices(std::ifstream& file);
	void GenerateTextureData();
	void GetSigmaFloat32(glm::vec4& rotation, glm::vec3& scale, std::vector<float>& sigmaFloat32);
	void SetUpAttribute();

private:
	PlyHeader m_header;
	int m_sphericalHarmonicsDegree = 3;
	MODEL_TYPE m_type = MODEL_TYPE::PLY;
	std::vector<PlyVertex3> m_vertices;
	std::shared_ptr<BaseSorter<PlyVertex3>> m_sorter = nullptr;
};

template<typename T>
inline void Base3DGSObj::PresortIndices(std::vector<T>& vertices)
{
	for (uint32_t i = 0; i < m_indices.size(); i++) {
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

RENDERABLE_END