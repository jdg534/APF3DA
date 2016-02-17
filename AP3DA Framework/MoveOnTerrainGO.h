#ifndef _MOVE_ON_HEIGHT_MAP_GO_H_
#define _MOVE_ON_HEIGHT_MAP_GO_H_

#include "GameObject.h"
#include "Terrain.h"

class MoveOnTerrainGameObject : public GameObject
{
	
public:
	MoveOnTerrainGameObject(string type, Geometry geometry, Material material);

	void Update(float t);
	

	Terrain * getMoveOn(){ return m_moveOn; }
	void setMoveOn(Terrain * moveOn){ m_moveOn = moveOn; }
private:
	Terrain * m_moveOn;

	DirectX::XMFLOAT3 m_pos, m_posLastFrame, m_heading;
	float m_moveSpeed;

	void changeDirection();
};


#endif