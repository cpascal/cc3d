#include "Layer3D.h"
#include "Node3D.h"
#include "Light.h"
#include "Camera.h"

using namespace cocos3d;

bool Layer3D::init()
{
	m_fixedLights = true;
	m_lightsDirty = false;
	m_camera = NULL;

	return true;
}

void Layer3D::visit()
{
	if (m_camera == NULL)
		add3DCamera(Camera::create());

	CCLayer::visit();
}

void Layer3D::add3DCamera(Camera* camera)
{
	if (m_camera != NULL)
		m_camera->release();

	m_camera = camera;
	m_originalCamPos = m_camera->get3DPosition();
	m_originalCamCenter = m_camera->getLookAt();
	addChild(camera);
}

Camera* Layer3D::get3DCamera()
{
	return m_camera;
}

void Layer3D::addLight(Light* light)
{
	if (m_lights.size() == 4)
		return;

	for (auto iter = m_lights.begin();
		 iter != m_lights.end();
		 iter++)
	{
		if (*iter == light)
			return;
	}

	m_lights.push_back(light);
	addChild(light);

	m_lightsDirty = true;
}

void Layer3D::removeLight(Light* light)
{
	for (auto iter = m_lights.begin();
		 iter != m_lights.end();
		 iter++)
	{
		if (*iter == light)
		{
			m_lights.erase(iter);
			removeChild(light);
			return;
		}
	}

	m_lightsDirty = true;
}

void Layer3D::removeAllLights()
{
	for (auto iter = m_lights.begin();
		 iter != m_lights.end();
		 iter++)
	{
		removeChild(*iter);
	}

	m_lights.clear();
	m_lightsDirty = true;
}

void Layer3D::setFixedLights(bool fixedLights)
{
	m_fixedLights = fixedLights;
}

void Layer3D::setPosition(const CCPoint& position)
{
	if (!m_fixedLights)
	{
		for (auto iter = m_lights.begin(); 
			 iter != m_lights.end();
			 iter++)
		{
			Light* light = *iter;

			Vec3 newPosition(light->getPosition().x + position.x, light->getPosition().y + position.y, 0);

			light->setPosition(newPosition);
		}

		m_lightsDirty = true;
	}

	if (m_camera != NULL)
	{
		const Vec3 camPos = m_camera->get3DPosition();
		const Vec3 lookAt = m_camera->getLookAt();
	
		CCSize size = CCDirector::sharedDirector()->getWinSize();

		//FIXME: We are fixing the camera at the middle!!!!!
		Vec3 newPos(static_cast<GLfloat>(size.width/2.0 - position.x), static_cast<GLfloat>(size.height/2.0 - position.y), camPos.z);
		Vec3 newLookAt(newPos.x, newPos.y, lookAt.z);
		
		m_camera->setPosition(newPos);
		m_camera->lookAt(newLookAt);
	}

	CCLayer::setPosition(position);
}