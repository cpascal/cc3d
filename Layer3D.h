#ifndef __LAYER_3D_H__
#define __LAYER_3D_H__
#include "cocos2d.h"
#include "Node3D.h"

using namespace cocos2d;

namespace cocos3d
{
	class Light;
	class Camera;

	class Layer3D : public CCLayer
	{
	public:

		CREATE_FUNC(Layer3D);

		virtual bool init();

		virtual void visit();

		void add3DCamera(Camera* camera);
		Camera* get3DCamera();

		void addLight(Light* light);
		void removeLight(Light* light);
		void removeAllLights();
		void setFixedLights(bool fixedLights = true);
		bool hasLights(){ return (m_lights.size() > 0); }
		std::vector<Light*>& getLights(){ return m_lights; }
		bool lightsDirty(){ return m_lightsDirty; }
		void cleanDirtyLights(){ m_lightsDirty = false; }
        void makeLightsDirty(){ m_lightsDirty = true; }

		virtual void setPosition(const CCPoint& position);
		virtual void setPositionX(float posX){ setPosition(CCPoint(posX, getPositionY())); }
		virtual void setPositionY(float posY){ setPosition(CCPoint(getPositionX(), posY)); }
	private:
		void createDefaultCamera();
		std::vector<Light*> m_lights;
		bool m_fixedLights, m_lightsDirty;
		Camera* m_camera;
		Vec3 m_originalCamPos, m_originalCamCenter;

		friend class Light;
	};
}
#endif
