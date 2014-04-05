#include "Camera.h"
#include <limits>

using namespace cocos3d;

bool Camera::init()
{
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	float eyeZ = CCDirector::sharedDirector()->getZEye();

	m_fov = 60;
	m_ratio = (GLfloat)size.width/size.height;
	m_near = 0.1f;
	m_far = eyeZ*2;
	m_dirty = true;
	
	recalculateProjection();

	kmVec3Fill(&m_eye, size.width/2.0f, size.height/2.0f, eyeZ);
	kmVec3Fill(&m_center, size.width/2.0f, size.height/2.0f, 0);
	kmVec3Fill(&m_up, 0.0f, 1.0f, 0.0f);
	kmMat4LookAt(&m_viewMatrix, &m_eye, &m_center, &m_up);

	return CCNode::init();
}

void Camera::setPosition(const ccVertex3F& position)
{
	m_eye.x = position.x;
	m_eye.y = position.y;
	m_eye.z = position.z;

	recalculateView();
}

void Camera::setPosition(const CCPoint& position)
{
	m_eye.x = position.x;
	m_eye.y = position.y;

	recalculateView();
}

const CCPoint& Camera::getPosition()
{
	m_position2d.x = m_eye.x;
	m_position2d.y = m_eye.y;

	return m_position2d;
}

const ccVertex3F& Camera::get3DPosition()
{
	m_position3d.x = m_eye.x;
	m_position3d.y = m_eye.y;
	m_position3d.z = m_eye.z;

	return m_position3d;
}

void Camera::lookAt(const ccVertex3F& position)
{
	m_center.x = position.x;
	m_center.y = position.y;
	m_center.z = position.z;

	recalculateView();
}

void Camera::lookAt(const CCPoint& position)
{
	m_center.x = position.x;
	m_center.y = position.y;

	recalculateView();
}

void Camera::setUp(const ccVertex3F& up)
{
	m_up.x = up.x;
	m_up.y = up.y;
	m_up.z = up.z;

	recalculateView();
}

const kmMat4& Camera::getViewMatrix()
{
	return m_viewMatrix;
}

const kmMat4& Camera::getProjectionMatrix()
{
	return m_projectionMatrix;
}

void Camera::recalculateView()
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_WP8
	recalculateProjection();
#endif
	kmMat4LookAt(&m_viewMatrix,&m_eye,&m_center,&m_up);

	m_dirty = true;
}

void Camera::recalculateProjection()
{
	kmMat4 i;
	kmMat4Identity(&i);

#if CC_TARGET_PLATFORM == CC_PLATFORM_WP8
            //if needed, we need to add a rotation for Landscape orientations on Windows Phone 8 since it is always in Portrait Mode
	kmMat4Multiply(&i, &i, CCEGLView::sharedOpenGLView()->getOrientationMatrix());
#endif
	kmMat4PerspectiveProjection(&m_projectionMatrix, m_fov, m_ratio, m_near, m_far);
#if CC_TARGET_PLATFORM == CC_PLATFORM_WP8
	kmMat4Multiply(&m_projectionMatrix, &i, &m_projectionMatrix);
#endif
	m_dirty = true;
}

void Camera::setFOV(float fov)
{
	m_fov = fov;

	recalculateProjection();
}

void Camera::setNear(float nearV)
{
	m_near = nearV;

	recalculateProjection();
}

void Camera::setFar(float farV)
{
	m_far = farV;

	recalculateProjection();
}

void Camera::setNearFar(float nearV, float farV)
{
	m_far = farV;
	m_near = nearV;

	recalculateProjection();
}

bool Camera::isObjectVisible(Node3D* node, kmMat4& mvp)
{
	Frustum frustum(this,mvp);

	kmAABB box;
	kmAABB tmp = node->getBoundingBox();

	return frustum.isBoxInFrustumPerPoint(tmp,Frustum::BOTTOM);
}

bool Camera::isObjectVisible(Node3D* node, kmMat4& mvp, Frustum::Planes plane)
{
	Frustum frustum(this,mvp);

	kmAABB box;
	kmAABB tmp = node->getBoundingBox();
	
	//return frustum.isPointInFrustum(p);

	return frustum.isBoxInFrustumPerPoint(tmp,plane);
}

Frustum::Frustum(Camera* camera, kmMat4& mvp)
{
	m_camera = camera;
	kmMat4 _mvp;
	kmMat4Multiply(&_mvp, &(m_camera->getProjectionMatrix()), &(m_camera->getViewMatrix()));

	kmMat4ExtractPlane(&m_planes[NEARP],  &_mvp, KM_PLANE_NEAR);
    kmMat4ExtractPlane(&m_planes[FARP],   &_mvp, KM_PLANE_FAR);
    kmMat4ExtractPlane(&m_planes[LEFT],   &_mvp, KM_PLANE_LEFT);
    kmMat4ExtractPlane(&m_planes[RIGHT],  &_mvp, KM_PLANE_RIGHT);
    kmMat4ExtractPlane(&m_planes[BOTTOM], &_mvp, KM_PLANE_BOTTOM);
    kmMat4ExtractPlane(&m_planes[TOP],    &_mvp, KM_PLANE_TOP);
}

kmVec3 getPositivePoint(const kmAABB& box, const kmVec3& direction)
{
	kmVec3 result = box.max;
	if( direction.x < 0 ) result.x = box.min.x;
    if( direction.y < 0 ) result.y = box.min.y;
    if( direction.z < 0 ) result.z = box.min.z;
	
	return result;
}

kmVec3 getNegativePoint(const kmAABB& box, const kmVec3& direction)
{
	kmVec3 result = box.min;
	if( direction.x < 0 ) result.x = box.max.x;
    if( direction.y < 0 ) result.y = box.max.y;
    if( direction.z < 0 ) result.z = box.max.z;

	return result;
}

bool Frustum::isBoxInFrustumPerPoint(kmAABB& box, Planes plane)
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

	for (int i = 0; i < 8; i++)
	{
		if (plane == N_PLANES)
		{
			if (isPointInFrustum(v[i]))
				return true;
		}
		else
		{
			if (isPointInFrustum(v[i],BOTTOM));
				return true;
		}
	}

	return false;
}

bool Frustum::isBoxInFrustum(kmAABB& box)
{
	bool result = true;
    int indexFirst = 0;
    int indexNumber = N_PLANES;
    
    for(int i = 0; i < N_PLANES; i++)
    {
        kmPlane plane = m_planes[static_cast<Planes>(i)];
        kmVec3 normal = {plane.a, plane.b, plane.c};
        kmVec3Normalize(&normal, &normal);
        kmVec3 positivePoint = getPositivePoint(box,normal);
        kmVec3 negativePoint = getNegativePoint(box,normal);
      
        if(kmPlaneDotCoord(&plane, &positivePoint) < 0)
            return false;
        if(kmPlaneDotCoord(&plane, &negativePoint) < 0)
            result = true;
    }
    
    return result;
}

bool Frustum::isPointInFrustum(kmVec3 &p)
{
    for(int i = 0; i < N_PLANES; i++)
    {
        if(kmPlaneDotCoord(&m_planes[static_cast<Planes>(i)], &p) < 0)
            return false;
    }

    return true;
}

bool Frustum::isPointInFrustum(kmVec3 &p, Planes plane)
{
    if(kmPlaneDotCoord(&m_planes[plane], &p) < 0)
		return false;

    return true;
}