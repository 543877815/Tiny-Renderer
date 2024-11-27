#pragma once
#include "common.h"

RENDERABLE_BEGIN

struct Point {
	float x = 0.0, y = 0.0, z = 0.0;
	float r = 0.0, g = 0.0, b = 0.0;
	float u = 0.0, v = 0.0;

	Point operator+(const Point& other) const {
		return Point{ x + other.x, y + other.y, z + other.z,
		r + other.r, g + other.g, b + other.b,
		u + other.u, v + other.v };
	}

	Point operator*(float scalar) const {
		return Point{ x * scalar, y * scalar, z * scalar,
		r * scalar, g * scalar, b * scalar,
		u * scalar, v * scalar };
	}

	friend Point operator*(float scalar, const Point& point) {
		return point * scalar;
	}

};


class MapObj : public RenderObjectNaive<Point, uint32_t> {

public:
	MapObj(std::shared_ptr<Parser::RenderObjConfigBase> baseConfigPtr);
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	void ImGuiCallback();

private:
	void SetUpTexture(int num = 0) override;
	void SetUpGLStatus();
	void SetUpData() override;
	void UV2XYZ(const Point& point, Point& xyz);

private:
	float m_gridLeft = -1.0;
	float m_gridRight = 1.0;
	float m_gridBottom = -1.0;
	float m_gridTop = 1.0;
	int m_gridWidth = 50;
	int m_gridHeight = 50;
	float m_transform_scale = 0.0;
	std::vector<VertexInfo> m_vertexInfo = std::vector<VertexInfo>{
		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 1, 0},
		{"aColor", 1, 3, GL_FLOAT, GL_FALSE, 1, 3},
		{"aTexCoord", 2, 2, GL_FLOAT, GL_FALSE, 1, 6}
	};
	std::unordered_map<GLsizei, std::string> m_primitiveName{
		{GL_POINTS, "points"},
		{GL_TRIANGLES, "triangles"}
	};

};


RENDERABLE_END