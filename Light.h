#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "cocos2d.h"
#include "Node3D.h"

using namespace cocos2d;

namespace cocos3d
{
	class CCLightTo : public CCActionInterval
	{
	public:
		void update(float time);
		virtual CCObject* copyWithZone(CCZone* pZone);
	    virtual void startWithTarget(CCNode *pTarget);
		bool initWithDuration(float duration, Vec3& ambience, Vec3& diffuse, Vec3& specular);
	public:
		static CCLightTo* create(float duration, Vec3& ambience, Vec3& diffuse, Vec3& specular);
	private:
		Vec3 m_fromAmbience, m_fromDiffuse, m_fromSpecular, m_toAmbience, m_toDiffuse, m_toSpecular;
	}; 

	struct ccVertex4F
	{
		ccVertex4F() : x(0), y(0), z(0), w(0)
		{}

		ccVertex4F(float _x, float _y, float _z, float _w)
			: x(_x), y(_y), z(_z), w(_w)
		{}

		float x,y,z,w;
	};

	class Light : public Node3D
	{
	public:
		CREATE_FUNC(Light);

		static const int maxLights = 4;

		virtual bool init();

		void setEnabled(bool enabled){ m_enabled = enabled; }

		void setAmbientDiffuseSpecularIntensity(const Vec3& ambient, const Vec3& diffuse, const Vec3& specular, float intensity = 1.0f); 

		void setAmbient(const Vec3& ambient);
		void setDiffuse(const Vec3& diffuse);
		void setSpecular(const Vec3& specular);
		void setIntensity(const float);

		void setCutOffAngle(float angle){ m_cutoffAngleCosine = angle; }
		void setSpotExponent(float exponent){ m_spotExponent = exponent; }

		const Vec3& getAmbient(){ return m_ambient; }
		const Vec3& getDiffuse(){ return m_diffuse; }
		const Vec3& getSpecular(){ return m_specular; }
		const Vec3& getDirection(){ return m_direction; }
		const float getIntensity(){ return m_intensity; }

		bool isEnabled(){ return m_enabled; }

		void setParentDirty();

	protected:
		Vec3 m_ambient;
		Vec3 m_diffuse;
		Vec3 m_specular;
		Vec3 m_attenuation;
		Vec3 m_direction;
		float m_intensity;
		float m_spotExponent;
		float m_cutoffAngleCosine;
		bool m_enabled;

		CCNode* m_parent;
	};
}
#endif
