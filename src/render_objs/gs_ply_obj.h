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
#include "./gs_framebuffer_obj.h"
#include "../draw/shader_c.h"
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
	RADIX_SORT,
	GPU_SINGLE_RADIX_SORT,
	GPU_MULTI_RADIX_SORT
};
template <typename T>
class BaseSorter {
public:
	BaseSorter() {}
	BaseSorter(uint32_t vertexCount, SORT_ORDER sortOrder) :m_vertexCount(vertexCount), m_sortOrder(sortOrder) {}
	virtual void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo) = 0;

protected:
	uint32_t m_vertexCount = 0;
	SORT_ORDER m_sortOrder = DESCENDING;
};

template <typename T>
class SingleRadixSortGPU : public BaseSorter<T>
{
public:
	SingleRadixSortGPU(uint32_t vertexCount, SORT_ORDER sortOrder)
	{
		this->m_vertexCount = vertexCount;
		this->m_sortOrder = sortOrder;
		m_preSortProg = std::make_shared<ComputeShader>("./shader/presort_comp.glsl");
		m_singleRadixSortProg = std::make_shared<ComputeShader>("./shader/single_radixsort_comp.glsl");
	}
	void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo)
	{
		if (m_depthVec.empty())
		{
			m_posVec.resize(this->m_vertexCount);
			m_indexVec.resize(this->m_vertexCount);
			m_depthVec.resize(this->m_vertexCount);
			m_atomicCounterVec.resize(1);
			for (size_t i = 0; i < this->m_vertexCount; i++)
			{
				m_posVec[i] = glm::vec4(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, 1.0f);
				m_depthVec[i] = i;
			}
			m_keyBuffer = std::make_shared<VertexBufferObject>(GL_SHADER_STORAGE_BUFFER, m_depthVec, GL_DYNAMIC_STORAGE_BIT);
			m_key2Buffer = std::make_shared<VertexBufferObject>(GL_SHADER_STORAGE_BUFFER, m_depthVec, GL_DYNAMIC_STORAGE_BIT);
			m_valBuffer = std::make_shared<VertexBufferObject>(GL_SHADER_STORAGE_BUFFER, m_indexVec, GL_DYNAMIC_STORAGE_BIT);
			m_val2Buffer = std::make_shared<VertexBufferObject>(GL_SHADER_STORAGE_BUFFER, m_indexVec, GL_DYNAMIC_STORAGE_BIT);
			m_posBuffer = std::make_shared<VertexBufferObject>(GL_SHADER_STORAGE_BUFFER, m_posVec, GL_DYNAMIC_STORAGE_BIT);
			m_atomicCounterBuffer = std::make_shared<VertexBufferObject>(GL_SHADER_STORAGE_BUFFER, m_atomicCounterVec, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
		}

		{
			const uint32_t MAX_DEPTH = UINT32_MAX;
			auto instance = Camera::GetInstance();
			glm::vec2 nearFar = glm::vec2(instance->GetNear(), instance->GetFar());
			auto modelViewProjMatrix = instance->GetProjMat() * instance->GetViewMat();

			m_preSortProg->Use();
			m_preSortProg->SetMat4("modelViewProj", modelViewProjMatrix);
			m_preSortProg->SetVec2("nearFar", nearFar);
			m_preSortProg->SetUInt("keyMax", MAX_DEPTH);

			// reset counter back to zero
			m_atomicCounterVec[0] = 0;
			m_atomicCounterBuffer->Update(m_atomicCounterVec);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBuffer->GetObj());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_keyBuffer->GetObj());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_valBuffer->GetObj());
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, m_atomicCounterBuffer->GetObj());

			const uint32_t LOCAL_SIZE = 256;
			glDispatchCompute(static_cast<uint32_t>((this->m_vertexCount + LOCAL_SIZE - 1) / LOCAL_SIZE), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
		}

		{
			m_atomicCounterBuffer->Read(m_atomicCounterVec);
			assert(m_atomicCounterVec[0] <= this->m_vertexCount);
		}

		{
			m_singleRadixSortProg->Use();
			m_singleRadixSortProg->SetUInt("g_num_elements", m_atomicCounterVec[0]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_keyBuffer->GetObj());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_key2Buffer->GetObj());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_valBuffer->GetObj());
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, m_val2Buffer->GetObj());
			glDispatchCompute(1, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
		}

		{
			glBindBuffer(GL_COPY_READ_BUFFER, m_valBuffer->GetObj());
			glBindBuffer(GL_COPY_WRITE_BUFFER, vbo->GetObj());
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, this->m_vertexCount * sizeof(uint32_t));
		}
	}

private:
	std::shared_ptr<ComputeShader> m_preSortProg = nullptr;
	std::shared_ptr<ComputeShader> m_singleRadixSortProg = nullptr;
	std::shared_ptr<VertexBufferObject> m_keyBuffer = nullptr;
	std::shared_ptr<VertexBufferObject> m_key2Buffer = nullptr;
	std::shared_ptr<VertexBufferObject> m_valBuffer = nullptr;
	std::shared_ptr<VertexBufferObject> m_val2Buffer = nullptr;
	std::shared_ptr<VertexBufferObject> m_posBuffer = nullptr;
	std::shared_ptr<VertexBufferObject> m_atomicCounterBuffer = nullptr;
	std::vector<glm::vec4> m_posVec = {};
	std::vector<uint32_t> m_indexVec = {};
	std::vector<uint32_t> m_depthVec = {};
	std::vector<uint32_t> m_atomicCounterVec = {};
};

