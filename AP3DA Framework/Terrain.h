#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "GameObject.h" // will inherit from Geomatry
#include <d3d11.h> // need to know about the Dx11 types for creating the buffers

#include <string>

class Terrain
{
public:
	Terrain(string type, Geometry geometry, Material material);
	~Terrain();


	
	virtual void Update(float t);
	virtual void Draw(ID3D11DeviceContext * pImmediateContext);

private:
	int m_rows;
	int m_columns;
	float m_cellWidth;
	float m_cellDepth;
	float m_width;
	float m_height;
	float m_minX;
	float m_maxX;
	float m_minZ;
	float m_maxZ;

	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT4X4 m_worldMatrix;


};



#endif 