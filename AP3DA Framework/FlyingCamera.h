#ifndef _FLYING_CAMERA_H_
#define _FLYING_CAMERA_H_

#include "Camera.h"


class FlyingCamera : public Camera
{
public:
	FlyingCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at, DirectX::XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~FlyingCamera();
	virtual void updateLogic(float dt);
protected:
	/* Camera defined at centre of own coord system
	+ Z is the "look at" vector
	+ X is the "right" vector
	+ Y is the "up" vector

	*/

	/* required functionality
	1. "Running" Move along look vector
	2. "Strafing" move along right vector
	3. "Look up / down" rotate around right vector
	4. Looking left / right

	*/

	DirectX::XMFLOAT3 m_defaultForward;
	DirectX::XMFLOAT3 m_defaultRight;
	
	
	DirectX::XMFLOAT3 m_right;
	DirectX::XMFLOAT3 m_forward;
	DirectX::XMFLOAT3 m_upVecForFlyCam;

	DirectX::XMFLOAT3 m_lookAtDir; // for sub class they need to know the direction that the camera is looking at

	float m_moveSpeed;
	float m_turnSpeed;

	// lookat == target (external tutorial terminology)


	float m_yaw; // rotation in Y axis
	float m_pitch; // rotation in 

	DirectX::XMFLOAT4X4 m_rotationMat;

};


#endif