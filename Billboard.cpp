#include "Billboard.h"
#include "Light.h"
#include "Layer3D.h"
#include "Camera.h"
#include "shaders.h"
#include "Scene3D.h"
#include "VBOCache.h"
#include <limits>
#include <map>

#include "poly2tri/poly2tri.h"
#include "tristripper/tri_stripper.h"

using namespace cocos3d;

#define LINKS_VERTEX_ATTRIB 6

static const std::string randomString(int length) 
{
	static std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	std::string result;
	result.resize(length);

	for (int i = 0; i < length; i++)
		result[i] = charset[rand() % charset.length()];

	return result;
}

Billboard::Billboard()
: Model()
, m_hulled(false)
, m_animatedHull(false)
, m_at(0)
, m_delay(0)
, m_linksVBO(0)
{
	m_color.x = m_color.y = m_color.z = 1;
}

Billboard* Billboard::createWithSize(const CCSize& size, float thickness)
{
	Billboard *pRet = new Billboard();
	if (pRet && pRet->initWithSize(size, thickness))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

Billboard* Billboard::createWithTexture(CCTexture2D* texture, float thickness)
{
	Billboard *pRet = new Billboard();
	if (pRet && pRet->initWithTexture(texture, thickness))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

Billboard* Billboard::createWithTextureGrid(CCTexture2D* texture, const int framesPerRow, const CCSize& frameSize, float thickness)
{
	Billboard *pRet = new Billboard();
	if (pRet && pRet->initWithTextureGrid(texture,framesPerRow,frameSize,thickness))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

bool Billboard::initWithSize(const CCSize& size, float thickness)
{
	srand((unsigned)time(NULL));

	m_id = "billboard_" + randomString(10);
	m_textured = false;
	m_culling = false;

	m_dTexture = NULL;

	m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_KEY);

	if (m_program == NULL)
	{
		m_program = new CCGLProgram();
		MESH_INIT_PHONG(m_program);
		CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_KEY);
	}

	setShaderProgram(m_program);

	if (thickness == 0)
		createQuad(size.width, size.height);
	else
		createCube(size.width, size.height, thickness);

	generateVBOs();
	initShaderLocations();

#if CC_ENABLE_CACHE_TEXTURE_DATA
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(Billboard::listenBackToForeground),
		EVENT_COME_TO_FOREGROUND,
		NULL);
#endif

	return Node3D::init();
}

bool Billboard::initWithTexture(CCTexture2D* texture, float thickness)
{
	bool pRet = (texture != NULL);
	
	if (pRet)
	{
		m_id = "billboard_" + randomString(10);
		m_textured = true;
		m_culling = false;

		m_dTexture = texture;
		m_dTexture->retain();

		m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_TEXTURE_KEY);
			
		if (m_program == NULL)
		{
			m_program = new CCGLProgram();
			MESH_INIT_PHONG_TEXTURE(m_program);
			CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_TEXTURE_KEY);
		}

		setShaderProgram(m_program);

		createQuad(texture->getContentSize().width, texture->getContentSize().height);
		generateVBOs();
		initShaderLocations();
	}

#if CC_ENABLE_CACHE_TEXTURE_DATA
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(Billboard::listenBackToForeground),
		EVENT_COME_TO_FOREGROUND,
		NULL);
#endif

	return Node3D::init() && pRet;
}

