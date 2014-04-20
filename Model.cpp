#include "Model.h"
#include "Light.h"
#include "Layer3D.h"
#include "Camera.h"
#include "shaders.h"
#include "Scene3D.h"
#include <limits>

using namespace cocos3d;

#define MAX_LIGHTS 4

Model::Model()
: m_opacity(1.0)
, m_lines(false)
, m_pVBO(0)
, m_nVBO(0)
, m_lightsAmbience(NULL)
, m_lightsDiffuses(NULL)
, m_lightsPositions(NULL)
, m_lightsIntensity(NULL)
, m_lightsEnabled(NULL)
, m_defaultLightUsed(false)
, m_lightsToSet(false)
, m_drawOBB(false)
, m_culling(true)
, m_shadowMapSet(false)
{
	m_lightsAmbience = new ccVertex3F[MAX_LIGHTS]();
	m_lightsDiffuses = new ccVertex3F[MAX_LIGHTS]();
	m_lightsPositions = new ccVertex3F[MAX_LIGHTS]();
	m_lightsIntensity = new float[MAX_LIGHTS]();
	m_lightsEnabled = new bool[MAX_LIGHTS]();
}

Model::~Model()
{
	if (m_pVBO != 0)
		glDeleteBuffers(1, &m_pVBO);

	if (m_nVBO != 0)
		glDeleteBuffers(1, &m_nVBO);

	delete [] m_lightsAmbience;
	delete [] m_lightsDiffuses;
	delete [] m_lightsPositions;
	delete [] m_lightsIntensity;
	delete [] m_lightsEnabled;
}

Model* Model::createWithFiles(const string& id,
							  const string& objFile, 
							  const string& mtlFile, 
							  float scale,
							  const string& texture)
{
	Model *pRet = new Model();
	if (pRet && pRet->initWithFiles(id,objFile,mtlFile,scale, texture))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

Model* Model::createWithBuffers(const string& id,
								const string& obj, 
								const string& mtl, 
								float scale,
								const string& textureName, 
								const char* textureBuffer, 
								unsigned long size)
{
	Model *pRet = new Model();
	if (pRet && pRet->initWithBuffers(id,obj,mtl,scale,textureName,textureBuffer,size))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

bool Model::initWithFiles(const string& id,
						  const string& objFile, 
						  const string& mtlFile, 
						  float scale, 
						  const string& texture)
{
	m_id = id;
	m_scale = scale;
	
	if (texture != "")
		m_dTexture = CCTextureCache::sharedTextureCache()->addImage(texture.c_str());
	else
		m_dTexture = NULL;

	string fullPathObj = CCFileUtils::sharedFileUtils()->fullPathForFilename(objFile.c_str());
	string fullPathMtl = CCFileUtils::sharedFileUtils()->fullPathForFilename(mtlFile.c_str());

	bool pRet = m_parser.readFile(fullPathObj, fullPathMtl, scale);

	m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_KEY);

	if (m_program == NULL)
	{
		m_program = new CCGLProgram();
		MESH_INIT_PHONG(m_program);
		CCShaderCache::sharedShaderCache()->addProgram(m_program,PHONG_SHADER_KEY);
	}

	setShaderProgram(m_program);

	generateVBOs();
	initShaderLocations();

	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
                                                                  callfuncO_selector(Model::listenBackToForeground),
                                                                  EVENT_COME_TO_FOREGROUND,
                                                                  NULL);

	return pRet && Node3D::init();
}

