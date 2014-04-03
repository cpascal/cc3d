#ifndef __LAYER_3D_H__
#define __LAYER_3D_H__
#include "cocos2d.h"

using namespace cocos2d;

namespace cocos3d
{
	class Light;

	class Layer3D : public CCLayer
	{
	public:

		CREATE_FUNC(Layer3D);

		bool init();

		//placeholder for layer lightning

		void addLight(Light* light);
		void removeLight(Light* light);
		void removeAllLights();
		void setFixedLights(bool fixedLights = true);
		bool hasLights(){ return (m_lights.size() > 0); }
		std::vector<Light*>& getLights(){ return m_lights; }
		bool lightsDirty(){ m_lightsDirty = !m_lightsDirty; return !m_lightsDirty; }

		virtual void setPosition(const CCPoint& position);
		virtual void setPositionX(float posX);
		virtual void setPositionY(float posY);
	private:
		std::vector<Light*> m_lights;
		bool m_fixedLights, m_lightsDirty;

		friend class Light;
	};
}
#endif