bool Billboard::initWithTextureGrid(CCTexture2D* texture, const int framesPerRow, const CCSize& frameSize, float thickness)
{
	bool pRet = (texture != NULL);

	if (pRet)
	{
		m_id = "billboard_" + randomString(10);
		m_textured = true;
		m_culling = false;

		m_dTexture = texture;
		m_dTexture->retain();

		float rows = m_dTexture->getContentSize().height / frameSize.height;
		float cols = m_dTexture->getContentSize().width / frameSize.width;

		if (rows == (int)rows && cols == (int)cols)
		{
			m_nframes = rows*cols;
			m_cols = (unsigned int)cols;
			m_rows = (unsigned int)rows;
			m_frameSize = frameSize;
		}

		m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_TEXTURE_KEY);

		if (m_program == NULL)
		{
			m_program = new CCGLProgram();
			MESH_INIT_PHONG_TEXTURE(m_program);
			CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_TEXTURE_KEY);
		}

		setShaderProgram(m_program);

		if (thickness == 0)
		{
			if (m_nframes == 0)
				createQuad(texture->getContentSize().width, texture->getContentSize().height);
			else
				createAnimatedQuad();
		}
		else
		{
			if (m_nframes == 0)
				createCube(texture->getContentSize().width, texture->getContentSize().height, thickness);
			else
				createAnimatedCube(thickness);
		}

		generateVBOs();
		initShaderLocations();
	}

#if CC_ENABLE_CACHE_TEXTURE_DATA
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(Billboard::listenBackToForeground),
		EVENT_COME_TO_FOREGROUND,
		NULL);
#endif

	return Node3D::init() && pRet;
}

void Billboard::createQuad(int width, int height)
{
	Vec3 v1(-width/2.0f, -height/2.0f, 0);
	Vec3 v2(-width/2.0f, height/2.0f, 0	);
	Vec3 v3(width/2.0f, -height/2.0f, 0 );
	Vec3 v4(width/2.0f, height/2.0f, 0	);
	
	Vec2 t1(0, 1);
	Vec2 t2(0, 0);
	Vec2 t3(1, 1);
	Vec2 t4(1, 0);

	Vec3 u(v2.x - v1.x, v2.y - v1.y, 0);
	Vec3 v(v3.x - v1.x, v3.y - v1.y, 0);

	Vec3 n1(u.y * v.z - u.z*v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
	Vec3 n2, n3, n4;

	n2 = n3 = n4 = n1;

	m_aabb.max.x = width / 2.0f;
	m_aabb.max.y = height / 2.0f;
	m_aabb.max.z = m_aabb.min.z = 0;
	m_aabb.min.x = -m_aabb.max.x;
	m_aabb.min.y = -m_aabb.max.y;

	m_vertices.push_back(v1);
	m_vertices.push_back(v2);
	m_vertices.push_back(v3);
	m_vertices.push_back(v4);

	m_texels.push_back(t1);
	m_texels.push_back(t2);
	m_texels.push_back(t3);
	m_texels.push_back(t4);

	m_normals.push_back(n1);
	m_normals.push_back(n2);
	m_normals.push_back(n3);
	m_normals.push_back(n4);
}

void Billboard::updateFrame(float dt)
{
	m_currentFrame++;
	
	if (m_currentFrame == m_nframes)
		m_currentFrame = 0;
}

void Billboard::setDelay(float delay)
{
	m_delay = delay;
	
	unschedule(schedule_selector(Billboard::updateFrame));
	
	if (m_nframes > 0)
	{
		this->schedule(schedule_selector(Billboard::updateFrame),m_delay);
	}
}

void Billboard::createAnimatedQuad()
{
	int width = m_frameSize.width;
	int height = m_frameSize.height;

	createQuad(width*m_fScaleX, height*m_fScaleY);

	float texelWidth = m_frameSize.width / m_dTexture->getContentSizeInPixels().width;
	float texelHeight = m_frameSize.height / m_dTexture->getContentSizeInPixels().height;

	for (int j = 0; j < m_rows; j++)
	{
		for (int i = 0; i < m_cols; i++)
		{
			std::vector<Vec2> texelBuffer;

			Vec2 _t1(i*texelWidth,		(j+1)*texelHeight	);	// 0,1
			Vec2 _t2(i*texelWidth,		j*texelHeight		);	// 0,0
			Vec2 _t3((i+1)*texelWidth,	(j+1)*texelHeight	);	// 1,1
			Vec2 _t4((i+1)*texelWidth,	j*texelHeight		);	// 1,0

			texelBuffer.push_back(_t1);
			texelBuffer.push_back(_t2);
			texelBuffer.push_back(_t3);
			texelBuffer.push_back(_t4);

			m_texelsFrame.push_back(texelBuffer);
		}
	}

	CC_ASSERT(m_texelsFrame.size() == m_nframes);

	m_texels = m_texelsFrame[0];
}

void Billboard::createCube(int width, int height, float thickness)
{
}

void Billboard::createAnimatedCube(float thickness)
{
}

void Billboard::listenBackToForeground(CCObject *obj)
{
	m_program = new CCGLProgram();

	if (m_textured && !m_textureToAlpha)
	{
		MESH_INIT_PHONG_TEXTURE(m_program);
		CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_TEXTURE_KEY);
	}
	else
	if (m_textureToAlpha)
	{
		MESH_INIT_PHONG_TEXTURE_TO_ALPHA(m_program);
		CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_TEXTURE_TO_ALPHA_KEY);
	}
	else
	{
		MESH_INIT_PHONG(m_program);
		CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_KEY);
	}

	if (VBOCache::sharedVBOCache()->cacheIsInvalid())
		VBOCache::sharedVBOCache()->purgeCache();

	setShaderProgram(m_program);
	generateVBOs();

	m_program->use();

	initShaderLocations();

	m_lightsToSet = true;
}

