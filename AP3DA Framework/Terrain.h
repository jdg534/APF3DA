#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "GameObject.h" 
#include "HeightMap.h"

#include <d3d11.h> // need to know about the Dx11 types for creating the buffers

#include <string>



class Terrain
{
public:
	Terrain();
	~Terrain();

	virtual bool initAsFlatTerrain(int mRows, int nColumns, float cellWidth, float cellDepth, ID3D11Device * devicePtr); // just the func from Application.cpp
	
	virtual void Update(float t); // probably not nessary
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

	HeightMap * m_heightMap;

	Geometry m_geometry;
};



#endif 