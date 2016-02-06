#include "FlyingCamera.h"

// #include <Xinput.h>

#include <Windows.h>// for GetAsyncKeyState()


FlyingCamera::FlyingCamera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
	: Camera(position, at, up, windowWidth, windowHeight, nearDepth, farDepth)
{
	m_defaultForward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_defaultRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_right = m_defaultRight;
	m_forward = m_defaultForward;

	m_upVecForFlyCam = XMFLOAT3(0.0f, 1.0f, 0.0f);

	m_yaw = 0.0f; // rotation in Y axis
	m_pitch = 0.0f; // rotation in 

	XMStoreFloat4x4(&m_rotationMat, XMMatrixIdentity());
}

FlyingCamera::~FlyingCamera()
{

}

void FlyingCamera::updateLogic(float dt)
{
	// http://www.braynzarsoft.net/viewtutorial/q16390-18-first-person-camera
	// was useful start


	/*
	+Z is ‘look at’ vector
	+X is the ‘right’ vector
	+Y is the ‘up’ vector 
	*/

	float moveSpeed = 0.05f;

	moveSpeed *= dt;

	float turnSpeed = 0.01f;
	turnSpeed *= dt;
	// alter yaw & pitch with arrow keys (for now!!!)

	// Yaw for rotate around Y axis (turn L & R)

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		m_yaw -= turnSpeed;
	}
	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		m_yaw += turnSpeed;
	}

	// pitch for x axis (looking up & down)
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		m_pitch -= turnSpeed;
	}
	else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		m_pitch += turnSpeed;
	}

	/* (old code)
	XMVECTOR z = XMLoadFloat3(&m_at);
	XMVECTOR y = XMLoadFloat3(&m_up);
	XMVECTOR x = XMVector3Cross(y,z); 

	z = XMVector3Normalize(z);
	y = XMVector3Normalize(y);
	x = XMVector3Normalize(x);

	XMFLOAT3 right;
	XMStoreFloat3(&right, x);
	*/
	
	// based off http://www.braynzarsoft.net/viewtutorial/q16390-18-first-person-camera

	XMMATRIX rotM = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

	XMVECTOR targetVec = XMVector3TransformCoord(XMLoadFloat3(&m_defaultForward), rotM);
	targetVec = XMVector3Normalize(targetVec);
	// rember the lookAt vec = eyePosition + targetVec
	

	XMMATRIX tmpYRotMat = XMMatrixRotationY(m_yaw);

	/* transform:
	XMFLOAT3 m_right;
	XMFLOAT3 m_forward;
	XMFLOAT3 m_upVecForFlyCam;
	*/

	XMVECTOR dr, r,
		df, f, upTmp, upRes;
	dr = XMLoadFloat3(&m_defaultRight);
	df = XMLoadFloat3(&m_defaultForward);
	upTmp = XMLoadFloat3(&m_upVecForFlyCam);
	
	r = XMVector3TransformNormal(dr, tmpYRotMat);
	f = XMVector3TransformNormal(df, tmpYRotMat);
	upRes = XMVector3TransformNormal(upTmp, tmpYRotMat);
	
	XMStoreFloat3(&m_right, r);
	XMStoreFloat3(&m_forward, f);
	XMStoreFloat3(&m_upVecForFlyCam, upRes);


	float moveLeftRight = 0.0f;
	float moveForwardBackward = 0.0f;

	// use input determine the values
	// A D for left & right
	// W S for forward and back

	// GetAsyncKeyState()

	// 0x8000 for key currently down
	// 0x0001 for just been pressed

	if (GetAsyncKeyState('A') & 0x8000)
	{
		moveLeftRight -= moveSpeed;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		moveLeftRight += moveSpeed;
	}

	if (GetAsyncKeyState('W') & 0x8000)
	{
		moveForwardBackward += moveSpeed;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		moveForwardBackward -= moveSpeed;
	}

	// for up & down (use pageUp & pageDown)
	float moveUpDown = 0.0f;
	if (GetAsyncKeyState(VK_PRIOR) & 0x8000) // VK_PRIOR = page up
	{
		moveUpDown += moveSpeed;
	}
	if (GetAsyncKeyState(VK_NEXT) & 0x8000) // VK_NEXT = page down
	{
		moveUpDown -= moveSpeed;
	}



	// just need to set the final values for m_eye, m_at & m_up
	XMVECTOR rightScaled = XMVectorScale(r, moveLeftRight);
	XMVECTOR forwardScaled = XMVectorScale(f, moveForwardBackward);
	XMVECTOR upScaled = XMVectorScale(upRes, moveUpDown);
	// for flying camera, also add the up Vector, for basic FPS camera don't
	

	XMVECTOR pos = XMLoadFloat3(&m_eye);
	pos = XMVectorAdd(pos, rightScaled);
	pos = XMVectorAdd(pos, forwardScaled);
	pos = XMVectorAdd(pos, upScaled);
	XMStoreFloat3(&m_eye, pos);

	// set the upVector
	XMStoreFloat3(&m_up, upRes);

	// finally the at position, eyePos + target vec
	XMVECTOR finalAt = XMVectorAdd(pos, targetVec);
	XMStoreFloat3(&m_at, finalAt);
	// DONE

	if (GetAsyncKeyState('R') & 0x8000) // R for Rest
	{
		m_at = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_eye = XMFLOAT3(0.0f, 0.0f, -5.0f);

	}


	// the view matix stuff is handled in Camera::update(), which isn't overriden

}