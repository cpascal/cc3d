#ifndef __SCENE_3D_H__
#define __SCENE_3D_H__
#include "cocos2d.h"
#include "RenderCache.h"

using namespace cocos2d;

namespace cocos3d
{
	class Scene3D : public CCScene
	{
	public:
		virtual ~Scene3D();

		CREATE_FUNC(Scene3D);

		virtual bool init();

		RenderCache* getCache();

	private:
		RenderCache* m_cache;
	};
}
#endif
