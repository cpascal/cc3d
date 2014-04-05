#include "MTLParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits>

using namespace cocos3d;

MTLParser::~MTLParser()
{
	if (m_copied)
		return;
}

string tokenize(string before, string & after, const char * token = " ")
{
    int tokLength = before.find(token);
    
	if (tokLength <= 0) 
	{
        after = "";
        return before;
    }

    after = before.substr(tokLength + 1);
    return before.substr(0, tokLength);
}

ccVertex3F parse3F(string content)
{
    ccVertex3F vertex;
	vertex.x = atof(tokenize(content, content).c_str());
    vertex.y = atof(tokenize(content, content).c_str());
    vertex.z = atof(tokenize(content, content).c_str());
    return vertex;
}

ccVertex2F parse2F(string content)
{
    ccVertex2F vertex;
    vertex.x = atof(tokenize(content, content).c_str());
    vertex.y = atof(tokenize(content, content).c_str());
    return vertex;
}

bool MTLParser::extractOBJData(istream & objStream)
{
    int mtl = -1;
    m_partsPerFace = 3;
    if (!objStream.good())
		return false;

    while (!objStream.eof())
	{
        string line;
        getline(objStream, line);
        string type = line.substr(0, 2);

        std::string content;
        tokenize(line, content);
        if (type.compare("us") == 0)
		{
            // usemtl content
            for (unsigned int i=0; i < m_materials.size(); ++i)
			{
                if (m_materials[i].compare(content) == 0)
                    mtl = i;
            }
        }
		else
		if (type.compare("v ") == 0)
			m_positions.push_back(parse3F(content));
		else
		if (type.compare("vt") == 0)
			m_texels.push_back(parse2F(content));
		else
		if (type.compare("vn") == 0)
            m_normals.push_back(parse3F(content));
		else
		if (type.compare("f ") == 0)
		{
            while(!content.empty())
			{
				int tmpPush = -1;

                string part = tokenize(content, content, " ");
                for(int i=0; i < m_partsPerFace; ++i)
				{
                    std::string tok = tokenize(part, part, "/");

					if (tok.length() >0 && tok[0] == '/')
					{
						m_faces.push_back(0);
						tmpPush = atoi(tok.substr(1).c_str());
					}
					else
					if (tok.length() == 0)
						m_faces.push_back(tmpPush);
					else
						m_faces.push_back(atoi(tok.c_str()));
                }
            }
            m_faces.push_back(mtl);
        }
    }
    return true;
}


void MTLParser::reorgPositions()
{
    // for(int p = 0; p < m_positions.size(); ++p)
    //     cout << "initial positions " << m_positions[p].x << " " << m_positions[p].y << " " <<m_positions[p].z << endl;
    // for (int j=0; j < m_faces.size() / 10; ++j)
    //     cout << m_faces[10*j + 0] << "/"<< m_faces[10*j + 1] << "/"<< m_faces[10*j + 2] << " "<< m_faces[10*j + 3] << "/"<< m_faces[10*j + 4] << "/"<< m_faces[10*j + 5] << " "<< m_faces[10*j + 6] << "/"<< m_faces[10*j + 7] << "/"<< m_faces[10*j + 8] << endl;
    m_counts.clear();
    m_counts.resize(m_materials.size(), 0);
    m_firsts.clear();
    m_firsts.resize(m_materials.size(), 0);

    int faceSetSize = m_partsPerFace * 3 + 1;
    int faceDataSize = m_partsPerFace * 3;

    for(unsigned int k=0; k < m_materials.size(); ++k)
	{
        for(unsigned int i=0; i<m_faces.size() / faceSetSize; i++)
		{
            if (m_faces[faceSetSize*i + faceDataSize] == k)
			{
                int vA = m_faces[faceSetSize*i] - 1;
                int vB = m_faces[faceSetSize*i + 3] - 1;
                int vC = m_faces[faceSetSize*i + 6] - 1;
                // cout << "pos " << i << " " << vA << " " << vB << " " << vC << endl;

                if (vA >= 0)
                    m_vertices.push_back(m_positions[vA]);
                if (vB >= 0)
                    m_vertices.push_back(m_positions[vB]);
                if (vC >= 0)
                    m_vertices.push_back(m_positions[vC]);

                if (m_partsPerFace > 1)
				{
                    int tA = m_faces[faceSetSize*i + 1] - 1;
                    int tB = m_faces[faceSetSize*i + 4] - 1;
                    int tC = m_faces[faceSetSize*i + 7] - 1;

                    if (tA >= 0)
                        m_realTexels.push_back(m_texels[tA]);
                    if (tB >= 0)
                        m_realTexels.push_back(m_texels[tB]);
                    if (tC >= 0)
                        m_realTexels.push_back(m_texels[tC]);

                    if (m_partsPerFace > 2)
					{

                        int nA = m_faces[faceSetSize*i + 2] - 1;
                        int nB = m_faces[faceSetSize*i + 5] - 1;
                        int nC = m_faces[faceSetSize*i + 8] - 1;

                        if (nA >= 0)
                            m_realNormals.push_back(m_normals[nA]);
                        if (nB >= 0)
                            m_realNormals.push_back(m_normals[nB]);
                        if (nC >= 0)
                            m_realNormals.push_back(m_normals[nC]);
                    }

                }

                m_counts[k] += 3;
            }
        }
    }
    for (int i=0; i < m_counts.size() - 1; ++i)
	{
        m_firsts[i + 1] = m_firsts[i] + m_counts[i];
    }

	normalize(m_scale);
}

