#pragma once
#ifndef __MESH_PARSER_H__
#define __MESH_PARSER_H__
#include <iostream>
#include <fstream>
#include <string>
#include "cocos2d.h"
#include "Node3D.h"

using namespace cocos2d;

namespace cocos3d
{
	class MeshParser
	{
	public:
		const std::vector<Vec3>& positions() { return m_vertices; }
		const std::vector<Vec2>& texels() { return m_realTexels; }
		const std::vector<Vec3>& normals() { return m_realNormals; }
		const std::vector<Vec3>& diffuses() { return m_diffuses; }
		const std::vector<Vec3>& speculars() { return m_speculars; }
		const std::vector<std::string>& materials() { return m_materials; }
		const std::vector<int>& firsts() { return m_firsts; }
		const std::vector<int>& counts() { return m_counts; }
		const std::vector<int>& faces() { return m_faces; }
		const Vec3& getCenter() { return m_center; }
		const float getRadius(){ return m_radius; }
		const Vec3& getSize(){ return m_size; }
		const kmAABB getAABB(){ return m_aabb; }
	protected:
		std::vector<Vec2> m_realTexels;
		std::vector<Vec3> m_realNormals;

		std::vector<Vec3> m_positions;
		std::vector<Vec3> m_normals;
		std::vector<Vec3> m_vertices;
		std::vector<Vec2> m_texels;
		std::vector<GLushort> m_indices;
		std::vector<Vec3> m_diffuses;
		std::vector<Vec3> m_speculars;
		std::vector<std::string> m_materials;
		std::vector<int> m_faces;
		std::vector<int> m_firsts;
		std::vector<int> m_counts;
		Vec3 m_center;
		Vec3 m_size;
		float m_radius;
		kmAABB m_aabb;
	};
}
#endif