void Billboard::draw3D()
{
	m_dirty = true;

	m_at += CCDirector::sharedDirector()->getDeltaTime();

	setupMatrices();
	setupShadow();

	bool toRender = true;

	if (m_culling)
		toRender = ((Layer3D*)m_pParent)->get3DCamera()->isObjectVisible(this, Frustum::ALL_PLANES);

	if (!toRender)
		return;
	
	setupLights();

	if (m_animatedHull)
		setupAnimation();
	
	setupTextures();

	Vec3 specular(1, 1, 1);

    setupMaterial(m_color, specular);
    setupAttribs();

    if (m_lines)
		glDrawArrays(GL_LINES, 0,m_vertices.size());
    else
	if (m_hulled)
	{
		if (m_indices.size() > 0)
			glDrawElements(GL_TRIANGLE_STRIP, m_indices.size(), GL_UNSIGNED_INT, &(m_indices[0]));
		else
			glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
	}
	else
		glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertices.size());

    CC_INCREMENT_GL_DRAWS(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();	

	if (m_drawOBB)
		renderOOBB();

	glBindTexture(GL_TEXTURE_2D, NULL);
}

void Billboard::setupAnimation()
{
	float dt = CCDirector::sharedDirector()->getDeltaTime();
	
	static float time = 0;
	
	time += dt;
	
	glUniform4f(m_shaderLocations["CC_Time"], time, time, time, time);
}

void Billboard::setId(const std::string& id)
{
	m_id = id;
}

void Billboard::setColor(const Vec3& color)
{
	m_color = color;
}

