#ifndef __MESH_H__
#define __MESH_H__
#include "MeshParser.h"
#include <vector>
#include "Node3D.h"
#include "Camera.h"

using namespace std;

namespace cocos3d
{
	class SpinBy : public CCActionInterval
	{
	public:
		/** creates the action */
		static SpinBy* create(float fDuration, float fDeltaAngle);
		/** initializes the action */
		bool initWithDuration(float fDuration, float fDeltaAngle);
    
		static SpinBy* create(float fDuration, float fDeltaAngleX, float fDeltaAngleY, float fDeltaAngleZ);
		bool initWithDuration(float fDuration, float fDeltaAngleX, float fDeltaAngleY, float fDeltaAngleZ);
		/**
		 *  @js NA
		 *  @lua NA
		 */
		virtual CCObject* copyWithZone(CCZone* pZone);
		virtual void startWithTarget(CCNode *pTarget);
		virtual void update(float time);
		virtual CCActionInterval* reverse(void);
    
	protected:
		float m_fAngleX;
		float m_fStartAngleX;
		float m_fAngleY;
		float m_fStartAngleY;
		float m_fAngleZ;
		float m_fStartAngleZ;
	};

	class Light;

	class Model : public Node3D, public CCRGBAProtocol
	{
	public:

		enum ShineMode
		{
			SHINE_SPECULAR = 2,
			SHINE_LAMBERTIAN = 3,
			NO_SHINE = 4
		};

		enum TexturePosition
		{
			DEFAULT_TEXTURE = -1,
			TEXTURE_0 = 0
		};

		virtual ~Model();

		static Model* createWithFiles(const std::string& id,
									  const std::string& objFile, 
									  const std::string& mtlFile, 
									  float scale = 1.0f, 
									  const std::string& texture = "");

		static Model* createWithBuffers(const std::string& id,
										const std::string& obj, 
										const std::string& mtl, 
										float scale = 1.0f,
										const std::string& textureName = "", 
										const char* textureBuffer = NULL, 
										unsigned long size = 0);

		virtual void setScale(float scale);

		virtual bool initWithFiles(const std::string& id,
								   const std::string& objFile, 
								   const std::string& mtlFile, 
								   float scale = 1.0f, 
								   const string& texture = "");

		virtual bool initWithBuffers(const std::string& id,
									 const std::string& obj,
									 const std::string& mtl, 
									 float scale = 1.0f, 
									 const std::string& textureName = "", 
									 const char* textureBuffer = NULL, 
									 unsigned long size = 0);

		
		virtual void draw3D();

		virtual const Vec3& getCenter();
		virtual float getRadius();

		//CCRGBA protocol

		virtual void setColor(const ccColor3B& color){ CC_UNUSED_PARAM(color); }
		virtual const ccColor3B& getColor(void){ return ccWHITE; }
		virtual const ccColor3B& getDisplayedColor(void){ return ccWHITE; }
		virtual GLubyte getDisplayedOpacity(void){ return (GLubyte)m_opacity*255.0f; }
		virtual GLubyte getOpacity(void){ return (GLubyte)m_opacity*255.0f; }
		virtual void setOpacity(GLubyte opacity){ m_opacity = (float)opacity/255.0f; }
		virtual void setOpacityModifyRGB(bool bValue){ CC_UNUSED_PARAM(bValue); }
		virtual bool isOpacityModifyRGB(void){ return false; }
		virtual bool isCascadeColorEnabled(void){ return false; }
		virtual void setCascadeColorEnabled(bool cascadeColorEnabled){ CC_UNUSED_PARAM(cascadeColorEnabled); }
		virtual void updateDisplayedColor(const ccColor3B& color){ CC_UNUSED_PARAM(color); }
		virtual bool isCascadeOpacityEnabled(void){ return false; }
		virtual void setCascadeOpacityEnabled(bool cascadeOpacityEnabled){ CC_UNUSED_PARAM(cascadeOpacityEnabled); }
		virtual void updateDisplayedOpacity(GLubyte opacity){ CC_UNUSED_PARAM(opacity); }

		virtual void setShineMode(ShineMode, float exponent);

		void backFaceCulling(bool culling);

		virtual void addCustomLights(const std::list<Light*>& lights);
		
		void addAnimationTextures(const std::vector<CCTexture2D*>& textures, float time = 0);
		void setCurrentTexture(int position);
		void nextTexture();

		void setTextureToAlpha();

		void setFrustumCulling(bool culling);
		bool isOutOfCamera(Frustum::Planes plane);
		void setDrawOBB(bool draw);
		void renderLines(bool lines);

		void listenBackToForeground(CCObject *obj);

		virtual void setId(const std::string& id){}
		const string& getId(){ return m_id; }
	protected:
		Model();

		void fillVectors(MeshParser* parser);
		void generateVBOs();
		void initShaderLocations();
		void setupMatrices();
		void setupLights();
		void setupTextures();
		void setupShadow();
		void setupMaterial(const Vec3& diffuse, const Vec3& specular);
		void setupTextureToAlpha();

		virtual void setupAttribs();

		void transformAABB(const kmAABB& box);
		void renderOOBB();

		void clearLights();

		CCTexture2D* m_dTexture;
		std::vector<CCTexture2D*> m_animationTextures;
		int m_currentTexture;
		float m_textureDt;
		float m_textureAt;

		bool m_textureToAlpha;

		CCGLProgram* m_program;

		Vec3 *m_lightsDiffuses,
					*m_lightsAmbience,
					*m_lightsPositions;

		bool m_customLights;
		bool* m_lightsEnabled;
		float* m_lightsIntensity;
		
		GLuint m_pVBO,
			   m_tVBO,
			   m_nVBO;

		kmMat4 m_matrixM,
			   m_matrixMV,
			   m_matrixMVP,
			   m_matrixNormal;

		map<string,GLint> m_shaderLocations;

		bool m_culling,
			 m_cullBackFace,
			 m_shadowMapSet,
			 m_lines,
			 m_drawOBB,
			 m_defaultLightUsed,
			 m_lightsToSet,
			 m_dirtyCheck;

		ShineMode m_shineMode;
		float m_opacity;
		float m_exponent;

		string m_id;
		bool m_textured;
		
		kmAABB m_aabb;

		float m_radius;

		unsigned int m_nframes, m_currentFrame;

		std::vector<Vec2> m_texels;
		std::vector<Vec3> m_vertices;
		std::vector<Vec3> m_normals;
		std::vector<std::string> m_materials;
		std::vector<Vec3> m_diffuses;
		std::vector<Vec3> m_speculars;
		std::vector<int> m_firsts;
		std::vector<int> m_counts;
	};
}
#endif