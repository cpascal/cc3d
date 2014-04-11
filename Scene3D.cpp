#include "Scene3D.h"

using namespace cocos3d;

bool Scene3D::init()
{
	m_cache = new RenderCache();

	return CCScene::init();
}

Scene3D::~Scene3D()
{
	delete m_cache;
}

RenderCache* Scene3D::getCache()
{
	return m_cache;
}