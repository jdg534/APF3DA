#include "FirstPersonCamera.h"

FirstPersonCamera::FirstPersonCamera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, Terrain * moveOn, float additionalHeight)
	:FlyingCamera(position, at, up, windowWidth, windowHeight, nearDepth, farDepth)

{
	m_additionalHeight = additionalHeight;
	m_moveOn = moveOn;
}

FirstPersonCamera::~FirstPersonCamera()
{
	// deleteing the Terrain pointer to be handled by the Terrain manager class
}

void FirstPersonCamera::updateLogic(float dt)
{
	FlyingCamera::updateLogic(dt); // applys the flying camera logic, now just alter the position and lookat 

	
	if (!m_moveOn->isPositionOnTerrain(m_eye.x, m_eye.z))
	{
		// moved off the height map, (just go back top the position from the last frame)
		m_eye = m_positionLastFrame;

		m_at.x = m_eye.x + m_lookAtDir.x;
		m_at.y = m_eye.y + m_lookAtDir.y;
		m_at.z = m_eye.z + m_lookAtDir.z;
	}
	else
	{
		// now just change the Y value to be what ever the value from the height map is + m_additionalHeight
		float heightOnTheHeightMap = m_moveOn->getHeightAtLocation(m_eye.x, m_eye.z);
		m_eye.y = heightOnTheHeightMap + m_additionalHeight;
		
		m_at.x = m_eye.x + m_lookAtDir.x;
		m_at.y = m_eye.y + m_lookAtDir.y;
		m_at.z = m_eye.z + m_lookAtDir.z;

		// update m_positionLastFrame
		m_positionLastFrame = m_eye;
	}

	
	if (GetAsyncKeyState(VK_PRIOR) & 0x8000) // VK_PRIOR = page up
	{
		m_additionalHeight += 0.01f;
	}
	if (GetAsyncKeyState(VK_NEXT) & 0x8000) // VK_NEXT = page down
	{
		m_additionalHeight -= 0.01f;
	}


}