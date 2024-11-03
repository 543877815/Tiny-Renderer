#pragma once
#include <iostream>
#include <algorithm> 
#include <format>

class AABB {
public:
	// Define the type for a point
	struct Point {
		float x, y, z;

		Point() : x(0.0), y(0.0), z(0.0) {}
		Point(float x, float y, float z) : x(x), y(y), z(z) {}

		// Overload the less than operator to facilitate the use of std::min and std::max
		bool operator<(const Point& other) const {
			return x < other.x || (x == other.x && (y < other.y || (y == other.y && z < other.z)));
		}
	};

private:
	Point m_minCorner{ FLT_MAX, FLT_MAX, FLT_MAX };  // The minimum corner of the AABB
	Point m_maxCorner{ FLT_MIN, FLT_MIN, FLT_MIN };  // The maximum corner of the AABB

public:
	// Constructor
	AABB() {};
	AABB(const Point& min, const Point& max) : m_minCorner(min), m_maxCorner(max) {}

	// Get the minimum corner
	Point getMinCorner() const {
		return m_minCorner;
	}

	// Get the maximum corner
	Point getMaxCorner() const {
		return m_maxCorner;
	}

	void reset() {
		m_minCorner = Point{ FLT_MAX, FLT_MAX, FLT_MAX };
		m_maxCorner = Point{ FLT_MIN, FLT_MIN, FLT_MIN };
	}

	// Expand the AABB to include a given point
	void expand(const Point& point) {
		m_minCorner.x = (std::min)(m_minCorner.x, point.x);
		m_minCorner.y = (std::min)(m_minCorner.y, point.y);
		m_minCorner.z = (std::min)(m_minCorner.z, point.z);
		m_maxCorner.x = (std::max)(m_maxCorner.x, point.x);
		m_maxCorner.y = (std::max)(m_maxCorner.y, point.y);
		m_maxCorner.z = (std::max)(m_maxCorner.z, point.z);
	}

	// Check if a point is inside the AABB
	bool contains(const Point& point) const {
		return point.x >= m_minCorner.x && point.x <= m_maxCorner.x &&
			point.y >= m_minCorner.y && point.y <= m_maxCorner.y &&
			point.z >= m_minCorner.z && point.z <= m_maxCorner.z;
	}

	// Check if two AABBs overlap
	bool overlaps(const AABB& other) const {
		return m_minCorner.x <= other.m_maxCorner.x && m_maxCorner.x >= other.m_minCorner.x &&
			m_minCorner.y <= other.m_maxCorner.y && m_maxCorner.y >= other.m_minCorner.y &&
			m_minCorner.z <= other.m_maxCorner.z && m_maxCorner.z >= other.m_minCorner.z;
	}

	// Print the AABB information
	void print() const {
		std::cout << std::format("AABB: Min Corner ({}, {}, {})", m_minCorner.x, m_minCorner.y, m_minCorner.z)
			<< std::format("Max Corner({}, {}, {})", m_maxCorner.x, m_maxCorner.y, m_maxCorner.z) << std::endl;
	}
};

