#include "Layer3D.h"
#include "Node3D.h"
#include "Light.h"

using namespace cocos3d;

bool Layer3D::init()
{
	m_fixedLights = true;
	m_lightsDirty = false;

	return true;
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
	CCLayer::setPosition(position);

	if (!m_fixedLights)
	{
		for (auto iter = m_lights.begin(); 
			 iter != m_lights.end();
			 iter++)
		{
			Light* light = *iter;

			ccVertex3F newPosition = 
			{  
				light->getPosition().x + position.x,  
				light->getPosition().y + position.y 
			};

			light->setPosition(newPosition);
		}

		m_lightsDirty = true;
	}
}

void Layer3D::setPositionX(float posX)
{
	CCLayer::setPositionX(posX);

	if (!m_fixedLights)
	{
		for (auto iter = m_lights.begin(); 
			 iter != m_lights.end();
			 iter++)
		{
			Light* light = *iter;

			ccVertex3F newPosition = 
			{ 
				 light->getPosition().x + posX, 
				 light->getPosition().y + getPositionY()
			};

			light->setPosition(newPosition);
		}

		m_lightsDirty = true;
	}
}

void Layer3D::setPositionY(float posY)
{
	CCLayer::setPositionY(posY);

	if (!m_fixedLights)
	{
		for (auto iter = m_lights.begin(); 
			 iter != m_lights.end();
			 iter++)
		{
			Light* light = *iter;

			ccVertex3F newPosition = 
			{ 
				 light->getPosition().x + getPositionX(), 
				 light->getPosition().y + posY
			};

			light->setPosition(newPosition);
		}

		m_lightsDirty = true;
	}
}