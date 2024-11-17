#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <any>

#include "../draw/shader_s.h"
#include "../draw/texture.h"
#include "../parser/config_parser.h"

#define RENDERABLE_BEGIN namespace Renderable {
#define RENDERABLE_END }

RENDERABLE_BEGIN
struct VertexInfo {
	VertexInfo(const char* name, uint32_t location, uint32_t count, uint32_t type, bool normalized, uint32_t offset, uint32_t stride) :
		attriName(std::string(name)), attriLocation(location), count(count), type(type), normalized(normalized), offset(offset), stride(stride) {}

	std::string attriName = "";
	uint32_t attriLocation = 0;
	uint32_t count = 0;
	uint32_t type = 0;
	bool normalized = false;
	uint32_t offset = 0;
	uint32_t stride = 0;

};

class RenderObjectBase {
public:
	virtual ~RenderObjectBase() = default;
	virtual void DrawObj(const std::unordered_map<std::string, std::any>& uniform) = 0;
	virtual void ImGuiCallback() {};

protected:
	virtual void Draw() = 0;
};

template<typename vT, typename iT = uint32_t>
class RenderObjectNaive : public RenderObjectBase {
private:
	GLsizei m_primitive = GL_TRIANGLES;
	size_t m_vertexCount = 0;
	uint32_t m_vertexAttrNum = 0;
	size_t m_indiceCount = 0;
	float m_lineWidth = 0.0f;
	float m_pointSize = 0.0f;
	std::vector<vT> m_vertices;
	std::vector<iT> m_indices;

protected:
	uint32_t m_VAO = 0;
	uint32_t m_VBO = 0;
	uint32_t m_EBO = 0;
	std::unique_ptr<Shader> m_shader = nullptr;
	std::unique_ptr<Texture> m_textures = nullptr;
	std::vector<size_t> m_textureIdx{};
	virtual void SetUpShader(const std::string& vertex_shader, const std::string& fragment_shader);
	virtual void SetUpShader() {};
	virtual void SetUpData() {};
	virtual void SetUpTexture(int num = 0) {};
	virtual void DrawObj(const std::unordered_map<std::string, std::any>& uniform) {};
	RenderObjectNaive() {};
	RenderObjectNaive(Parser::RenderObjConfigNaive& config) {}
	~RenderObjectNaive();

	void SetLineWidth(float val) { m_lineWidth = val; }
	void SetPointSize(float val) { m_pointSize = val; }
	void SetVertexCount(int val) { m_vertexCount = val; }
	void SetIndiceCount(int val) { m_indiceCount = val; }
	void SetPrimitive(GLsizei val) { m_primitive = val; }
	const GLsizei GetPrimitive() const { return m_primitive; }
	uint32_t GetVAO() { return m_VAO; }
	uint32_t GetVBO() { return m_VBO; }
	uint32_t GetEBO() { return m_EBO; }
	void SetMesh(std::vector<vT>* vertices = nullptr, std::vector<VertexInfo>* infos = nullptr, std::vector<iT>* indices = nullptr);
	void Draw();

};

template<typename vT, typename iT>
inline void RenderObjectNaive<vT, iT>::SetUpShader(const std::string& vertex_shader, const std::string& fragment_shader)
{
	m_shader = std::make_unique<Shader>(vertex_shader.c_str(), fragment_shader.c_str());
}

template<typename vT, typename iT>
inline RenderObjectNaive<vT, iT>::~RenderObjectNaive()
{
	if (m_VAO > 0)
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}

	if (m_VBO > 0)
	{
		glDeleteBuffers(1, &m_VBO);
		m_VBO = 0;
	}

	if (m_EBO > 0)
	{
		glDeleteBuffers(1, &m_EBO);
		m_EBO = 0;
	}
}

template<typename vT, typename iT>
inline void RenderObjectNaive<vT, iT>::SetMesh(std::vector<vT>* vertices, std::vector<VertexInfo>* infos, std::vector<iT>* indices)
{
	if ((*vertices).empty()) return;

	m_vertices = *vertices;
	m_vertexAttrNum = (*infos)[0].offset;
	m_vertexCount = m_vertices.size() / m_vertexAttrNum;

	if (indices) {
		m_indices = *indices;
		m_indiceCount = m_indices.size();
	}

	if (m_VAO == 0)
	{
		glGenVertexArrays(1, &m_VAO);
	}

	if (m_VBO == 0)
	{
		glGenBuffers(1, &m_VBO);
	}

	if (m_EBO == 0 && !m_indices.empty())
	{
		glGenBuffers(1, &m_EBO);
	}

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(vT), static_cast<void*>(m_vertices.data()), GL_STATIC_DRAW);

	if (m_EBO != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(iT), static_cast<void*>(m_indices.data()), GL_STATIC_DRAW);
	}


	for (auto info : *infos)
	{
		glVertexAttribPointer(info.attriLocation, info.count, info.type, info.normalized, info.offset * sizeof(vT), (void*)(info.stride * sizeof(info.type)));
		glEnableVertexAttribArray(info.attriLocation);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

template<typename vT, typename iT>
inline void RenderObjectNaive<vT, iT>::Draw()
{
	if (m_EBO > 0 && m_indices.empty())
	{
		return;
	}

	if (m_vertices.empty())
	{
		return;
	}

	if (m_VAO > 0)
	{
		glBindVertexArray(m_VAO);
	}

	if (m_lineWidth > 0.0f)
	{
		glLineWidth(m_lineWidth);
	}

	if (m_pointSize > 0.0f)
	{
		glPointSize(m_pointSize);
	}

	if (!m_indices.empty())
	{
		glDrawElements(m_primitive, static_cast<GLsizei>(m_indiceCount), GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(m_primitive, 0, static_cast<GLsizei>(m_vertexCount));
		glBindVertexArray(0);
	}

}
RENDERABLE_END

