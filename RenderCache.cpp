#include "RenderCache.h"

using namespace cocos3d;

RenderCache::~RenderCache()
{
	for (auto iter = m_textures.begin();
		 iter != m_textures.end();
		 iter++)
	{
		iter->second->release();
	}
    
    m_textures.clear();
}

void RenderCache::addTexture(const string& key, CCTexture2D* texture)
{
	auto find = m_textures.find(key);

	if (find != m_textures.end())
		texture->release();

	m_textures[key] = texture;
	texture->retain();
}

void RenderCache::removeTexture(const string& key)
{
	auto found = m_textures.find(key);

	if (found != m_textures.end())
	{
		m_textures[key]->release();
		m_textures.erase(found);
	}
}

CCTexture2D* RenderCache::getTexture(const string& key)
{
	auto found = m_textures.find(key);

	if (found != m_textures.end())
		return m_textures[key];

	return NULL;
}
