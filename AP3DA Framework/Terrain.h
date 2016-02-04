#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "GameObject.h" 
#include "HeightMap.h"
#include "Structures.h"

#include <d3d11.h> // need to know about the Dx11 types for creating the buffers

#include <string>



class Terrain
{
public:
	Terrain();
	~Terrain();

	virtual bool initAsFlatTerrain(int mRows, int nColumns, float cellWidth, float cellDepth, ID3D11Device * devicePtr); // just the func from Application.cpp
	virtual bool initViaHeightMap(HeightMap * hm, float scaleHeightBy, ID3D11Device * devicePtr);


	virtual void Update(float t); // probably not nessary
	virtual void Draw(ID3D11DeviceContext * pImmediateContext);

	
	void setPosition(float x, float y, float z)
	{
		m_position.x = x;
		m_position.y = y;
		m_position.z = z;
	}

	DirectX::XMMATRIX getWorldMat(){ return DirectX::XMLoadFloat4x4(&m_worldMatrix); }

protected:

	float calculateTextureCoord(float minPos, float maxPos, float pos);

	int m_rows;
	int m_columns;
	
	int m_cellRows;
	int m_cellColumns;

	float m_cellWidth;
	float m_cellDepth;
	float m_width;
	float m_depth;
	float m_minX;
	float m_maxX;
	float m_minZ;
	float m_maxZ;

	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT4X4 m_worldMatrix;

	HeightMap * m_heightMap;

	Geometry m_geometry;

	void correctVertexNormals(std::vector<SimpleVertex> & toCorrect, std::vector<WORD> & indices);
	std::vector<Facet> getConnectedFacets(XMFLOAT3 vertex, std::vector<Facet> & facets);
	void removeDuplicateNormals(std::vector<DirectX::XMFLOAT3> & n);
};



#endif 