template <typename T>
class QuickSortCPU : public BaseSorter<T>
{
public:
	QuickSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder);
	void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo) override;
private:
	std::vector<std::pair<uint32_t, float>> m_index2depth{};
};

template <typename T>
class RadixSortCPU : public BaseSorter<T>
{
public:
	RadixSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder);
	void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo) override;

private:
	const uint32_t m_bit = 32;
	const uint32_t m_bitPerIter = 8;  // 8 for GPU
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
class CountingSortCPU : public BaseSorter<T>
{
public:
	CountingSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder);
	void Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo) override;

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
	enum MODEL_TYPE : uint32_t
	{
		SPLAT,
		PLY
	};
	void SetUpShader(const char* vertexShader, const char* fragmentShader);
	virtual void GenerateTexture();
	virtual void SetUpData();
	virtual void SetUpGLStatus();
	void SetUpAttribute();
	template <typename T> void PresortIndices(std::vector<T>& vertices);
	void ImGuiCallback() override;
	void Draw();

protected:
	uint32_t m_vertexCount = 0, m_vertexLength = 0;
	int m_textureWidth = 1024 * 2, m_textureHeight = 0;
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
	GSPlyObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	void ImGuiCallback() override;

private:
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

	void LoadModelHeader(std::ifstream& file, PlyHeader& header);
	void LoadVertices(std::ifstream& file);
	void GenerateTextureData();
	void GetSigmaFloat32(glm::vec4& rotation, glm::vec3& scale, std::vector<float>& sigmaFloat32);
	void SetUpAttribute();
	void RunSortUpdateDepth();
	void SetUpFbo(const char* vertexShader, const char* fragmentShader);

private:
	PlyHeader m_header;
	int m_sphericalHarmonicsDegree = 3;
	MODEL_TYPE m_type = MODEL_TYPE::PLY;
	std::vector<PlyVertex3> m_vertices;
	std::shared_ptr<BaseSorter<PlyVertex3>> m_sorter = nullptr;
	std::shared_ptr<GSFrameBufferObj> m_fbo = nullptr;
};

template<typename T>
inline void Base3DGSObj::PresortIndices(std::vector<T>& vertices)
{
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

class GSSplatObj : public Base3DGSObj {
public:
	GSSplatObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	void ImGuiCallback() override;

private:
	struct SplatVertex {
		glm::vec3 position;
		glm::vec3 scale;
		uint8_t shs[4];  // 0.5 * SH_C0 * v["f_dc"]
		uint8_t rotation[4];
	};

	union FloatIntUnion {
		float f;
		uint32_t ui;
	};

	void SetUpAttribute();
	void GetVertexCount(std::ifstream& file);
	void GetSigmaHalf2x16(SplatVertex& vertexBuffer, std::vector<uint32_t>& sigmasHalf2x16);
	void RunSortUpdateDepth();
	void LoadVertices(std::ifstream& file);
	uint32_t floatToHalf(float f);
	uint32_t packHalf2x16(float x, float y);

private:
	MODEL_TYPE m_type = MODEL_TYPE::SPLAT;
	std::vector<SplatVertex> m_vertices;
	std::shared_ptr<BaseSorter<SplatVertex>> m_sorter = nullptr;
};


template<typename T>
inline QuickSortCPU<T>::QuickSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder)
{
	this->m_vertexCount = vertexCount;
	this->m_sortOrder = sortOrder;
	m_index2depth.resize(vertexCount);
}

template<typename T>
inline void QuickSortCPU<T>::Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo)
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

	vbo->Update(depthIndex);
}

template<typename T>
inline RadixSortCPU<T>::RadixSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder)
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

template<typename T>
inline void RadixSortCPU<T>::Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo)
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
		for (const auto& elem : source)
		{
			const auto& [_, depth] = elem;
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
		for (const auto& elem : source)
		{
			const auto& [_, depth] = elem;
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
		if (this->m_sortOrder == DESCENDING)
			depthIndex[i] = idx;
		else
			depthIndex[this->m_vertexCount - 1 - i] = idx;
	}

	vbo->Update(depthIndex);
}

template<typename T>
inline CountingSortCPU<T>::CountingSortCPU(uint32_t vertexCount, SORT_ORDER sortOrder)
{
	this->m_vertexCount = vertexCount;
	this->m_sortOrder = sortOrder;
	m_sizeList.resize(vertexCount);
	std::memset(m_sizeList.data(), 0, sizeof(int32_t) * vertexCount);
	m_depth.resize(vertexCount);
	std::memset(m_depth.data(), 0, sizeof(float) * vertexCount);
}

template<typename T>
inline void CountingSortCPU<T>::Sort(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, std::vector<uint32_t>& depthIndex, std::shared_ptr<VertexBufferObject> vbo)
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

	vbo->Update(depthIndex);
}

RENDERABLE_END

