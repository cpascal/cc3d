#include "Light.h"
#include "Layer3D.h"

using namespace cocos3d;

CCLightTo* CCLightTo::create(float duration, Vec3& ambience, Vec3& diffuse, Vec3& specular)
{
    CCLightTo* pAction = new CCLightTo();

    pAction->initWithDuration(duration, ambience, diffuse, specular);
    pAction->autorelease();

    return pAction;
}

bool CCLightTo::initWithDuration(float duration, Vec3& ambience, Vec3& diffuse, Vec3& specular)
{
	if (CCActionInterval::initWithDuration(duration))
	{
		m_toAmbience = ambience;
		m_toDiffuse  = diffuse;
		m_toSpecular = specular;
		return true;
	}

	return false;
}

CCObject* CCLightTo::copyWithZone(CCZone *pZone)
{
    CCZone* pNewZone = NULL;
    CCLightTo* pCopy = NULL;
    if(pZone && pZone->m_pCopyObject)
    {
        //in case of being called at sub class
        pCopy = (CCLightTo*)(pZone->m_pCopyObject);
    }
    else
    {
        pCopy = new CCLightTo();
        pZone = pNewZone = new CCZone(pCopy);
    }
    
    CCActionInterval::copyWithZone(pZone);

	pCopy->initWithDuration(m_fDuration, m_toAmbience, m_toDiffuse, m_toSpecular);

    CC_SAFE_DELETE(pNewZone);

    return pCopy;
}

void CCLightTo::update(float time)
{
	Vec3 curAmb 
	(
		m_fromAmbience.x + (m_toAmbience.x - m_fromAmbience.x) * time,
		m_fromAmbience.y + (m_toAmbience.y - m_fromAmbience.y) * time,
		m_fromAmbience.z + (m_toAmbience.z - m_fromAmbience.z) * time
	);

	Vec3 curDif
	(
		m_fromDiffuse.x + (m_toDiffuse.x - m_fromDiffuse.x) * time,
		m_fromDiffuse.y + (m_toDiffuse.y - m_fromDiffuse.y) * time,
		m_fromDiffuse.z + (m_toDiffuse.z - m_fromDiffuse.z) * time
	);

	Vec3 curSpec
	(
		m_fromSpecular.x + (m_toSpecular.x - m_fromSpecular.x) * time,
		m_fromSpecular.y + (m_toSpecular.y - m_fromSpecular.y) * time,
		m_fromSpecular.z + (m_toSpecular.z - m_fromSpecular.z) * time
	);

	Light* light = dynamic_cast<Light*>(m_pTarget);
	if (light != NULL)
    {
		light->setAmbientDiffuseSpecularIntensity(curAmb, curDif, curSpec, 0.5f);// light->getIntensity());
		light->setParentDirty();
	}
}

void CCLightTo::startWithTarget(CCNode *pTarget)
{
    CCActionInterval::startWithTarget(pTarget);
  
	Light* light = dynamic_cast<Light*>(m_pTarget);
   
	if (light != NULL)
    {
        m_fromAmbience = light->getAmbient();
		m_fromDiffuse  = light->getDiffuse();
		m_fromSpecular = light->getSpecular();
    }
}

bool Light::init()
{
	m_enabled = true;
	m_intensity = 1.0f;

	return CCNode::init();
}

void Light::setAmbientDiffuseSpecularIntensity(const Vec3& ambient, const Vec3& diffuse, const Vec3& specular, float intensity)
{
	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
	m_intensity = intensity;
}

void Light::setAmbient(const Vec3& ambient)
{
	m_ambient = ambient;
}

void Light::setDiffuse(const Vec3& diffuse)
{
	m_diffuse = diffuse;
}

void Light::setSpecular(const Vec3& specular)
{
	m_specular = specular;
}

void Light::setIntensity(float intensity)
{
	m_intensity = intensity;
}

void Light::setParentDirty()
{
	Layer3D* node = dynamic_cast<Layer3D*>(m_pParent);

	if (node != NULL)
		node->m_lightsDirty = true;
}