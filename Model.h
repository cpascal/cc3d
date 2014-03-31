#ifndef __MESH_H__
#define __MESH_H__
#include "Node3D.h"
#include "MTLParser.h"

namespace cocos3d
{
	class Light;

	class Model : public Node3D, public CCRGBAProtocol
	{
	public:

		Model();
		~Model();

		static Model* createWithFiles(const string& objFile, 
									  const string& mtlFile, 
									  float scale = 1.0f, 
									  const string& texture = "");

		static Model* createWithBuffers(const string& obj, 
										const string& mtl, 
										float scale = 1.0f,
										const string& textureName = "", 
										const char* textureBuffer = NULL, 
										unsigned long size = 0);

		virtual void setScale(float scale);

		virtual bool initWithFiles(const string& objFile, 
								   const string& mtlFile, 
								   float scale = 1.0f, 
								   const string& texture = "");

		virtual bool initWithBuffers(const string& obj,
									 const string& mtl, 
									 float scale = 1.0f, 
									 const string& textureName = "", 
									 const char* textureBuffer = NULL, 
									 unsigned long size = 0);

		void renderLines(bool lines){ m_lines = lines; }

		virtual void draw3D();

		virtual void addLight(Light* light);
		virtual void removeLight(Light* light);
		virtual void removeAllLights();
		const std::vector<Light*>& getLights();

		const ccVertex3F& getCenter();
		float getRadius();

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

	protected:
		void generateVBOs();
		virtual void setupMatrices();
		virtual void setupVertices();
		virtual void setupLights();
		void setupMaterial(const ccVertex3F& diffuse, const ccVertex3F& specular);
		void clearLights();

		CCTexture2D* m_dTexture;
		MTLParser m_parser;
		CCGLProgram* m_program;
		vector<Light*> m_lights;
		bool m_lightsDirty;

		ccVertex3F *m_lightsDiffuses,
					*m_lightsAmbience,
					*m_lightsPositions;

		bool *m_lightsEnabled;
		float* m_lightsIntensity;

		GLuint m_pVBO, m_tVBO, m_nVBO;
		float m_dt;
	private:
		bool m_lines;
		float m_opacity;
	};
}
#endif