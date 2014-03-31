#ifndef __CUBE_H__
#define __CUBE_H__
#include "Node3D.h"

namespace cocos3d
{
	class Cube : public Node3D
	{
	public:
		CREATE_FUNC(Cube);

		virtual bool init();
		void draw3D();

	private:
		CCTexture2D* m_texture;
	};
}
#endif