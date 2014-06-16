#include "VBOCache.h"

using namespace cocos3d;

VBOCache::VBOCache() : m_cacheInvalidated(false)
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(VBOCache::listenBackToForeground),
		EVENT_COME_TO_FOREGROUND,
		NULL);
#endif
}

VBOCache::~VBOCache()
{
	purgeCache();
}

VBOCache* VBOCache::sharedVBOCache()
{
	static VBOCache* cache = nullptr;

	if (cache == nullptr)
	{
		cache = new VBOCache();
		cache->autorelease();
		cache->retain();
	}

	return cache;
}

bool VBOCache::getVBO(const std::string& id, GLuint *vertices, GLuint *normals, GLuint *texels)
{
	if (m_vbos.find(id) != m_vbos.end())
	{
		*vertices = m_vbos[id].vertex;
		*normals = m_vbos[id].normal;
		*texels = m_vbos[id].texel;
		return true;
	}
	
	glGenBuffers(1, vertices);
	glGenBuffers(1, normals);
	glGenBuffers(1, texels);

	VBOSet newSet = { *vertices, *normals, *texels, 0 };

	m_vbos[id] = newSet;

	return false;
}

void VBOCache::addDataToVBOs(const std::string& id,
							 const std::vector<Vec3>& vertices,
							 const std::vector<Vec3>& normals,
							 const std::vector<Vec2>& texels,
							 bool overwrite)
{
	if (m_vbos.find(id) == m_vbos.end())
		return;

	VBOSet vbos = m_vbos[id];
	
	glBindBuffer(GL_ARRAY_BUFFER, vbos.vertex);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vec3), &(vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (texels.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbos.texel);
		glBufferData(GL_ARRAY_BUFFER, texels.size()*sizeof(Vec2), &(texels[0]), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbos.normal);
	glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(Vec3), &(normals[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBOCache::purgeCache()
{
	m_cacheInvalidated = false;

	for (auto iter = m_vbos.begin(); iter != m_vbos.end(); iter++)
	{
		VBOSet& set = iter->second;

		if (set.vertex != 0)
			glDeleteBuffers(1, &set.vertex);

		if (set.normal != 0)
			glDeleteBuffers(1, &set.normal);

		if (set.texel != 0)
			glDeleteBuffers(1, &set.texel);
	}

	m_vbos.clear();
}

void VBOCache::listenBackToForeground(CCObject *obj)
{
	m_cacheInvalidated = true;
}

bool VBOCache::cacheIsInvalid()
{
	return m_cacheInvalidated;
}