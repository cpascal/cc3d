#include "Cube.h"

using namespace cocos3d;

bool Cube::init()
{
	m_texture = CCTextureCache::sharedTextureCache()->addImage("HelloWorld.png");

	return Node3D::init();
}

void Cube::draw3D()
{
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	float posX = m_position.x + this->m_pParent->getPositionX();
	float posY = m_position.y + this->m_pParent->getPositionY();

	kmMat4 matrixP;
	kmMat4 matrixMV;
	kmMat4 matrixMVP;
	
	kmMat3 rotation;
	kmVec3 translation;
	kmMat4 rotationAndMove;

	kmQuaternion quat;
		
	GLuint matrixId = glGetUniformLocation(getShaderProgram()->getProgram(), "CC_MVPMatrix");
	
	kmGLGetMatrix(KM_GL_PROJECTION, &matrixP );
	kmGLGetMatrix(KM_GL_MODELVIEW, &matrixMV );

	kmQuaternionRotationYawPitchRoll(&quat, m_yaw, m_pitch, m_roll);
	kmMat3RotationQuaternion(&rotation, &quat);

	kmVec3Fill(&translation, posX, posY, m_fullPosition.z);// this->m_obPosition.x, this->m_obPosition.y, 400);

	kmMat4RotationTranslation(&rotationAndMove, &rotation, &translation);

	kmMat4Multiply(&matrixMVP, &matrixP, &matrixMV);
	kmMat4Multiply(&matrixMVP, &matrixMVP, &rotationAndMove);				// apply rotation and translation to the matrix

	getShaderProgram()->setUniformLocationWithMatrix4fv(matrixId, matrixMVP.mat, 1);

	// texture for the box	
	ccGLBindTexture2D( m_texture->getName() );

	//draw the box
	ccVertex3F vertices[4];
	ccVertex2F uv[4];

	glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, uv);

	float x = 0;
	float y = 0;
	float len = 40*m_fScaleX;

	/////////// front
	vertices[0] = vertex3(x-len,y-len,len);
	vertices[1] = vertex3(x-len,y+len,len);
	vertices[2] = vertex3(x+len,y-len,len);
	vertices[3] = vertex3(x+len,y+len,len);

	uv[0] = vertex2(0, 1);
	uv[1] = vertex2(0, 0);
	uv[2] = vertex2(1, 1);
	uv[3] = vertex2(1, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	////////// right
	vertices[0] = vertex3(x+len,y-len,len);
	vertices[1] = vertex3(x+len,y+len,len);
	vertices[2] = vertex3(x+len,y-len,-len);
	vertices[3] = vertex3(x+len,y+len,-len);

	uv[0] = vertex2(0, 1);
	uv[1] = vertex2(0, 0);
	uv[2] = vertex2(1, 1);
	uv[3] = vertex2(1, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	///////// back
	vertices[0] = vertex3(x+len,y-len,-len);
	vertices[1] = vertex3(x+len,y+len,-len);
	vertices[2] = vertex3(x-len,y-len,-len);
	vertices[3] = vertex3(x-len,y+len,-len);

	uv[0] = vertex2(0, 1);
	uv[1] = vertex2(0, 0);
	uv[2] = vertex2(1, 1);
	uv[3] = vertex2(1, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	////////// left
	vertices[0] = vertex3(x-len,y-len,len);
	vertices[1] = vertex3(x-len,y+len,len);
	vertices[2] = vertex3(x-len,y-len,-len);
	vertices[3] = vertex3(x-len,y+len,-len);

	uv[0] = vertex2(0, 1);
	uv[1] = vertex2(0, 0);
	uv[2] = vertex2(1, 1);
	uv[3] = vertex2(1, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	

	///////// top
	vertices[0] = vertex3(x+len,y+len,len);
	vertices[1] = vertex3(x-len,y+len,len);
	vertices[2] = vertex3(x+len,y+len,-len);
	vertices[3] = vertex3(x-len,y+len,-len);

	uv[0] = vertex2(0, 0);
	uv[1] = vertex2(1, 0);
	uv[2] = vertex2(0, 1);
	uv[3] = vertex2(1, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	///////// bottom
	vertices[0] = vertex3(x+len,y-len,len);
	vertices[1] = vertex3(x-len,y-len,len);
	vertices[2] = vertex3(x+len,y-len,-len);
	vertices[3] = vertex3(x-len,y-len,-len);

	uv[0] = vertex2(0, 0);
	uv[1] = vertex2(1, 0);
	uv[2] = vertex2(0, 1);
	uv[3] = vertex2(1, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}