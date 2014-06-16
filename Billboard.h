#ifndef __BILLBOARD_H__
#define __BILLBOARD_H__
#include <vector>
#include "Model.h"

namespace cocos3d
{
	class Billboard : public Model
	{
	public:
		static Billboard* createWithSize(const CCSize& size, float thickness = 0);
		static Billboard* createWithTexture(CCTexture2D* texture, float thickness = 0);
		static Billboard* createWithTextureGrid(CCTexture2D* texture, const int framesPerRow, const CCSize& frameSize, float thickness = 0);

		virtual bool initWithSize(const CCSize& size, float thickness = 0);
		virtual bool initWithTexture(CCTexture2D* texture, float thickness = 0);
		virtual bool initWithTextureGrid(CCTexture2D* texture, const int framesPerRow, const CCSize& frameSize, float thickness = 0);

		virtual void setId(const std::string& id);

		void setDelay(float delay);
		
		void updateFrame(float dt);

		virtual void draw3D();

		virtual float getRadius();

		void setColor(const Vec3& color);

		void hull(int factor, bool textured = true, bool animated = false, float increase = 50, int axis = 0x000);
		void dehull();

		void listenBackToForeground(CCObject *obj);
	protected:
		Billboard();

		void createQuad(int width, int height);
		void createCube(int width, int height, float thickness);
		void createAnimatedQuad();
		void createAnimatedCube(float thickness);
		void setupAttribs();
		void setupAnimation();

		void triangulation(int factor);
		void generateLinks(bool textured, float increase);
		void generateVertexIndex();
		void generateTriangleStrip();

		std::vector<std::vector<Vec2> > m_texelsFrame;

		std::vector<unsigned int> m_indices;

		Vec3 m_color;

		GLuint m_linksVBO;
		
		bool m_hulled, m_animatedHull;
		float m_delay, m_at;
		CCSize m_frameSize;
		unsigned int m_rows, m_cols;
	};
}
#endif