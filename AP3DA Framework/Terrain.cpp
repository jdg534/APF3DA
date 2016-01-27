#include "Terrain.h"

#include <fstream>

Terrain::Terrain()
{
	m_rows = 0;
	m_columns = 0;
	m_cellWidth = 0;
	m_cellDepth = 0;
	m_width = 0.0f;
	m_height = 0.0f;
	m_minX = 0.0f;
	m_maxX = 0.0f;
	m_minZ = 0.0f;
	m_maxZ = 0.0f;

	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_position.z = 0.0f;

	DirectX::XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixIdentity());
}

Terrain::~Terrain()
{

}

bool Terrain::generateFlatGrid(ID3D11Device * d3dDPtr, int mRows, int nColumns, float cellWidth, float cellDepth)
{

	return false;
}

bool Terrain::generateFromHeightMapRawFile(ID3D11Device * d3dDPtr, std::string heightMapFileName, float cellWidth, float cellDepth)
{
	std::ifstream heightMap(heightMapFileName, std::ios::binary);

	if (!heightMap.good())
	{
		return false;
	}

	// can read from the file



	return false;
}

void Terrain::Update(float t)
{

}

void Terrain::Draw(ID3D11DeviceContext * pImmediateContext)
{

}