#ifndef __RENDER_CACHE_H__
#define __RENDER_CACHE_H__
#include "cocos2d.h"
#include <string>
#include <map>

using namespace std;
using namespace cocos2d;

namespace cocos3d
{
	class RenderCache : public CCObject
	{
	public:
		~RenderCache();

		void addTexture(const string& key, CCTexture2D* texture);
		void removeTexture(const string& key);

		CCTexture2D* getTexture(const string& key);
	protected:
		map<string,CCTexture2D*> m_textures;
	};
}
#endif