void Billboard::setupAttribs()
{
	if (m_textured && m_nframes > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_tVBO);

		if (m_nframes > 0)
		{
			m_texels = m_texelsFrame[m_currentFrame];
			glBufferData(GL_ARRAY_BUFFER, m_texels.size()*sizeof(Vec2), &(m_texels[0]), GL_DYNAMIC_DRAW);
		}

		glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	Model::setupAttribs();

	if (m_linksVBO == 0 && m_animatedHull)
	{
		glDisableVertexAttribArray(LINKS_VERTEX_ATTRIB);
	}

	if (m_animatedHull)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_linksVBO);
		glVertexAttribPointer(glGetAttribLocation(getShaderProgram()->getProgram(), "a_links"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

float Billboard::getRadius()
{ 
	return 0.0f;
}

static double randomDistribution(double x)
{
	return 2.5 + sin(10 * x) / x;
}

static double randomPoints(double(*function)(double), double xmin = 0, double xmax = 1)
{
	static double(*Fun)(double) = NULL, YMin, YMax;
	static bool First = true;

	// Initialises random generator for first call
	if (First)
	{
		First = false;
		srand((unsigned)time(NULL));
	}
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WP8)
	double randMax = 0x7fff;
#else
	double randMax = RAND_MAX;
#endif
	// Evaluates maximum of function
	if (function != Fun)
	{
		Fun = function;
		YMin = 0, YMax = Fun(xmin);
		for (int iX = 1; iX < randMax; iX++)
		{
			double X = xmin + (xmax - xmin) * iX / randMax;
			double Y = Fun(X);
			YMax = Y > YMax ? Y : YMax;
		}
	}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	// Gets random values for X & Y
	double X = xmin + (xmax - xmin) * rand() / randMax;
	double Y = YMin + (YMax - YMin) * rand() / randMax;
#else
	// Gets random values for X & Y
	double X = xmin + (xmax - xmin) * fmod(rand(),randMax) / randMax;
	double Y = YMin + (YMax - YMin) * fmod(rand(),randMax) / randMax;
#endif
	// Returns if valid and try again if not valid
	return Y < function(X) ? X : randomPoints(Fun, xmin, xmax);
}

class compareVertex3F
{
public:
	bool operator()(Vec3 a, Vec3 b) const
	{
		return !(a.x == b.x && a.y == b.y && a.z == b.z);
	}
};

void Billboard::generateVertexIndex()
{
	std::vector<unsigned int> indicesSet;
	m_indices.resize(m_vertices.size());
	unsigned int index = 0;
	for (auto i = m_vertices.begin(); i != m_vertices.end(); i++, index++)
	{
		Vec3& v1 = *i;

		unsigned int index2 = index + 1;
		for (auto j = i + 1; j != m_vertices.end(); j++, index2++)
		{
			Vec3& v2 = *j;

			if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
			{
				auto alreadyIn = std::find(indicesSet.begin(), indicesSet.end(), index2);

				if (alreadyIn == indicesSet.end())
				{
					m_indices[index2] = index;
					indicesSet.push_back(index2);
				}
			}
			else
			{
				auto alreadyIn = std::find(indicesSet.begin(), indicesSet.end(), index2);

				if (alreadyIn == indicesSet.end())
					m_indices[index2] = index2;
			}
		}
	}
}

void Billboard::generateTriangleStrip()
{
	std::vector<unsigned int> newIndices;

	using namespace triangle_stripper;

	// convert 32 bits indices from the submesh into the type used by Tri Stripper
	indices Indices(m_indices.begin(), m_indices.end());

	primitive_vector PrimitivesVector;

	{ // we want to time the tri_stripper object destruction as well
		tri_stripper TriStripper(Indices);

		TriStripper.SetMinStripSize(2);
		TriStripper.SetCacheSize();
		TriStripper.SetBackwardSearch(false);

		TriStripper.Strip(&PrimitivesVector);

		for (int i = 0; i < PrimitivesVector.size(); i++)
		{
			if (PrimitivesVector[i].Type == TRIANGLE_STRIP)
			{
				for (auto iter = PrimitivesVector[i].Indices.begin();
					iter != PrimitivesVector[i].Indices.end();
					iter++)
				{
					newIndices.push_back(*iter);
				}
			}
		}
	}

	m_indices = newIndices;
}

void Billboard::triangulation(int factor)
{
	using namespace p2t;

	vector<Triangle*> triangles;
	list<Triangle*> map;
	vector< vector<Point*> > polylines;
	vector<p2t::Point*> polyline;

	polyline.push_back(new Point(m_aabb.min.x, m_aabb.min.y));
	polyline.push_back(new Point(m_aabb.min.x, m_aabb.max.y));
	polyline.push_back(new Point(m_aabb.max.x, m_aabb.max.y));
	polyline.push_back(new Point(m_aabb.max.x, m_aabb.min.y));

	polylines.push_back(polyline);

	CDT* cdt = new CDT(polyline);

	for (int i = 0; i < factor; i++)
	{
		double x = randomPoints(randomDistribution, m_aabb.min.x, m_aabb.max.x);
		double y = randomPoints(randomDistribution, m_aabb.min.y, m_aabb.max.y);
		cdt->AddPoint(new Point(x, y));
	}

	cdt->Triangulate();

	triangles = cdt->GetTriangles();

	m_vertices.clear();
	m_normals.clear();
	m_texels.clear();

	for (int i = 0; i < triangles.size(); i++)
	{
		Triangle& t = *triangles[i];
		Point& a = *t.GetPoint(0);
		Point& b = *t.GetPoint(1);
		Point& c = *t.GetPoint(2);

		Vec3 v1 = Vec3(static_cast<GLfloat>(a.x), static_cast<GLfloat>(a.y), 0.0f);
		Vec3 v2 = Vec3(static_cast<GLfloat>(b.x), static_cast<GLfloat>(b.y), 0.0f);
		Vec3 v3 = Vec3(static_cast<GLfloat>(c.x), static_cast<GLfloat>(c.y), 0.0f);

		Vec3 n1 = Vec3(static_cast<GLfloat>(a.x), static_cast<GLfloat>(a.y), 1.0f);
		Vec3 n2 = Vec3(static_cast<GLfloat>(b.x), static_cast<GLfloat>(b.y), 1.0f);
		Vec3 n3 = Vec3(static_cast<GLfloat>(c.x), static_cast<GLfloat>(c.y), 1.0f);

		Vec2 t1 = Vec2(static_cast<GLfloat>(a.x / m_aabb.max.x + 0.5f), static_cast<GLfloat>(a.y / m_aabb.min.y + 0.5f));
		Vec2 t2 = Vec2(static_cast<GLfloat>(b.x / m_aabb.max.x + 0.5f), static_cast<GLfloat>(b.y / m_aabb.min.y + 0.5f));
		Vec2 t3 = Vec2(static_cast<GLfloat>(c.x / m_aabb.max.x + 0.5f), static_cast<GLfloat>(c.y / m_aabb.min.y + 0.5f));

		m_texels.push_back(t1);
		m_texels.push_back(t2);
		m_texels.push_back(t3);

		m_vertices.push_back(v1);
		m_vertices.push_back(v2);
		m_vertices.push_back(v3);

		m_normals.push_back(n1);
		m_normals.push_back(n2);
		m_normals.push_back(n3);
	}

	delete cdt;

	for (int i = 0; i < polylines.size(); i++)
	{
		vector<Point*> poly = polylines[i];

		for (auto iter = polylines[i].begin(); iter != polylines[i].end(); iter++)
		{
			delete *iter;
		}
	}
}

void Billboard::hull(int factor, bool textured, bool animated, float increase, int axis)
{
	m_animatedHull = false;
	m_hulled = true;

	glDisableVertexAttribArray(LINKS_VERTEX_ATTRIB);

	triangulation(factor);
	//generateVertexIndex();
	//generateTriangleStrip();

	if (!textured && !animated)
	{
		m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_KEY);

		if (m_program == NULL)
		{
			m_program = new CCGLProgram();
			MESH_INIT_PHONG(m_program);
			CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_KEY);
		}

		setShaderProgram(m_program);
	}

	glDeleteBuffers(1, &m_pVBO);
	glDeleteBuffers(1, &m_tVBO);
	glDeleteBuffers(1, &m_nVBO);

	generateVBOs();

	if (animated)
		generateLinks(textured, increase);
}

