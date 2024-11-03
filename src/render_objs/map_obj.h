#pragma once
#include "common.h"

RENDERABLE_BEGIN

static struct Point {
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


class MapObj : public RenderObject<Point, uint32_t> {

public:
	MapObj();
	void DrawObj(const std::unordered_map<std::string, std::any>& uniform);
	void ImGuiCallback();

private:
	void SetUpGLStatus() {
		glEnable(GL_DEPTH_TEST);
	}

	void SetUpTexture(int num = 0) override;
	void SetUpShader() override;
	void SetUpData() override;
	void UV2XYZ(const Point& point, Point& xyz);

private:
	float m_grid_left = -1.0;
	float m_grid_right = 1.0;
	float m_grid_bottom = -1.0;
	float m_grid_top = 1.0;
	int m_grid_width = 50;
	int m_grid_height = 50;
	float m_transform_scale = 0.0;

	std::unordered_map<GLsizei, std::string> m_primitive_name{
		{GL_POINTS, "points"},
		{GL_TRIANGLES, "triangles"}
	};

};


RENDERABLE_END