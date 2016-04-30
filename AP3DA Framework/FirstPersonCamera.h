#ifndef _FIRST_PERSON_CAMERA_H_
#define _FIRST_PERSON_CAMERA_H_

#include "FlyingCamera.h"
#include "Terrain.h"

class FirstPersonCamera : public FlyingCamera
{
	// This camera is to behave like a flying Camera, but with the Y axis coord for the eyePos to be determined via height map value + m_additionalHeight

public:
	FirstPersonCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at, DirectX::XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, Terrain * moveOn, float additionalHeight);
	~FirstPersonCamera();

	virtual void updateLogic(float dt);

	float getAdditionalHeight(){ return m_additionalHeight; }
	Terrain * getMoveOn(){ return m_moveOn; }
	void setAdditionalHeight(float additionalHeightAboveTerrain){ m_additionalHeight = additionalHeightAboveTerrain; }
	void setMoveOn(Terrain * terrainToMoveOn){ m_moveOn = terrainToMoveOn; }
private:
	float m_additionalHeight; // to be on top of the value determined 
	Terrain * m_moveOn;
	DirectX::XMFLOAT3 m_positionLastFrame; // won't allow for the camera to move off the terrain
};


#endif