void Billboard::generateLinks(bool textured, float increase)
{
	std::vector<unsigned int> links(m_vertices.size());
	std::vector<unsigned int> linksCopy;

	m_animatedHull = true;

	//find duplicate vertices

	std::vector<unsigned int> linksSet;
	linksSet.resize(m_vertices.size());
	unsigned int index = 0;
	for (auto i = m_vertices.begin(); i != m_vertices.end(); i++, index++)
	{
		Vec3& v1 = *i;

		unsigned int index2 = index + 1;
		for (auto j = i + 1; j != m_vertices.end(); j++, index2++)
		{
			Vec3& v2 = *j;

			if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
			{
				auto alreadyIn = std::find(linksSet.begin(), linksSet.end(), index2);

				if (alreadyIn == linksSet.end())
				{
					links[index2] = index;
					linksSet.push_back(index2);
				}
			}
			else
			{
				auto alreadyIn = std::find(linksSet.begin(), linksSet.end(), index2);

				if (alreadyIn == linksSet.end())
					links[index2] = index2;
			}
		}
	}

	//keep mapping of duplicate indices of vertices
	linksCopy = links;

	//keep unique list of indices
	std::sort(links.begin(), links.end());
	links.erase(std::unique(links.begin(), links.end()), links.end());

	//create a map of value per index
	std::map<unsigned int, float> linksMap;
	bool reverse = true;
	for (auto iter = links.begin(); iter != links.end(); iter++)
	{
		linksMap[*iter] = (reverse) ? 50 : -50;

		reverse = !reverse;
	}

	//assign the value to each vertex and store it in vertex attribute for the shader
	std::vector<Vec3> linksValues;
	for (int i = 0; i < m_vertices.size(); i++)
	{
		Vec3 v = Vec3(0, 0, 0);

		v.z = linksMap[linksCopy[i]];

		linksValues.push_back(v);
	}


	if (m_linksVBO != 0)
		glDeleteBuffers(1, &m_linksVBO);

	glGenBuffers(1, &m_linksVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_linksVBO);
	glBufferData(GL_ARRAY_BUFFER, linksValues.size()*sizeof(Vec3), &(linksValues[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (!textured)
	{
		m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_ANIMATED_KEY);

		if (m_program == NULL)
		{
			m_program = new CCGLProgram();
			MESH_INIT_PHONG_ANIMATED(m_program);
			CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_ANIMATED_KEY);
		}
	}
	else
	{
		m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_TEXTURE_ANIMATED_KEY);

		if (m_program == NULL)
		{
			m_program = new CCGLProgram();
			MESH_INIT_PHONG_TEXTURE_ANIMATED(m_program);
			CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_TEXTURE_ANIMATED_KEY);
		}
	}

	setShaderProgram(m_program);
	
	glBindAttribLocation(getShaderProgram()->getProgram(), LINKS_VERTEX_ATTRIB, "a_links");
	glEnableVertexAttribArray(LINKS_VERTEX_ATTRIB);

	initShaderLocations();
	
	m_shaderLocations["CC_Time"] = glGetUniformLocation(getShaderProgram()->getProgram(), "CC_Time");
}

void Billboard::dehull()
{
	if (!m_hulled)
		return;
	
	if (m_animatedHull && m_textured)
	{
		m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_TEXTURE_KEY);
		
		if (m_program == NULL)
		{
			m_program = new CCGLProgram();
			MESH_INIT_PHONG_TEXTURE(m_program);
			CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_TEXTURE_KEY);
		}
	}
	else
	{
		m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_KEY);
		
		if (m_program == NULL)
		{
			m_program = new CCGLProgram();
			MESH_INIT_PHONG(m_program);
			CCShaderCache::sharedShaderCache()->addProgram(m_program, PHONG_SHADER_KEY);
		}
	}
	
	m_hulled = m_animatedHull = false;
	
	setShaderProgram(m_program);
	
	glDeleteBuffers(1, &m_linksVBO);
	//glDisableVertexAttribArray(LINKS_VERTEX_ATTRIB);
	
	m_vertices.clear();
	m_normals.clear();
	m_texels.clear();
	
	createQuad(m_aabb.max.x*2.0f, m_aabb.max.y*2.0f);
	generateVBOs();
	initShaderLocations();
}