bool MTLParser::extractMTLData(istream & mtlStream)
{
    if (!mtlStream.good()) 
	{
        return false;
    }

    while (!mtlStream.eof()) 
	{
        string line;
        getline(mtlStream, line);
        string type = line.substr(0, 2);

        std::string content;
        tokenize(line, content);
        if (type.compare("ne") == 0)
            m_materials.push_back(content);
		else 
		if (type.compare("Kd") == 0)
            m_diffuses.push_back(parse3F(content));
        else 
		if (type.compare("Ks") == 0)
            m_speculars.push_back(parse3F(content));
    }
    return true;
}

bool MTLParser::readBuffer(const string &objData, const string &mtlData, float scale)
{
	m_scale = scale;

    stringstream mtlStream(mtlData.c_str());
    stringstream objStream(objData.c_str());
    return read(objStream, mtlStream);
}
bool MTLParser::readFile(const string &objFile, const string &mtlFile, float scale)
{
	m_scale = scale;

    ifstream mtlStream(mtlFile.c_str());
    ifstream objStream(objFile.c_str());
    return read(objStream, mtlStream);
}

bool MTLParser::read(istream &objStream, istream &mtlStream)
{
    if (extractMTLData(mtlStream)) 
	{
        extractOBJData(objStream);
        reorgPositions();
		flatNormals();
        return true;
    }

    return false;
}

void MTLParser::getBounds()
{
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
#undef max()
#undef min()
#endif

	float width, height, length;
    float xMax = numeric_limits<float>::min();
    float yMax = numeric_limits<float>::min();
    float zMax = numeric_limits<float>::min();

    float xMin = numeric_limits<float>::max();
    float yMin = numeric_limits<float>::max();
    float zMin = numeric_limits<float>::max();

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    int numVerts = static_cast<int>(m_vertices.size());

    for (int i = 0; i < numVerts; ++i)
    {
        x = m_vertices[i].x;
        y = m_vertices[i].y;
        z = m_vertices[i].z;

        if (x < xMin)
            xMin = x;

        if (x > xMax)
            xMax = x;

        if (y < yMin)
            yMin = y;

        if (y > yMax)
            yMax = y;

        if (z < zMin)
            zMin = z;

        if (z > zMax)
            zMax = z;
    }

    m_center.x = (xMin + xMax) / 2.0f;
    m_center.y = (yMin + yMax) / 2.0f;
    m_center.z = (zMin + zMax) / 2.0f;

    width = xMax - xMin;
    height = yMax - yMin;
    length = zMax - zMin;

	kmVec3Fill(&m_aabb.min, xMin, yMin, zMin);
	kmVec3Fill(&m_aabb.max, xMax, yMax, zMax);

	m_size.x = width;
	m_size.y = height;
	m_size.z = length;

    m_radius = max(max(width, height), length);
}

void MTLParser::normalize(float scaleTo, bool center)
{
    float width = 0.0f;
    float height = 0.0f;
    float length = 0.0f;
    float centerPos[3] = {0.0f};

    getBounds();

    float offset[3] = {0.0f};

    if (center)
    {
        offset[0] = -centerPos[0];
        offset[1] = -centerPos[1];
        offset[2] = -centerPos[2];
    }
    else
    {
        offset[0] = 0.0f;
        offset[1] = 0.0f;
        offset[2] = 0.0f;
    }

    getBounds();
}

void MTLParser::flatNormals()
{
	//return;

	m_realNormals.clear();

	for (int i = 0; i < m_vertices.size(); i = i + 3)
	{
		kmVec3* a = (kmVec3*)&(m_vertices[i]);
		kmVec3* b = (kmVec3*)&(m_vertices[i + 1]);
		kmVec3* c = (kmVec3*)&(m_vertices[i + 2]);

		kmVec3 normal;
		kmVec3 diff1, diff2;

		kmVec3Subtract(&diff1,c,a);
		kmVec3Subtract(&diff2,b,a);

		kmVec3Cross(&normal,&diff1,&diff2);

		ccVertex3F* realNormal = (ccVertex3F*)&normal;

		m_realNormals.push_back(*realNormal);
		m_realNormals.push_back(*realNormal);
		m_realNormals.push_back(*realNormal);
	}
}