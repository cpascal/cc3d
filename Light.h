#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "Model.h"

namespace cocos3d
{
	

	struct ccVertex4F
	{
		ccVertex4F() : x(0), y(0), z(0), w(0)
		{}

		ccVertex4F(float _x, float _y, float _z, float _w)
			: x(_x), y(_y), z(_z), w(_w)
		{}

		float x,y,z,w;
	};

	class Light : public CCObject
	{
	public:
		static ccVertex3F CC_VERTEX_3F(float x, float y, float z)
		{
			ccVertex3F v = { x, y, z};
			return v;
		}

		CREATE_FUNC(Light);

		virtual bool init();

		void setEnabled(bool enabled){ m_enabled = enabled; }

		void setPosition(const ccVertex3F& position);

		void setAmbientDiffuseSpecularIntensity(const ccVertex3F& ambient, const ccVertex3F& diffuse, const ccVertex3F& specular, float intensity = 1.0f); 

		void setAmbient(const ccVertex3F& ambient);
		void setDiffuse(const ccVertex3F& diffuse);
		void setSpecular(const ccVertex3F& specular);
		void setIntensity(const float);

		void setParent(Model* model);

		void setCutOffAngle(float angle){ m_cutoffAngleCosine = angle; }
		void setSpotExponent(float exponent){ m_spotExponent = exponent; }

		const ccVertex3F& getAmbient(){ return m_ambient; }
		const ccVertex3F& getDiffuse(){ return m_diffuse; }
		const ccVertex3F& getSpecular(){ return m_specular; }
		const ccVertex3F& getDirection(){ return m_direction; }
		const ccVertex3F& getPosition(){ return m_position; }
		const float getIntensity(){ return m_intensity; }

		bool isEnabled(){ return m_enabled; }

	protected:
		ccVertex3F m_position;
		ccVertex3F m_ambient;
		ccVertex3F m_diffuse;
		ccVertex3F m_specular;
		ccVertex3F m_attenuation;
		ccVertex3F m_direction;
		float m_intensity;
		float m_spotExponent;
		float m_cutoffAngleCosine;
		bool m_enabled;

		Model* m_parent;

		friend class Model;
	};
}
#endif