bool Model::initWithBuffers(const string& id,
							const string& obj,
							const string& mtl, 
							float scale, 
							const string& textureName, 
							const char* textureBuffer, 
							unsigned long size)
{
	m_id = id;
	m_scale = scale;

	if (textureName != "")
	{
		m_dTexture = CCTextureCache::sharedTextureCache()->textureForKey(textureName.c_str());

		if (m_dTexture == NULL)
		{
		
			CCImage* image = new CCImage();
			bool res = image->initWithImageData((void *)textureBuffer, size);//, CCImage::EImageFormat::kFmtPng,0,0,8);

			if (res == true)
				m_dTexture = CCTextureCache::sharedTextureCache()->addUIImage(image,textureName.c_str());
		}
	}
    else
    {
        m_dTexture = NULL;
    }

	bool pRet = m_parser.readBuffer(obj, mtl, scale);

    if (pRet)
    {
        m_program = CCShaderCache::sharedShaderCache()->programForKey(PHONG_SHADER_KEY);

        if (m_program == NULL)
        {
            m_program = new CCGLProgram();
            MESH_INIT_PHONG(m_program);
            CCShaderCache::sharedShaderCache()->addProgram(m_program,PHONG_SHADER_KEY);
        }
    }

	setShaderProgram(m_program);

	generateVBOs();
	initShaderLocations();
#if CC_ENABLE_CACHE_TEXTURE_DATA
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
                                                                  callfuncO_selector(Model::listenBackToForeground),
                                                                  EVENT_COME_TO_FOREGROUND,
                                                                  NULL);
#endif
	return pRet && Node3D::init();
}

void Model::listenBackToForeground(CCObject *obj)
{
	m_program = new CCGLProgram();
	MESH_INIT_PHONG(m_program);
	CCShaderCache::sharedShaderCache()->addProgram(m_program,PHONG_SHADER_KEY);
	setShaderProgram(m_program);
	generateVBOs();
	initShaderLocations();

	m_lightsToSet = true;
}

