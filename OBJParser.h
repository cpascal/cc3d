#pragma once
#ifndef __OBJ_PARSER_H__
#define __OBJ_PARSER_H__
#include <iostream>
#include <fstream>
#include <string>
#include "cocos2d.h"
#include "MeshParser.h"

using namespace std;
using namespace cocos2d;

namespace cocos3d
{
	class Model;

	class OBJParser : public MeshParser
	{
	public:
		bool readFile(const string &objFile, const string & mtlFile, float scale = 1.0f);
		bool readBuffer(const string &objData, const string &mtlData, float scale = 1.0f);
		void flatNormals();
	private:
		void getBounds();
		void normalize(float scaleTo = 1.0f, bool center = true);
		void generateNormals();

		bool read(istream & objStream, istream & mtlStream);


		bool extractOBJData(istream &fp);
		bool extractMTLData(istream &fp);
		void reorgPositions();

		int m_partsPerFace;
	};
}
#endif
