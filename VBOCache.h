#ifndef __VBO_CACHE_H__
#define __VBO_CACHE_H__
#include "cocos2d.h"
#include <string>
#include <map>
#include "Node3D.h"

using namespace std;
using namespace cocos2d;

namespace cocos3d
{
	class VBOCache : public CCObject
	{
	public:
		VBOCache();
		~VBOCache();

		static VBOCache* sharedVBOCache();

		bool getVBO(const std::string& id, GLuint *vertices, GLuint *normals, GLuint *texels);
		//void getVBO(const std::string& id, GLuint *interleaved);

		void addDataToVBOs (const std::string& id,
							const std::vector<Vec3>& vertices,
							const std::vector<Vec3>& normals,
							const std::vector<Vec2>& texels,
							bool overwrite = false);

		void purgeCache();

		void listenBackToForeground(CCObject *obj);

		bool cacheIsInvalid();
	private:
		struct VBOSet
		{
			GLuint vertex, normal, texel, interleaved;
		};

		map<std::string,VBOSet> m_vbos;

		bool m_cacheInvalidated;
	};
}
#endif
