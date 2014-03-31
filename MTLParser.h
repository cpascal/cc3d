#pragma once
#ifndef __MTL_PARSER_H__
#define __MTL_PARSER_H__
#include <iostream>
#include <fstream>
#include <string>
#include "cocos2d.h"

using namespace std;
using namespace cocos2d;

namespace cocos3d
{
	class Model;

	class MTLParser
	{
	public:
		MTLParser() : m_scale(1.0f), m_copied(false){}
		~MTLParser();

		bool readFile(const string &objFile, const string & mtlFile, float scale = 1.0f);
		bool readBuffer(const string &objData, const string &mtlData, float scale = 1.0f);

		const vector<ccVertex3F>& positions() { return m_vertices; }
		const vector<ccVertex2F>& texels() { return m_realTexels; }
		const vector<ccVertex3F>& normals() { return m_realNormals; }
		const vector<ccVertex3F>& diffuses() { return m_diffuses; }
		const vector<ccVertex3F>& speculars() { return m_speculars; }
		const vector<string>& materials() { return m_materials; }
		const vector<int>& firsts() { return m_firsts; }
		const vector<int>& counts() { return m_counts; }
		const vector<int>& faces() { return m_faces; }
		const ccVertex3F& getCenter() { return m_center; }
		const float getRadius(){ return m_radius; }
		const ccVertex3F& getSize();

		void flatNormals();

		void clearPositions(){ m_vertices.clear(); }
		void clearNormals(){ m_realNormals.clear(); }
	private:
		void getBounds();
		void normalize(float scaleTo = 1.0f, bool center = true);
		void generateNormals();

		bool read(istream & objStream, istream & mtlStream);

		vector<ccVertex3F> m_positions;
		vector<ccVertex3F> m_vertices;

		vector<ccVertex2F> m_texels;
		vector<ccVertex2F> m_realTexels;

		vector<ccVertex3F> m_normals;
		vector<ccVertex3F> m_realNormals;

		vector<ccVertex3F> m_diffuses;
		vector<ccVertex3F> m_speculars;
		vector<string> m_materials;
		vector<int> m_faces;

		vector<int> m_firsts;
		vector<int> m_counts;

		bool extractOBJData(istream &fp);
		bool extractMTLData(istream &fp);
		void reorgPositions();

		ccVertex3F m_center;
		ccVertex3F m_size;
		float m_radius;
		float m_scale;
		int m_partsPerFace;
		bool m_copied;

		friend class Model;
	};
}
#endif
