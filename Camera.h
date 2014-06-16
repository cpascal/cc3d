#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "cocos2d.h"
#include "Node3D.h"

using namespace cocos2d;

namespace cocos3d
{
	class Camera;

	class Frustum
	{
	public:
		enum Planes
		{
			NEARP = 0,
			FARP,
			BOTTOM,
			TOP,
			LEFT,
			RIGHT,
			ALL_PLANES = 6
		};
		
		Frustum(Camera* camera);
		bool isPointInFrustum(kmVec3 &p);
		bool isPointInFrustum(kmVec3 &p, Planes plane);
		bool isBoxInFrustum(kmAABB& box);
		bool isBoxInFrustumPerPoint(kmAABB& box, Planes plane = ALL_PLANES);

	private:
		kmPlane m_planes[ALL_PLANES];
		Camera* m_camera;
	};

	class Camera : public Node3D
	{
	public:

		CREATE_FUNC(Camera);

		virtual bool init();

		void setPosition(float x, float y);
		void setPosition(const Vec3& position);
		void setPosition(const CCPoint& position);
		const CCPoint& getPosition();
		const Vec3& get3DPosition();
		void getPosition(float* x, float* y);
		void  setPositionX(float x);
		float getPositionX(void);
		void  setPositionY(float y);
		float getPositionY(void);
		void setPosition(float x, float y, float z);
		void setPositionZ(float z);

		void lookAt(const Vec3& position);
		void lookAt(const CCPoint& position);

		const Vec3& getLookAt();

		void setProjection(float fov, float ratio, float near, float far);
		void setFOV(float fov);
		void setNear(float nearV);
		void setFar(float farV);
		void setNearFar(float nearV, float farV);
		void setUp(const Vec3& up);

		bool isDirty();
		void notDirty();

		bool isObjectVisible(Node3D* node, Frustum::Planes plane);

		const kmMat4& getProjectionMatrix();
		const kmMat4& getViewMatrix();

	protected:
		void recalculateProjection();
		void recalculateView();
		kmMat4 m_projectionMatrix, m_viewMatrix;
		kmVec3 m_eye, m_center, m_up;
		GLfloat m_fov, m_ratio, m_near, m_far;
		Vec3 m_lookAt3d;

		bool m_projectionDirty, m_viewDirty;

		friend class Frustum;
		friend class Model;
	};
}
#endif
