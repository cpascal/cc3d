#include "Node3D.h"

using namespace cocos3d;

#define PARENT_LAYER_DELTA() \
if (getParent() != NULL) \
{\
	pDeltaX = getParent()->getPositionX();\
	pDeltaY = getParent()->getPositionY();\
}

Node3D::Node3D() 
: m_position(CCPoint(0,0))
, m_yaw(0)
, m_pitch(0)
, m_roll(0)
, m_dirty(true)
{
	m_fullPosition.x = m_fullPosition.y = m_fullPosition.z = 0.0f;
	m_center.x = m_center.y = m_center.z = 0.0f;
}

void Node3D::setPosition(float x, float y)
{
	m_position.x = x;
	m_position.y = y;
	m_fullPosition.x = x;
	m_fullPosition.y = y;

	m_dirty = true;
}

void Node3D::visit()
{
	if (getParent() != NULL && getShaderProgram() == NULL)
		setShaderProgram(getParent()->getShaderProgram());

	CCNode::visit();
}

void Node3D::setYawPitchRoll(const ccVertex3F& ypr)
{
	m_yaw = ypr.x;
	m_pitch = ypr.y;
	m_roll = ypr.z;

	m_dirty = true;
}

void Node3D::setYawPitchRoll(const float yaw, const float pitch, const float roll)
{
	m_yaw = yaw;
	m_pitch = pitch;
	m_roll = roll;

	m_dirty = true;
}

void Node3D::setYaw(const float yaw)
{
	m_yaw = yaw;

	m_dirty = true;
}

void Node3D::setPitch(const float pitch)
{
	m_pitch = pitch;

	m_dirty = true;
}

void Node3D::setRoll(const float roll)
{
	m_roll = roll;

	m_dirty = true;
}

void Node3D::draw()
{
	if (getShaderProgram() != NULL)
	{
		getShaderProgram()->use();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		draw3D();
		glDisable(GL_DEPTH_TEST);
	}

	m_dirty = false;
}

void Node3D::setPosition(const CCPoint &position)
{
	m_position = position;
	m_fullPosition.x = position.x;
	m_fullPosition.y = position.y;
	m_fullPosition.z = 0;

	m_dirty = true;
}

const CCPoint& Node3D::getPosition()
{
	return m_position;
}

void Node3D::getPosition(float* x, float* y)
{
	*x = m_position.x;
	*y = m_position.y;
}

void Node3D::setPositionX(float x)
{
	m_position.x = x;
	m_fullPosition.x = x;

	m_dirty = true;
}

float Node3D::getPositionX(void)
{
	return m_position.x;
}

void Node3D::setPositionY(float y)
{
	m_position.y = y;
	m_fullPosition.y = y;
}

float Node3D::getPositionY(void)
{
	return m_position.y;
}

void Node3D::setPosition(float x, float y, float z)
{
	setPosition(x,y);
	m_fullPosition.z = z;

	m_dirty = true;
}

void Node3D::setPosition(const ccVertex3F& position)
{
	m_position.x = position.x;
	m_position.y = position.y;
	m_fullPosition.x = position.x;
	m_fullPosition.y = position.y;
	m_fullPosition.z = position.z;

	m_dirty = true;
}

void Node3D::setPositionZ(float z)
{
	m_fullPosition.z = z;

	m_dirty = true;
}

const ccVertex3F& Node3D::get3DPosition()
{
	return m_fullPosition;
}
