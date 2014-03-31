#include "Light.h"

using namespace cocos3d;

#define UNIFORM_ENABLE "u_cc3LightIsLightEnabled"
#define UNIFORM_POSITION "u_cc3LightPositionModel"
#define UNIFORM_AMBIENT "u_cc3LightAmbientColor"
#define UNIFORM_DIFFUSE "u_cc3LightDiffuseColor"
#define UNIFORM_SPECULAR "u_cc3LightSpecularColor"
#define UNIFORM_ATTENUATION "u_cc3LightAttenuation"
#define UNIFORM_DIRECTION "u_cc3LightSpotDirectionModel"
#define UNIFORM_SPOT_EXPONENT "u_cc3LightSpotExponent"
#define UNIFORM_CUTTOFF_ANGLE "u_cc3LightSpotCutoffAngleCosine"

bool Light::init()
{
	m_enabled = true;
	m_intensity = 1.0f;
	return true;
}

void Light::setPosition(const ccVertex3F& position)
{
	m_position = position;
}

void Light::setAmbientDiffuseSpecularIntensity(const ccVertex3F& ambient, const ccVertex3F& diffuse, const ccVertex3F& specular, float intensity)
{
	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
	m_intensity = intensity;
}

void Light::setParent(Model* model)
{
	m_parent = model;
}

void Light::setAmbient(const ccVertex3F& ambient)
{
	m_ambient = ambient;
}

void Light::setDiffuse(const ccVertex3F& diffuse)
{
	m_diffuse = diffuse;
}

void Light::setSpecular(const ccVertex3F& specular)
{
	m_specular = specular;
}

void Light::setIntensity(float intensity)
{
	m_intensity = intensity;
}