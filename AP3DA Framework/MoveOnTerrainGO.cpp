#include "MoveOnTerrainGO.h"


MoveOnTerrainGameObject::MoveOnTerrainGameObject(string type, Geometry geometry, Material material)
	: GameObject(type, geometry, material)
{
	m_moveOn = nullptr;

	m_pos = DirectX::XMFLOAT3(0.0f,0.0f,0.0f), m_posLastFrame = m_pos, m_heading = DirectX::XMFLOAT3(0.0,0.0,1.0);
	m_moveSpeed = 0.001f;

}

void MoveOnTerrainGameObject::Update(float t)
{
	if (!m_moveOn->isPositionOnTerrain(m_pos.x, m_pos.z))
	{
		m_pos = m_posLastFrame;
		changeDirection();
	}
	else
	{
		XMVECTOR heading = XMVector3Normalize(XMLoadFloat3(&m_heading));
		heading = XMVectorScale(heading, m_moveSpeed * t);
		XMVECTOR pos = XMLoadFloat3(&m_pos);
		m_posLastFrame = m_pos;
		pos = XMVectorAdd(pos, heading);
		XMStoreFloat3(&m_pos, pos);
		m_pos.y = m_moveOn->getHeightAtLocation(m_pos.x, m_pos.z);
		
	}

	_position = m_pos;

	GameObject::Update(t);
}

void MoveOnTerrainGameObject::changeDirection()
{
	// the pattern: forward, right, toward, away
	if (m_heading.z > 0.0f)
	{
		// was going forward
		m_heading = XMFLOAT3(1.0f, 0.0f, 0.0f); // go right
	}
	else if (m_heading.x > 0.0f)
	{
		// was going right
		m_heading = XMFLOAT3(0.0f, 0.0f, -1.0f); // go toward
	}
	else if (m_heading.z < 0.0f)
	{
		 // was going toward
		m_heading = XMFLOAT3(-1.0f, 0.0f, 0.0f); // go left
	}
	else if (m_heading.x < 0.0f)
	{
		// was going left
		m_heading = XMFLOAT3(0.0f, 0.0f, 1.0f); // go away
	}
	else
	{
		// (unexpected behavior)
		m_heading = XMFLOAT3(0.0f, 0.0f, 1.0f); // go away
	}
}