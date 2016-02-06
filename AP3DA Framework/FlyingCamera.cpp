#include "FlyingCamera.h"

#include <Xinput.h>

FlyingCamera::FlyingCamera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
	: Camera(position, at, up, windowWidth, windowHeight, nearDepth, farDepth)
{

}

FlyingCamera::~FlyingCamera()
{

}

void FlyingCamera::updateLogic(float dt)
{
	/*
	+Z is ‘look at’ vector
	+X is the ‘right’ vector
	+Y is the ‘up’ vector 
	*/
	XMVECTOR z = XMLoadFloat3(&m_at);
	XMVECTOR y = XMLoadFloat3(&m_up);
	XMVECTOR x = XMVector3Cross(y,z); 

	z = XMVector3Normalize(z);
	y = XMVector3Normalize(y);
	x = XMVector3Normalize(x);

	XMFLOAT3 right;
	XMStoreFloat3(&right, x);

	float moveSpeed = 0.01f;// 
}