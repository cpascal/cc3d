#include "Model.h"
#include "Layer3D.h"
#include "Camera.h"
#include "shaders.h"

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

	Layer3D* parent = (Layer3D*)m_pParent;

	if (parent->getLights().size() == 0 && !m_defaultLightUsed)
	{
		clearLights();

		ccVertex3F ambient = { 0, 0, 0};
		ccVertex3F diffuse = { 1, 1, 1};
		ccVertex3F position = { 2400 , 852, 736 * 2 };

		m_lightsAmbience[0] = ambient;
		m_lightsDiffuses[0] = diffuse;
		m_lightsIntensity[0] = 1.0f;
		m_lightsPositions[0] = position;
		m_lightsEnabled[0] = true;

		m_defaultLightUsed = true;

		parent->cleanDirtyLights();
	}
	else
	if (parent->lightsDirty())
	{
		m_defaultLightUsed = false;

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

				m_lightsPositions[i].x = light->getLightPosition().x * m_scale;
				m_lightsPositions[i].y = light->getLightPosition().y * m_scale;
				m_lightsPositions[i].z = light->getLightPosition().z * m_scale;

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

void Model::transformAABB(const kmAABB& box)
{
	kmVec3 v[8];

	kmVec3Fill(&v[0],box.min.x,box.max.y, box.max.z);
	kmVec3Fill(&v[1],box.min.x,box.max.y, box.min.z);
	kmVec3Fill(&v[2],box.max.x,box.max.y, box.min.z);
	kmVec3Fill(&v[3],box.max.x,box.max.y, box.max.z);
	kmVec3Fill(&v[4],box.min.x,box.min.y, box.max.z);
	kmVec3Fill(&v[5],box.min.x,box.min.y, box.min.z);
	kmVec3Fill(&v[6],box.max.x,box.min.y, box.min.z);
	kmVec3Fill(&v[7],box.max.x,box.min.y, box.max.z);

	Camera* camera = ((Layer3D*)getParent())->get3DCamera();

	kmMat4 t;
	kmMat4Translation(&t, CCDirector::sharedDirector()->getWinSize().width/2.0f, CCDirector::sharedDirector()->getWinSize().height/2.0f, camera->get3DPosition().z/2.0f );

	for (int i = 0; i < 8 ; i++)
	{
		kmVec3TransformCoord(&v[i],&v[i],&m_matrixMV);
		kmVec3TransformCoord(&v[i],&v[i],&t);
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
	CCSize size = CCDirector::sharedDirector()->getWinSize();
	Layer3D* parent = dynamic_cast<Layer3D*>(m_pParent);

	if (m_dirty || parent->get3DCamera()->isDirty())
	{
		parent->get3DCamera()->notDirty();

		const kmMat4 matrixP = parent->get3DCamera()->getProjectionMatrix();	
		kmMat4 rotation;
		kmMat4 translation;
		kmMat4 scale; 
		kmQuaternion quat;

		float eyeZ = size.height / 1.1566f;
		//Adjust to parent layer
		float pDeltaX = 0.0f; 
		float pDeltaY = 0.0f;

		if (getParent() != NULL)
		{
			pDeltaX = getParent()->getPositionX();
			pDeltaY = getParent()->getPositionY();
		}

		//model matrix
		kmMat4Identity(&m_matrixM);

		//model view matrix
		kmMat4Multiply(&m_matrixMV, &(parent->get3DCamera()->getViewMatrix()), &m_matrixM);

		//transformations
		kmMat4Translation(&translation,m_fullPosition.x + pDeltaX , m_fullPosition.y + pDeltaY, m_fullPosition.z);
		kmQuaternionRotationYawPitchRoll(&quat, -m_yaw, -m_pitch, -m_roll);
		kmMat4RotationQuaternion(&rotation, &quat);
		kmMat4Multiply(&translation,&translation,&rotation);
		kmMat4Multiply(&m_matrixMV, &m_matrixMV, &translation);
		kmMat4Scaling(&scale, m_scale, m_scale, m_scale);
		kmMat4Multiply(&m_matrixMV, &m_matrixMV, &scale);

		//normal matrix
		kmMat4Inverse(&m_matrixNormal, &m_matrixMV);
		kmMat4Transpose(&m_matrixNormal, &m_matrixNormal);

		//MVP matrix
		kmMat4Multiply(&m_matrixMVP, &matrixP, &m_matrixMV);

		transformAABB(m_parser.m_aabb);

		m_outOfCamera = parent->get3DCamera()->isObjectVisible(this,m_matrixMVP);

		//debug
		if (m_outOfCamera)
		{
			string s = "";
		}
		else
		{
			string s = "";
		}
	}

	GLuint location;

	//pass matrices to shader
	location = getShaderProgram()->getUniformLocationForName("CC_MVPMatrix");
	getShaderProgram()->setUniformLocationWithMatrix4fv(location, m_matrixMVP.mat, 1);

	location = getShaderProgram()->getUniformLocationForName("CC_MVMatrix");
	getShaderProgram()->setUniformLocationWithMatrix4fv(location, m_matrixMV.mat, 1);

	location = getShaderProgram()->getUniformLocationForName("CC_NormalMatrix");
	glUniformMatrix4fv(location, 1, 0, m_matrixNormal.mat);

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

	CCGLProgram* program = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionColor);
	program->setUniformsForBuiltins();
	program->use();
	
	GLint loc = program->getUniformLocationForName("CC_MVPMatrix");
	getShaderProgram()->setUniformLocationWithMatrix4fv(loc, m_matrixMVP.mat, 1);
	
	CCPoint points[8];
	
	points[0].x = m_parser.m_aabb.min.x; points[0].y = m_parser.m_aabb.min.y; 
	points[1].x = m_parser.m_aabb.min.x; points[1].y = m_parser.m_aabb.max.y; 
	
	points[2].x = m_parser.m_aabb.min.x; points[2].y = m_parser.m_aabb.max.y; 
	points[3].x = m_parser.m_aabb.max.x; points[3].y = m_parser.m_aabb.max.y; 
	
	points[4].x = m_parser.m_aabb.max.x; points[4].y = m_parser.m_aabb.max.y; 
	points[5].x = m_parser.m_aabb.max.x; points[5].y = m_parser.m_aabb.min.y;
	
	points[6].x = m_parser.m_aabb.max.x; points[6].y = m_parser.m_aabb.min.y; 
	points[7].x = m_parser.m_aabb.min.x; points[7].y = m_parser.m_aabb.min.y; 
	
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, points);
 	glDrawArrays(GL_LINES, 0, 8);
}

void Model::setupMaterial(const ccVertex3F& diffuses, const ccVertex3F& speculars)
{
	GLint location;

	location = getShaderProgram()->getUniformLocationForName("uDiffuse");
	getShaderProgram()->setUniformLocationWith3f(location,diffuses.x,diffuses.y,diffuses.z);

	location = getShaderProgram()->getUniformLocationForName("uSpecular");
	getShaderProgram()->setUniformLocationWith3f(location,speculars.x,speculars.y,speculars.z);
}

const ccVertex3F& Model::getCenter()
{ 
	return get3DPosition();
	//return m_parser.getCenter(); 
}

float Model::getRadius()
{ 
	return m_parser.getRadius() * m_scale;
}