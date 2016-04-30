#include "FlyingCamera.h"

// #include <Xinput.h>

#include <Windows.h>// for GetAsyncKeyState()


FlyingCamera::FlyingCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at, DirectX::XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
	: Camera(position, at, up, windowWidth, windowHeight, nearDepth, farDepth)
{
	m_defaultForward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_defaultRight = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_right = m_defaultRight;
	m_forward = m_defaultForward;

	m_upVecForFlyCam = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	m_yaw = 0.0f; // rotation in Y axis
	m_pitch = 0.0f; // rotation in 

	// m_lookAtDir = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_lookAtDir.x = at.x - position.x;
	m_lookAtDir.y = at.y - position.y;
	m_lookAtDir.z = at.z - position.z;


	DirectX::XMStoreFloat4x4(&m_rotationMat, DirectX::XMMatrixIdentity());
	m_moveSpeed = 1.1f;
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
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_moveSpeed += 0.1f;
	}
	else if (GetAsyncKeyState('E') & 0x8000)
	{
		m_moveSpeed -= 0.1f;
	}

	if (m_moveSpeed < 0.0f)
	{
		m_moveSpeed = 0.001f;
	}

	float moveSpeed = m_moveSpeed * dt;

	float turnSpeed = 5.0f;
	turnSpeed *= dt;
	// alter yaw & pitch with arrow keys (for now!!!)

	// Yaw for rotate around Y axis (turn L & R)

	float maxPitchDegrees = 80.0f;// 80 degrees

	float maxPitchRads = maxPitchDegrees * (3.142f / 180.0f);



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
		if (m_pitch < -maxPitchRads)
		{
			m_pitch = -maxPitchRads;
		}
	}
	else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		m_pitch += turnSpeed;
		if (m_pitch > maxPitchRads)
		{
			m_pitch = maxPitchRads;
		}
	}

	/* (old code)
	DirectX::XMVECTOR z = DirectX::XMLoadFloat3(&m_at);
	DirectX::XMVECTOR y = DirectX::XMLoadFloat3(&m_up);
	DirectX::XMVECTOR x = DirectX::XMVector3Cross(y,z); 

	z = DirectX::XMVector3Normalize(z);
	y = DirectX::XMVector3Normalize(y);
	x = DirectX::XMVector3Normalize(x);

	DirectX::XMFLOAT3 right;
	DirectX::XMStoreFloat3(&right, x);
	*/
	
	// based off http://www.braynzarsoft.net/viewtutorial/q16390-18-first-person-camera

	DirectX::XMMATRIX rotM = DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

	DirectX::XMVECTOR targetVec = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&m_defaultForward), rotM);
	targetVec = DirectX::XMVector3Normalize(targetVec);
	// rember the lookAt vec = eyePosition + targetVec
	DirectX::XMStoreFloat3(&m_lookAtDir, targetVec);

	DirectX::XMMATRIX tmpYRotMat = DirectX::XMMatrixRotationY(m_yaw);

	/* transform:
	DirectX::XMFLOAT3 m_right;
	DirectX::XMFLOAT3 m_forward;
	DirectX::XMFLOAT3 m_upVecForFlyCam;
	*/

	DirectX::XMVECTOR dr, r,
		df, f, upTmp, upRes;
	dr = DirectX::XMLoadFloat3(&m_defaultRight);
	df = DirectX::XMLoadFloat3(&m_defaultForward);
	upTmp = DirectX::XMLoadFloat3(&m_upVecForFlyCam);
	
	r = DirectX::XMVector3TransformNormal(dr, tmpYRotMat);
	f = DirectX::XMVector3TransformNormal(df, tmpYRotMat);
	upRes = DirectX::XMVector3TransformNormal(upTmp, tmpYRotMat);
	
	DirectX::XMStoreFloat3(&m_right, r);
	DirectX::XMStoreFloat3(&m_forward, f);
	DirectX::XMStoreFloat3(&m_upVecForFlyCam, upRes);


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
	DirectX::XMVECTOR rightScaled = DirectX::XMVectorScale(r, moveLeftRight);
	DirectX::XMVECTOR forwardScaled = DirectX::XMVectorScale(f, moveForwardBackward);
	DirectX::XMVECTOR upScaled = DirectX::XMVectorScale(upRes, moveUpDown);
	// for flying camera, also add the up Vector, for basic FPS camera don't
	

	DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_eye);
	pos = DirectX::XMVectorAdd(pos, rightScaled);
	pos = DirectX::XMVectorAdd(pos, forwardScaled);
	pos = DirectX::XMVectorAdd(pos, upScaled);
	DirectX::XMStoreFloat3(&m_eye, pos);

	// set the upVector
	DirectX::XMStoreFloat3(&m_up, upRes);

	// finally the at position, eyePos + target vec
	DirectX::XMVECTOR finalAt = DirectX::XMVectorAdd(pos, targetVec);
	DirectX::XMStoreFloat3(&m_at, finalAt);
	// DONE

	if (GetAsyncKeyState('R') & 0x8000) // R for Rest
	{
		m_at = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_eye = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);

	}


	// the view matix stuff is handled in Camera::update(), which isn't overriden

}