void Model::generateVBOs()
{
	glGenBuffers(1, &m_pVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_pVBO);
	glBufferData(GL_ARRAY_BUFFER, m_parser.positions().size()*sizeof(ccVertex3F), &(m_parser.positions()[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindAttribLocation(getShaderProgram()->getProgram(),5, "a_normal");
	
	glGenBuffers(1, &m_nVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_nVBO);
	glBufferData(GL_ARRAY_BUFFER, m_parser.normals().size()*sizeof(ccVertex3F), &(m_parser.normals()[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#if !CC_ENABLE_CACHE_TEXTURE_DATA
	m_parser.clearPositions();
	m_parser.clearNormals();
#endif
}

void Model::initShaderLocations()
{
#define SETUP_LOCATION(name) m_shaderLocations[name] = getShaderProgram()->getUniformLocationForName(name);

	//lights
	SETUP_LOCATION("uLightEnabled");
	SETUP_LOCATION("uLightAmbience");
	SETUP_LOCATION("uLightDiffuse");
	SETUP_LOCATION("uLightPosition");
	SETUP_LOCATION("uLightIntensity");

	//matrices
	SETUP_LOCATION("CC_MVPMatrix");
	SETUP_LOCATION("CC_MVMatrix");
	SETUP_LOCATION("CC_MMatrix");
	SETUP_LOCATION("CC_VMatrix");
	SETUP_LOCATION("CC_NormalMatrix");
	SETUP_LOCATION("uShadowProjectionMatrix");

	//options
	SETUP_LOCATION("mode");
	SETUP_LOCATION("alpha");

	//model material
	SETUP_LOCATION("uDiffuse");
	SETUP_LOCATION("uSpecular");

	//texture
	SETUP_LOCATION("uTexture");
	SETUP_LOCATION("uShadowMap");
	SETUP_LOCATION("uShadowMapEnabled");
	
	unsigned int textureId = 0;

	if (m_dTexture != NULL)
		glUniform1i(m_shaderLocations["uTexture"], textureId++);

	glUniform1i(m_shaderLocations["uShadowMap"], textureId);
	glUniform1i(m_shaderLocations["uShadowMapEnabled"], (GLint)false);
}

void Model::setScale(float scale)
{
	m_scale = scale;
}

void Model::clearLights()
{
	memset(m_lightsAmbience,0,sizeof(ccVertex3F)*MAX_LIGHTS);
	memset(m_lightsDiffuses,0,sizeof(ccVertex3F)*MAX_LIGHTS);
	memset(m_lightsPositions,0,sizeof(ccVertex3F)*MAX_LIGHTS);
	memset(m_lightsEnabled,0,sizeof(bool)*MAX_LIGHTS);
	memset(m_lightsIntensity,0,sizeof(float)*MAX_LIGHTS);
}

void Model::setupLights()
{
	Layer3D* parent = (Layer3D*)m_pParent;

	if (parent->getLights().size() == 0 && !m_defaultLightUsed)
	{
		clearLights();

		ccVertex3F ambient = { 0, 0, 0};
		ccVertex3F diffuse = { 1, 1, 1};
		ccVertex3F position = { -100 , 852, 736 * 2 };

		m_lightsAmbience[0] = ambient;
		m_lightsDiffuses[0] = diffuse;
		m_lightsIntensity[0] = 1.0f;
		m_lightsPositions[0] = position;
		m_lightsEnabled[0] = true;

		m_defaultLightUsed = true;

		m_lightsToSet = true;

		parent->cleanDirtyLights();
	}
	else
	if (parent->lightsDirty())
	{
		m_defaultLightUsed = false;

		m_lightsToSet = true;

		auto lights = parent->getLights();

		clearLights();

		int i = 0;

		for (auto iter = lights.begin(); i < MAX_LIGHTS; i++)
		{
			if (i < (int)lights.size() && iter != lights.end())
			{
				Light* light = *iter;

				m_lightsAmbience[i] = light->getAmbient(); 
				m_lightsDiffuses[i] = light->getDiffuse();
				m_lightsIntensity[i] = light->getIntensity();

				m_lightsPositions[i].x = light->get3DPosition().x * m_scale;
				m_lightsPositions[i].y = light->get3DPosition().y * m_scale;
				m_lightsPositions[i].z = light->get3DPosition().z * m_scale;

				if (light->isEnabled())
					m_lightsEnabled[i] = true;

				iter++;
			}
		}
 	}

	if (m_lightsToSet)
	{
		glUniform1iv(m_shaderLocations["uLightEnabled"], MAX_LIGHTS, (GLint*)m_lightsEnabled);
		glUniform3fv(m_shaderLocations["uLightAmbience"], MAX_LIGHTS, (GLfloat*)m_lightsAmbience);
		glUniform3fv(m_shaderLocations["uLightDiffuse"], MAX_LIGHTS, (GLfloat*)m_lightsDiffuses);
		glUniform3fv(m_shaderLocations["uLightPosition"], MAX_LIGHTS, (GLfloat*)m_lightsPositions);
		glUniform1fv(m_shaderLocations["uLightIntensity"],  MAX_LIGHTS, (GLfloat*)m_lightsIntensity);
	}

	m_lightsToSet = false;
}

void Model::setupAttribs()
{
	//setup texels or vertices for textures
	if (m_parser.texels().size() > 0)
		glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, &(m_parser.texels()[0]));

	if (m_nVBO == 0)
	{
		glVertexAttribPointer(glGetAttribLocation(getShaderProgram()->getProgram(), "a_normal"), 3, GL_FLOAT, GL_FALSE, 0, &(m_parser.normals()[0]));
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_nVBO);
		glVertexAttribPointer(glGetAttribLocation(getShaderProgram()->getProgram(), "a_normal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	
	//vertices as shader attributes
	if (m_pVBO == 0)
	{
		glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, 0, &(m_parser.positions()[0]));
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_pVBO);
		glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
}

void Model::transformAABB(const kmAABB& box)
{
	kmVec3 v[8];
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	kmVec3Fill(&v[0],box.min.x,box.max.y, box.max.z);
	kmVec3Fill(&v[1],box.min.x,box.max.y, box.min.z);
	kmVec3Fill(&v[2],box.max.x,box.max.y, box.min.z);
	kmVec3Fill(&v[3],box.max.x,box.max.y, box.max.z);
	kmVec3Fill(&v[4],box.min.x,box.min.y, box.max.z);
	kmVec3Fill(&v[5],box.min.x,box.min.y, box.min.z);
	kmVec3Fill(&v[6],box.max.x,box.min.y, box.min.z);
	kmVec3Fill(&v[7],box.max.x,box.min.y, box.max.z);

	for (int i = 0; i < 8 ; i++)
	{
		kmVec3TransformCoord(&v[i],&v[i],&m_matrixM);
	}

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
#undef max()
#undef min()
#endif

	float xMax = numeric_limits<float>::min();
    float yMax = numeric_limits<float>::min();
    float zMax = numeric_limits<float>::min();

    float xMin = numeric_limits<float>::max();
    float yMin = numeric_limits<float>::max();
    float zMin = numeric_limits<float>::max();

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    int numVerts = 8;

    for (int i = 0; i < numVerts; ++i)
    {
        x = v[i].x;
        y = v[i].y;
        z = v[i].z;

        if (x < xMin)
            xMin = x;

        if (x > xMax)
            xMax = x;

        if (y < yMin)
            yMin = y;

        if (y > yMax)
            yMax = y;

        if (z < zMin)
            zMin = z;

        if (z > zMax)
            zMax = z;
    }

	kmVec3Fill(&(m_bbox.min), xMin, yMin, zMin);
	kmVec3Fill(&(m_bbox.max), xMax, yMax, zMax);
}

void Model::setupMatrices()
{
	Layer3D* parent = dynamic_cast<Layer3D*>(m_pParent);

	CC_ASSERT(parent != NULL);

	if (m_dirty || parent->get3DCamera()->isDirty())
	{
		kmMat4 rotation;
		kmMat4 translation;
		kmMat4 scale; 
		kmQuaternion quat;

		//model matrix
		kmMat4Identity(&m_matrixM);
		kmMat4Translation(&translation,m_fullPosition.x, m_fullPosition.y, m_fullPosition.z);
		kmQuaternionRotationYawPitchRoll(&quat, -m_yaw, -m_pitch, -m_roll);
		kmMat4RotationQuaternion(&rotation, &quat);
		kmMat4Multiply(&translation,&translation,&rotation);
		kmMat4Multiply(&m_matrixM, &m_matrixM, &translation);
		kmMat4Scaling(&scale, m_scale, m_scale, m_scale);
		kmMat4Multiply(&m_matrixM, &m_matrixM, &scale);
	
		const kmMat4 matrixP = parent->get3DCamera()->getProjectionMatrix();	
		
		//model view matrix
		kmMat4Multiply(&m_matrixMV, &(parent->get3DCamera()->getViewMatrix()), &m_matrixM);

		//normal matrix
		kmMat4Inverse(&m_matrixNormal, &m_matrixM);
		kmMat4Transpose(&m_matrixNormal, &m_matrixNormal);
	
		//MVP matrix
		kmMat4Multiply(&m_matrixMVP, &matrixP, &m_matrixMV);

		transformAABB(m_parser.m_aabb);
	}

	//pass matrices to shader
	glUniformMatrix4fv(m_shaderLocations["CC_MVPMatrix"], 1, 0, m_matrixMVP.mat);
	glUniformMatrix4fv(m_shaderLocations["CC_MVMatrix"], 1, 0, m_matrixMV.mat);
	glUniformMatrix4fv(m_shaderLocations["CC_MMatrix"], 1, 0, m_matrixM.mat);
	glUniformMatrix4fv(m_shaderLocations["CC_VMatrix"], 1, 0, parent->get3DCamera()->getViewMatrix().mat);
	glUniformMatrix4fv(m_shaderLocations["CC_NormalMatrix"], 1, 0, m_matrixNormal.mat);
	glUniform1i(m_shaderLocations["mode"], 4);
	glUniform1f(m_shaderLocations["alpha"], m_opacity);

}

void Model::setupShadow()
{
	kmMat4 shadowProjectionMatrix;

	if (!m_shadowMapSet)
	{
		kmMat4Identity(&shadowProjectionMatrix);
	}
	else
	{
		Layer3D* parent = dynamic_cast<Layer3D*>(m_pParent);

		CC_ASSERT(parent != NULL);

		kmMat4 tmp,projection,shadowView;

		kmMat4Identity(&shadowProjectionMatrix);
		kmMat4Translation(&shadowProjectionMatrix, 0.5f, 0.5f, 0.5f);
		kmMat4Scaling(&shadowProjectionMatrix, 0.5f, 0.5f, 0.5f);

		kmVec3 eye = parent->get3DCamera()->m_eye;
		kmVec3 center = parent->get3DCamera()->m_center;
		kmVec3 up = { 0, 1, 0 };

		eye.x = 0;
		center.x = 0.13f*CCDirector::sharedDirector()->getWinSize().width;
		eye.y = eye.y - (CCDirector::sharedDirector()->getWinSize().height/2.0f);
		center.y = center.y - (CCDirector::sharedDirector()->getWinSize().height/2.0f + CCDirector::sharedDirector()->getWinSize().height*0.08f);

		projection = parent->get3DCamera()->getProjectionMatrix();
		kmMat4LookAt(&shadowView, &eye, &center, &up);

		kmMat4Multiply(&shadowProjectionMatrix, &shadowProjectionMatrix, kmMat4Multiply(&tmp, &projection, &shadowView));
	}

	glUniformMatrix4fv(m_shaderLocations["uShadowProjectionMatrix"], 1, 0, shadowProjectionMatrix.mat);
}

void Model::setupTextures()
{
	unsigned int textureId = 0;

	//Setup texture for simple Phong shader
	if (m_dTexture != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, m_dTexture->getName());
		glActiveTexture(GL_TEXTURE0 + textureId);
		glUniform1i(m_shaderLocations["uTexture"], textureId++);
	}

	Scene3D* scene = (Scene3D*)m_pParent->getParent();
	CCTexture2D* shadowMap = scene->getCache()->getTexture("shadow_map");

	if (shadowMap != NULL)
	{
		glUniform1i(m_shaderLocations["uShadowMapEnabled"], (GLint)true);
		glBindTexture(GL_TEXTURE_2D, shadowMap->getName());
		glActiveTexture(GL_TEXTURE0 + textureId);
		m_shadowMapSet = true;
	}
}

void Model::draw3D()
{
	setupMatrices();
	setupShadow();

	bool toRender = true;

	if (m_culling)
		toRender = ((Layer3D*)m_pParent)->get3DCamera()->isObjectVisible(this, Frustum::ALL_PLANES);

	if (!toRender)
		return;

	setupLights();
	setupTextures();

	//render by material
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	for (int i=0; i < (int)m_parser.materials().size(); ++i)
	{
		setupMaterial(m_parser.diffuses()[i],m_parser.speculars()[i]);		
		setupAttribs();

		if (m_lines)
			glDrawArrays(GL_LINES, m_parser.firsts()[i], m_parser.counts()[i]);
		else
			glDrawArrays(GL_TRIANGLES, m_parser.firsts()[i], m_parser.counts()[i]);

		CC_INCREMENT_GL_DRAWS(1);
    }

	glDisable(GL_CULL_FACE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();	

	if (m_drawOBB)
		renderOOBB();
}

void Model::setFrustumCulling(bool culling)
{
	m_culling = culling;
}

void Model::renderOOBB()
{
	CCGLProgram* program = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionColor);
	program->setUniformsForBuiltins();
	program->use();

	static GLint loc = -1;

	if (loc == -1)
		loc = program->getUniformLocationForName("CC_MVPMatrix");
	
	program->setUniformLocationWithMatrix4fv(loc, m_matrixMVP.mat, 1);
	
	ccVertex3F points[24];
	
	points[0].x = m_parser.m_aabb.min.x; points[0].y = m_parser.m_aabb.min.y; points[0].z = m_parser.m_aabb.max.z; 
	points[1].x = m_parser.m_aabb.min.x; points[1].y = m_parser.m_aabb.max.y; points[1].z = m_parser.m_aabb.max.z;
	points[2].x = m_parser.m_aabb.min.x; points[2].y = m_parser.m_aabb.max.y; points[2].z = m_parser.m_aabb.max.z;
	points[3].x = m_parser.m_aabb.max.x; points[3].y = m_parser.m_aabb.max.y; points[3].z = m_parser.m_aabb.max.z;
	points[4].x = m_parser.m_aabb.max.x; points[4].y = m_parser.m_aabb.max.y; points[4].z = m_parser.m_aabb.max.z;
	points[5].x = m_parser.m_aabb.max.x; points[5].y = m_parser.m_aabb.min.y; points[5].z = m_parser.m_aabb.max.z;
	points[6].x = m_parser.m_aabb.max.x; points[6].y = m_parser.m_aabb.min.y; points[6].z = m_parser.m_aabb.max.z;
	points[7].x = m_parser.m_aabb.min.x; points[7].y = m_parser.m_aabb.min.y; points[7].z = m_parser.m_aabb.max.z;

	points[8].x = m_parser.m_aabb.min.x; points[8].y = m_parser.m_aabb.min.y; points[8].z = m_parser.m_aabb.min.z; 
	points[9].x = m_parser.m_aabb.min.x; points[9].y = m_parser.m_aabb.max.y; points[9].z = m_parser.m_aabb.min.z;
	points[10].x = m_parser.m_aabb.min.x; points[10].y = m_parser.m_aabb.max.y; points[10].z = m_parser.m_aabb.min.z;
	points[11].x = m_parser.m_aabb.max.x; points[11].y = m_parser.m_aabb.max.y; points[11].z = m_parser.m_aabb.min.z;
	points[12].x = m_parser.m_aabb.max.x; points[12].y = m_parser.m_aabb.max.y; points[12].z = m_parser.m_aabb.min.z;
	points[13].x = m_parser.m_aabb.max.x; points[13].y = m_parser.m_aabb.min.y; points[13].z = m_parser.m_aabb.min.z;
	points[14].x = m_parser.m_aabb.max.x; points[14].y = m_parser.m_aabb.min.y; points[14].z = m_parser.m_aabb.min.z;
	points[15].x = m_parser.m_aabb.min.x; points[15].y = m_parser.m_aabb.min.y; points[15].z = m_parser.m_aabb.min.z;

	points[16].x = m_parser.m_aabb.min.x; points[16].y = m_parser.m_aabb.min.y; points[16].z = m_parser.m_aabb.min.z; 
	points[17].x = m_parser.m_aabb.min.x; points[17].y = m_parser.m_aabb.min.y; points[17].z = m_parser.m_aabb.max.z;
	points[18].x = m_parser.m_aabb.min.x; points[18].y = m_parser.m_aabb.max.y; points[18].z = m_parser.m_aabb.min.z;
	points[19].x = m_parser.m_aabb.min.x; points[19].y = m_parser.m_aabb.max.y; points[19].z = m_parser.m_aabb.max.z;
	
	points[20].x = m_parser.m_aabb.max.x; points[20].y = m_parser.m_aabb.min.y; points[20].z = m_parser.m_aabb.min.z;
	points[21].x = m_parser.m_aabb.max.x; points[21].y = m_parser.m_aabb.min.y; points[21].z = m_parser.m_aabb.max.z;
	
	points[22].x = m_parser.m_aabb.max.x; points[22].y = m_parser.m_aabb.max.y; points[22].z = m_parser.m_aabb.min.z;
	points[23].x = m_parser.m_aabb.max.x; points[23].y = m_parser.m_aabb.max.y; points[23].z = m_parser.m_aabb.max.z;

	glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, 0, points);
 	glDrawArrays(GL_LINES, 0, 24);

	//ccColor4F color = { 1.0f, 0.5f, 0.0f, 1.0f };
	//ccDrawSolidRect(CCPoint(m_bbox.min.x,m_bbox.min.y),CCPoint(m_bbox.max.x, m_bbox.max.y),color);

	CC_INCREMENT_GL_DRAWS(1);
}

void Model::setupMaterial(const ccVertex3F& diffuses, const ccVertex3F& speculars)
{
	glUniform3f(m_shaderLocations["uDiffuse"], diffuses.x,diffuses.y,diffuses.z);
	glUniform3f(m_shaderLocations["uSpecular"], speculars.x, speculars.y, speculars.z);
}

const ccVertex3F& Model::getCenter()
{ 
	return get3DPosition();
}

float Model::getRadius()
{ 
	return m_parser.getRadius() * m_scale;
}

bool Model::isOutOfCamera(Frustum::Planes plane)
{
	Layer3D* parent = dynamic_cast<Layer3D*>(m_pParent);

	if (parent == NULL)
		return true;

	return !(parent->get3DCamera()->isObjectVisible(this,plane));
}

void Model::setDrawOBB(bool draw)
{ 
	m_drawOBB = draw;
}

void Model::renderLines(bool lines)
{
	m_lines = lines;
}


//TODO: Move to an actions header/cpp

SpinBy* SpinBy::create(float fDuration, float fDeltaAngle)
{
    SpinBy *pRotateBy = new SpinBy();
    pRotateBy->initWithDuration(fDuration, fDeltaAngle);
    pRotateBy->autorelease();

    return pRotateBy;
}

bool SpinBy::initWithDuration(float fDuration, float fDeltaAngle)
{
    if (CCActionInterval::initWithDuration(fDuration))
    {
        m_fAngleX = m_fAngleY = fDeltaAngle;
        return true;
    }

    return false;
}

SpinBy* SpinBy::create(float fDuration, float fDeltaAngleX, float fDeltaAngleY, float fDeltaAngleZ)
{
    SpinBy *pRotateBy = new SpinBy();
    pRotateBy->initWithDuration(fDuration, fDeltaAngleX, fDeltaAngleY, fDeltaAngleZ);
    pRotateBy->autorelease();
    
    return pRotateBy;
}

bool SpinBy::initWithDuration(float fDuration, float fDeltaAngleX, float fDeltaAngleY, float fDeltaAngleZ)
{
    if (CCActionInterval::initWithDuration(fDuration))
    {
        m_fAngleX = fDeltaAngleX;
        m_fAngleY = fDeltaAngleY;
        return true;
    }
    
    return false;
}

CCObject* SpinBy::copyWithZone(CCZone *pZone)
{
    CCZone* pNewZone = NULL;
    SpinBy* pCopy = NULL;
    if(pZone && pZone->m_pCopyObject) 
    {
        //in case of being called at sub class
        pCopy = (SpinBy*)(pZone->m_pCopyObject);
    }
    else
    {
        pCopy = new SpinBy();
        pZone = pNewZone = new CCZone(pCopy);
    }

    CCActionInterval::copyWithZone(pZone);

    pCopy->initWithDuration(m_fDuration, m_fAngleX, m_fAngleY, m_fAngleZ);

    CC_SAFE_DELETE(pNewZone);
    return pCopy;
}

void SpinBy::startWithTarget(CCNode *pTarget)
{
    CCActionInterval::startWithTarget(pTarget);
	Model* model = (Model*)m_pTarget;
    m_fStartAngleX = model->getYaw();
    m_fStartAngleY = model->getRoll();
	m_fStartAngleZ = model->getPitch();
}

void SpinBy::update(float time)
{
    // XXX: shall I add % 360
    if (m_pTarget)
    {
		Model* model = (Model*)m_pTarget;
        model->setYaw(m_fStartAngleX + m_fAngleX * time);
        //model->setRoll(m_fStartAngleY + m_fAngleY * time);
		//model->setPitch(m_fStartAngleZ + m_fAngleZ * time);
    }
}

CCActionInterval* SpinBy::reverse(void)
{
    return SpinBy::create(m_fDuration, -m_fAngleX, -m_fAngleY, -m_fAngleZ);
}
