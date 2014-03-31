#include "Model.h"
#include "Light.h"
#include "shaders.h"

using namespace cocos3d;

#define MAX_LIGHTS 4

Model::Model()
: m_opacity(1.0)
, m_lines(false)
, m_pVBO(0)
, m_nVBO(0)
, m_lightsDirty(false)
, m_lightsAmbience(NULL)
, m_lightsDiffuses(NULL)
, m_lightsPositions(NULL)
, m_lightsIntensity(NULL)
, m_lightsEnabled(NULL)
{
	m_lightsAmbience = new ccVertex3F[MAX_LIGHTS]();
	m_lightsDiffuses = new ccVertex3F[MAX_LIGHTS]();
	m_lightsPositions = new ccVertex3F[MAX_LIGHTS]();
	m_lightsIntensity = new float[MAX_LIGHTS];
	m_lightsEnabled = new bool[MAX_LIGHTS];
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

Model* Model::createWithFiles(const string& objFile, 
							  const string& mtlFile, 
							  float scale,
							  const string& texture)
{
	Model *pRet = new Model();
	if (pRet && pRet->initWithFiles(objFile,mtlFile,scale, texture))
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

Model* Model::createWithBuffers(const string& obj, 
								const string& mtl, 
								float scale,
								const string& textureName, 
								const char* textureBuffer, 
								unsigned long size)
{
	Model *pRet = new Model();
	if (pRet && pRet->initWithBuffers(obj,mtl,scale,textureName,textureBuffer,size))
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

bool Model::initWithFiles(const string& objFile, const string& mtlFile, float scale, const string& texture)
{
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

	return pRet && Node3D::init();
}

bool Model::initWithBuffers(const string& obj, const string& mtl, float scale, const string& textureName, const char* textureBuffer, unsigned long size)
{
	m_scale = scale;

	if (textureName != "")
	{
		m_dTexture = CCTextureCache::sharedTextureCache()->textureForKey(textureName.c_str());

		if (m_dTexture == NULL)
		{
		
			CCImage image;
			bool res = image.initWithImageData((void *)textureBuffer, size);//, CCImage::EImageFormat::kFmtPng,0,0,8);

			if (res == true)
				m_dTexture = CCTextureCache::sharedTextureCache()->addUIImage(&image,textureName.c_str());
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

	return pRet && Node3D::init();
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

	m_parser.clearPositions();
	m_parser.clearNormals();

	m_dt = 0;
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
	GLuint location;

	if (m_lightsDirty == false && m_lights.size() == 0)
	{
		clearLights();

		Light* light = Light::create();

		ccVertex3F ambient = { 0, 0, 0};
		ccVertex3F diffuse = { 1, 1, 1};
		ccVertex3F position = { 0, 0, getRadius()*2.0f };

		light->setAmbientDiffuseSpecularIntensity(ambient, diffuse, ambient, 1.0);
		light->setPosition(position);
		light->setEnabled(true);

		addLight(light);

		return;//lights are dirty. we'll skip a cycle to get light
	}
	else
	if (m_lightsDirty)
	{
		clearLights();

		int i = 0;

		for (auto iter = m_lights.begin(); i < MAX_LIGHTS; i++)
		{
			if (i < (int)m_lights.size() && iter != m_lights.end())
			{
				Light* light = *iter;

				m_lightsAmbience[i] = light->getAmbient(); 
				m_lightsDiffuses[i] = light->getDiffuse();
				m_lightsIntensity[i] = light->getIntensity();

				m_lightsPositions[i].x = light->getPosition().x * -m_scale;
				m_lightsPositions[i].y = light->getPosition().y * -m_scale;
				m_lightsPositions[i].z = light->getPosition().z * -m_scale;

				if (light->isEnabled())
					m_lightsEnabled[i] = true;

				iter++;
			}
		}
	}

	location = getShaderProgram()->getUniformLocationForName("uLightEnabled");
	if (location != -1)
		glUniform1iv(location, MAX_LIGHTS, (GLint*)m_lightsEnabled);

	location = getShaderProgram()->getUniformLocationForName("uLightAmbience");
	if (location != -1)
		glUniform3fv(location, MAX_LIGHTS, (GLfloat*)m_lightsAmbience);

	location = getShaderProgram()->getUniformLocationForName("uLightDiffuse");
	if (location != -1)
		glUniform3fv(location, MAX_LIGHTS, (GLfloat*)m_lightsDiffuses);

	location = getShaderProgram()->getUniformLocationForName("uLightPosition");
	if (location != -1)
		glUniform3fv(location, MAX_LIGHTS, (GLfloat*)m_lightsPositions);

	location = getShaderProgram()->getUniformLocationForName("uLightIntensity");
	if (location != -1)
		glUniform1fv(location,  MAX_LIGHTS, (GLfloat*)m_lightsIntensity);

	m_lightsDirty = false;
}

void Model::setupVertices()
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

void Model::setupMatrices()
{
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	kmMat4 matrixP;
	kmMat4 matrixM;
	kmMat4 matrixMV;
	kmMat4 matrixMVP;	
	kmMat4 normalMatrix; // As 4x4. Shader has the responsability of making it the top-left 3x3
	kmMat3 rotation;
	kmVec3 translation;
	kmMat4 rotationAndMove;
	kmMat4 scale;
	kmVec3 eye, center, up;
	kmQuaternion quat;
	GLuint location;

	float eyeZ = size.height / 1.1566f;
	//Adjust to parent layer
	float pDeltaX = 0.0f; 
	float pDeltaY = 0.0f;

	if (getParent() != NULL)
	{
		pDeltaX = getParent()->getPositionX();
		pDeltaY = getParent()->getPositionY();
	}

	//projection
	kmGLGetMatrix(KM_GL_PROJECTION, &matrixP);

	//model matrix
	kmMat4Identity(&matrixM);

	//View matrix
	kmVec3Fill(&eye, size.width/2.0f, size.height/2.0f, eyeZ);
	kmVec3Fill(&center, size.width/2.0f, size.height/2.0f, 0);
	kmVec3Fill(&up, 0.0f, 1.0f, 0.0f);
	kmMat4LookAt(&matrixMV, &eye, &center, &up);
	kmMat4Multiply(&matrixMV, &matrixM, &matrixMV);

	//normal matrix
	kmMat4Inverse(&normalMatrix, &matrixMV);
	kmMat4Transpose(&normalMatrix, &normalMatrix);

	//MVP matrix
	kmMat4Multiply(&matrixMVP, &matrixP, &matrixMV);

	//translation rotation
	kmQuaternionRotationYawPitchRoll(&quat, m_yaw, m_pitch, m_roll);
	kmMat3RotationQuaternion(&rotation, &quat);
	kmVec3Fill(&translation,m_fullPosition.x + pDeltaX , m_fullPosition.y + pDeltaY, m_fullPosition.z);
	kmMat4RotationTranslation(&rotationAndMove, &rotation, &translation);
	kmMat4Multiply(&matrixMVP, &matrixMVP, &rotationAndMove);
	kmMat4Scaling(&scale, m_scale, m_scale, m_scale);
	kmMat4Multiply(&matrixMVP, &matrixMVP, &scale);

	//pass matrices to shader
	location = getShaderProgram()->getUniformLocationForName("CC_MVPMatrix");
	getShaderProgram()->setUniformLocationWithMatrix4fv(location, matrixMVP.mat, 1);

	location = getShaderProgram()->getUniformLocationForName("CC_MVMatrix");
	getShaderProgram()->setUniformLocationWithMatrix4fv(location, matrixMV.mat, 1);

	location = getShaderProgram()->getUniformLocationForName("CC_NormalMatrix");
	glUniformMatrix4fv(location, 1, 0, normalMatrix.mat);

	location = getShaderProgram()->getUniformLocationForName("mode");
	glUniform1i(location, 4);

	location = getShaderProgram()->getUniformLocationForName("alpha");
	glUniform1f(location, m_opacity);
}

void Model::draw3D()
{
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	setupMatrices();
	setupLights();

	//Setup texture for simple Phong shader
	GLint textLocation = glGetUniformLocation(getShaderProgram()->getProgram(), "uTexture");
	if (m_dTexture != NULL)
	{
		ccGLBindTexture2D(m_dTexture->getName());
		glUniform1i(textLocation, 1);
	}
	else
	{
		glUniform1i(textLocation, 0);	
	}

	//render by material
	glEnable(GL_CULL_FACE);
	for (int i=0; i < (int)m_parser.materials().size(); ++i)
	{
		setupMaterial(m_parser.diffuses()[i],m_parser.speculars()[i]);		
		setupVertices();
		
		if (m_lines)
			glDrawArrays(GL_LINES, m_parser.firsts()[i], m_parser.counts()[i]);
		else
			glDrawArrays(GL_TRIANGLES, m_parser.firsts()[i], m_parser.counts()[i]);
    }

	glDisable(GL_CULL_FACE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();	
}

void Model::setupMaterial(const ccVertex3F& diffuses, const ccVertex3F& speculars)
{
	GLint location;

	location = getShaderProgram()->getUniformLocationForName("uDiffuse");
	getShaderProgram()->setUniformLocationWith3f(location,diffuses.x,diffuses.y,diffuses.z);

	location = getShaderProgram()->getUniformLocationForName("uSpecular");
	getShaderProgram()->setUniformLocationWith3f(location,speculars.x,speculars.y,speculars.z);
}

void Model::addLight(Light* light)
{
	if (light == NULL)
		return;

	if (m_lights.size() == MAX_LIGHTS)
		return; // Output a warning

	for (auto iter = m_lights.begin(); iter != m_lights.end(); iter++)
	{
		if (*iter == light)
			return;
	}

	m_lights.push_back(light);
	light->setParent(this);
	light->retain();

	m_lightsDirty = true;
}

void Model::removeLight(Light* light)
{
	if (m_lights.size() == 0)
		return;

	for (auto iter = m_lights.begin(); iter != m_lights.end(); iter++)
	{
		if (*iter == light)
		{
			m_lights.erase(iter);
			light->setParent(NULL);
			light->release();
			return;
		}
	}
}

void Model::removeAllLights()
{
	if (m_lights.size() == 0)
		return;

	for (auto iter = m_lights.begin(); iter != m_lights.end(); iter++)
	{
		Light* light = *iter;

		light->setParent(NULL);
		light->release();
	}

	m_lights.clear();
}

const std::vector<Light*>& Model::getLights()
{
	return m_lights;
}

const ccVertex3F& Model::getCenter()
{ 
	return m_parser.getCenter(); 
}

float Model::getRadius()
{ 
	return m_parser.getRadius();
}
