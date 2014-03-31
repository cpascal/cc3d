#ifndef __LAYER_3D_H__
#define __LAYER_3D_H__
#include "cocos2d.h"
#include "Light.h"

using namespace cocos2d;

namespace cocos3d
{
	class Layer3D : public CCLayer
	{
	public:

		CREATE_FUNC(Layer3D);

		bool init(){ return true; }

		//placeholder for layer lightning

		void addLight(Light* light){CC_UNUSED_PARAM(light);}
		void removeLight(Light* light){CC_UNUSED_PARAM(light);}
		void removeAllLights(){}
	};
}
#endif
