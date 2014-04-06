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
		
		Frustum(Camera* camera, kmMat4& mvp);
		bool isPointInFrustum(kmVec3 &p);
		bool isPointInFrustum(kmVec3 &p, Planes plane);
		bool isBoxInFrustum(kmAABB& box);
		bool isBoxInFrustumPerPoint(kmAABB& box, Planes plane = ALL_PLANES);

	private:
		kmPlane m_planes[ALL_PLANES];
		Camera* m_camera;
	};

	class Camera : public CCNode
	{
	public:

		CREATE_FUNC(Camera);

		virtual bool init();

		void setPosition(const ccVertex3F& position);
		void setPosition(const CCPoint& position);

		const CCPoint& getPosition();
		const ccVertex3F& get3DPosition();

		void lookAt(const ccVertex3F& position);
		void lookAt(const CCPoint& position);

		const ccVertex3F& getLookAt();

		void setFOV(float fov);
		void setNear(float nearV);
		void setFar(float farV);
		void setNearFar(float nearV, float farV);
		void setUp(const ccVertex3F& up);

		bool isDirty();
		void notDirty();

		bool isObjectVisible(Node3D* node, kmMat4& mvp, Frustum::Planes plane);

		const kmMat4& getProjectionMatrix();
		const kmMat4& getViewMatrix();

	protected:
		void recalculateProjection();
		void recalculateView();
		kmMat4 m_projectionMatrix, m_viewMatrix;
		kmVec3 m_eye, m_center, m_up;
		GLfloat m_fov, m_ratio, m_near, m_far;
		CCPoint m_position2d;
		ccVertex3F m_position3d, m_lookAt3d;

		bool m_dirty;

		friend class Frustum;
	};
}
#endif
