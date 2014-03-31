#ifndef __NODE_3D_H__
#define __NODE_3D_H__
#include "cocos2d.h"

using namespace cocos2d;

namespace cocos3d
{
	class Node3D : public CCNode
	{
	public:
		Node3D();

		CREATE_FUNC(Node3D);
		
		virtual void setPosition(float x, float y);
		virtual void setPosition(const CCPoint &position);
		virtual const CCPoint& getPosition();
		virtual const ccVertex3F& get3DPosition();
		virtual void getPosition(float* x, float* y);
		virtual void  setPositionX(float x);
		virtual float getPositionX(void);
		virtual void  setPositionY(float y);
		virtual float getPositionY(void);
		virtual void setPosition(float x, float y, float z);
		virtual void setPosition(const ccVertex3F& position);
		virtual void setPositionZ(float z);
		virtual void setCameraDistance(float multiplier);
		virtual void setCameraTrackDelta(float trackX, float trackY);
		virtual void setCameraDollyDelta(float dollyX, float dollyY);
		virtual void setCameraTrackDelta(const CCPoint& track);
		virtual void setCameraDollyDelta(const CCPoint& dolly);
		virtual void setCameraTrackDeltaX(float trackX);
		virtual void setCameraDollyDeltaX(float dollyX);
		virtual void setCameraTrackDeltaY(float trackY);
		virtual void setCameraDollyDeltaY(float dollyY);

		virtual void visit();

		virtual void setYawnPitchRoll(const ccVertex3F& ypr);  
		virtual void setYawnPitchRoll(const float yaw, const float pitch, const float roll);
		virtual void setYaw(const float yaw);
		virtual void setPitch(const float pitch);
		virtual void setRoll(const float roll);

		virtual float getYaw(){ return m_yaw; }
		virtual float getRoll(){ return m_roll; }
		virtual float getPitch(){ return m_pitch; }

		virtual void draw();
		virtual void draw3D(){}

		
	protected:
		float m_cameraDistance;
		CCPoint m_position, m_tempPosition, m_track, m_dolly;
		ccVertex3F m_fullPosition, m_tempFullPosition;
		float m_yaw, m_pitch, m_roll, m_scale;
	};
}

#endif