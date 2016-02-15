#include "Terrain.h"

#include "Structures.h"

#include "MathFuncs.h"

#include <fstream>



Terrain::Terrain()
{
	m_rows = 0;
	m_columns = 0;
	m_cellWidth = 0;
	m_cellDepth = 0;
	m_width = 0.0f;
	m_depth = 0.0f;
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

bool Terrain::initAsFlatTerrain(int mRows, int nColumns, float cellWidth, float cellDepth, ID3D11Device * devicePtr)
{
	m_rows = mRows;
	m_columns = nColumns;
	m_cellRows = mRows - 1;
	m_cellColumns = nColumns - 1;
	m_cellWidth = cellWidth;
	m_cellDepth = cellDepth;
	m_width = m_rows * m_cellWidth;
	m_depth = m_columns * m_cellDepth;

	m_scaleHeightBy = 1.0f;

	int nCells = m_cellRows * m_cellColumns;
	int nTriangles = nCells * 2;
	int nVerts = mRows * nColumns;

	XMFLOAT2 t;
	t.x = -(m_width / 2.0f);
	t.y = m_depth / 2.0f;

	int k = 0;

	std::vector<XMFLOAT3> verts;
	verts.resize(nVerts);

	for (int i = 0; i < mRows; i++)
	{
		for (int j = 0; j < nColumns; j++)
		{
			verts[k].x = j * cellWidth + (-m_width * 0.5f);
			verts[k].y = 0.0f;
			verts[k].z = -(i * cellDepth) + (m_depth * 0.5f);
			k++;
		}
	}

	std::vector<WORD> indices;

	// for the ijth Quad


	// code based off slide 9 of first lecture

	for (int i = 0; i < mRows - 1; i++)
	{
		for (int j = 0; j < nColumns - 1; j++)
		{
			XMINT3 abc;
			abc.x = i * mRows + j;
			abc.y = i * mRows + j + 1;
			abc.z = (i + 1) * mRows + j;

			XMINT3 cbd;
			cbd.x = (i + 1) * mRows + j;
			cbd.y = i * mRows + j + 1;
			cbd.z = (i + 1) * mRows + j + 1;

			// add to the vector as WORD use static_cast<WORD>()
			indices.push_back(static_cast<WORD>(abc.x));
			indices.push_back(static_cast<WORD>(abc.y));
			indices.push_back(static_cast<WORD>(abc.z));

			indices.push_back(static_cast<WORD>(cbd.x));
			indices.push_back(static_cast<WORD>(cbd.y));
			indices.push_back(static_cast<WORD>(cbd.z));
		}
	}

	/*
	0,0---1,0
	|		|
	0,1---1,1
	*/

	XMFLOAT2 topLeft(99999.99f, 99999.99f), bottomRight(-topLeft.x, -topLeft.y);

	for (int i = 0; i < verts.size(); i++)
	{
		if (topLeft.x > verts[i].x)
		{
			topLeft.x = verts[i].x;
		}
		if (topLeft.y > verts[i].z)
		{
			topLeft.y = verts[i].z;
		}

		if (bottomRight.x < verts[i].x)
		{
			bottomRight.x = verts[i].x;
		}
		if (bottomRight.y < verts[i].z)
		{
			bottomRight.y = verts[i].z;
		}
	}

	std::vector<SimpleVertex> vertsToSendToD3dBuffer;
	for (int i = 0; i < verts.size(); i++)
	{
		SimpleVertex sv;
		sv.PosL.x = verts[i].x;
		sv.PosL.y = verts[i].y;
		sv.PosL.z = verts[i].z;

		sv.NormL.x = 0.0f;
		sv.NormL.y = 1.0f;
		sv.NormL.z = 0.0f;

		/*
		float scaleXBy = (1.0f / width);
		float scaleYBy = (1.0f / depth); // technically scale Z

		sv.Tex.x = sv.PosL.x * scaleXBy;
		sv.Tex.y = sv.PosL.z * scaleYBy;

		// tec coords now in range -1.0 to 1.0

		// need 0.0 to 1.0?
		sv.Tex.x = (sv.Tex.x + 1.0f) / 2.0f;
		sv.Tex.y = (sv.Tex.y + 1.0f) / 2.0f;
		*/

		sv.Tex.x = calculateTextureCoord(topLeft.x, bottomRight.x, sv.PosL.x);
		sv.Tex.y = calculateTextureCoord(topLeft.y, bottomRight.y, sv.PosL.z);

		vertsToSendToD3dBuffer.push_back(sv);
	}


	// now move to vertex & index buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * vertsToSendToD3dBuffer.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &vertsToSendToD3dBuffer[0];


	// Geometry * rv = new Geometry();

	// m_geometry instead of: Geometry * rv = new Geometry();

	HRESULT hr = devicePtr->CreateBuffer(&bd, &InitData, &m_geometry.vertexBuffer);
	m_geometry.vertexBufferStride = sizeof(SimpleVertex);
	m_geometry.vertexBufferOffset = 0; // would be somethign else, if got to start with verts that don't stat at beginning of the buffer

	if (FAILED(hr))
	{
		return false;
	}


	// now create the index buffer, and set the number of indecies used by the buffer

	D3D11_BUFFER_DESC indBufdesc;
	ZeroMemory(&indBufdesc, sizeof(indBufdesc));

	indBufdesc.Usage = D3D11_USAGE_DEFAULT;
	indBufdesc.ByteWidth = sizeof(WORD) * indices.size();
	indBufdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indBufdesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indBufInitData;
	ZeroMemory(&indBufInitData, sizeof(indBufInitData));
	indBufInitData.pSysMem = &indices[0];
	hr = devicePtr->CreateBuffer(&indBufdesc, &indBufInitData, &m_geometry.indexBuffer);

	if (FAILED(hr))
	{
		return false;
	}

	m_geometry.numberOfIndices = indices.size();

	return true;
}

bool Terrain::initViaHeightMap(HeightMap * hm, float scaleHeightBy, ID3D11Device * devicePtr, float width, float depth)
{
	/* (original calculations from initAsFlatTerrain())
	m_rows = mRows;
	m_columns = nColumns;
	m_cellRows = mRows - 1;
	m_cellColumns = nColumns - 1;
	m_cellWidth = cellWidth;
	m_cellDepth = cellDepth;
	m_width = m_rows * m_cellWidth;
	m_depth = m_columns * m_cellDepth;
	*/

	m_rows = hm->getWidth();
	m_columns = hm->getDepth();
	m_cellRows = m_rows - 1;
	m_cellColumns = m_columns - 1;

	m_scaleHeightBy = scaleHeightBy;

	/* before width & depth as a parameter
	m_cellWidth = 1.0f; // should have as a parameter
	m_cellDepth = 1.0f;
	
	m_width = m_rows * m_cellWidth;
	m_depth = m_columns * m_cellDepth;
	*/
	m_width = width;
	m_depth = depth;

	m_cellWidth = m_width / (float) m_rows;
	m_cellDepth = m_depth / (float)m_columns;

	int nCells = m_cellRows * m_cellColumns;
	int nTriangles = nCells * 2;
	int nVerts = m_rows * m_columns;

	XMFLOAT2 t;
	t.x = -(m_width / 2.0f);
	t.y = m_depth / 2.0f;

	int k = 0;

	std::vector<VertexForVertexNormalCalc> verts;
	verts.resize(nVerts);

	for (int i = 0; i < m_rows; i++)
	{
		for (int j = 0; j < m_columns; j++)
		{
			verts[k].pos.x = j * m_cellWidth + (-m_width * 0.5f);
			// verts[k].y = 0.0f; original

			unsigned char hmVal = hm->getHeightAt(i, j);

			// hmVal is 0 - 255, scale to 0.0 - 1.0
			float downScale = 1.0f / 255.0f;
			float downScaled = static_cast<float>(hmVal)* downScale;

			// rescale to the scale use requested

			verts[k].pos.y = downScaled * scaleHeightBy;

			verts[k].pos.z = -(i * m_cellDepth) + (m_depth * 0.5f);
			k++;
		}
	}

	std::vector<WORD> indices;

	// for the ijth Quad


	// code partly based off slide 9 of first lecture

	for (int i = 0; i < m_rows - 1; i++)
	{
		for (int j = 0; j < m_columns - 1; j++)
		{
			// treat each tri as a facet
			
			XMINT3 abc;
			abc.x = i * m_rows + j;
			abc.y = i * m_rows + j + 1;
			abc.z = (i + 1) * m_rows + j;

			XMFLOAT3 abcSn = calcSurfaceNormal(XMLoadFloat3(&verts[abc.x].pos), 
				XMLoadFloat3(&verts[abc.y].pos),
				XMLoadFloat3(&verts[abc.z].pos));

			
						

			XMINT3 cbd;
			cbd.x = (i + 1) * m_rows + j;
			cbd.y = i * m_rows + j + 1;
			cbd.z = (i + 1) * m_rows + j + 1;

			XMFLOAT3 cbdSn = calcSurfaceNormal(XMLoadFloat3(&verts[cbd.x].pos),
				XMLoadFloat3(&verts[cbd.y].pos),
				XMLoadFloat3(&verts[cbd.z].pos));

			verts[abc.x].surfaceNormals.push_back(abcSn);
			verts[abc.y].surfaceNormals.push_back(abcSn);
			verts[abc.z].surfaceNormals.push_back(abcSn);

			verts[cbd.x].surfaceNormals.push_back(cbdSn);
			verts[cbd.y].surfaceNormals.push_back(cbdSn);
			verts[cbd.z].surfaceNormals.push_back(cbdSn);

			// add to the vector as WORD use static_cast<WORD>()
			indices.push_back(static_cast<WORD>(abc.x));
			indices.push_back(static_cast<WORD>(abc.y));
			indices.push_back(static_cast<WORD>(abc.z));

			indices.push_back(static_cast<WORD>(cbd.x));
			indices.push_back(static_cast<WORD>(cbd.y));
			indices.push_back(static_cast<WORD>(cbd.z));
		}
	}



	/*
	0,0---1,0
	|		|
	0,1---1,1
	*/

	XMFLOAT2 topLeft(99999.99f, 99999.99f), bottomRight(-topLeft.x, -topLeft.y);

	for (int i = 0; i < verts.size(); i++)
	{
		if (topLeft.x > verts[i].pos.x)
		{
			topLeft.x = verts[i].pos.x;
		}
		if (topLeft.y > verts[i].pos.z)
		{
			topLeft.y = verts[i].pos.z;
		}

		if (bottomRight.x < verts[i].pos.x)
		{
			bottomRight.x = verts[i].pos.x;
		}
		if (bottomRight.y < verts[i].pos.z)
		{
			bottomRight.y = verts[i].pos.z;
		}
	}

	// set the m_ versions topLeft & bottomRight
	m_topLeftPoint = topLeft;
	m_bottomRightPoint = bottomRight;

	std::vector<SimpleVertex> vertsToSendToD3dBuffer;
	for (int i = 0; i < verts.size(); i++)
	{
		SimpleVertex sv;
		sv.PosL.x = verts[i].pos.x;
		sv.PosL.y = verts[i].pos.y;
		sv.PosL.z = verts[i].pos.z;



		// this has to be changed
		// will just have a normals correction func, calc surface normal, then 
		
		XMFLOAT3 VertexNormal(0.0, 0.0, 0.0);
		for (int j = 0; j < verts[i].surfaceNormals.size(); j++)
		{
			VertexNormal.x += verts[i].surfaceNormals[j].x;
			VertexNormal.y += verts[i].surfaceNormals[j].y;
			VertexNormal.z += verts[i].surfaceNormals[j].z;
		}
		float downScaleVNBy = 1.0f / (float)verts[i].surfaceNormals.size();
		sv.NormL.x = VertexNormal.x * downScaleVNBy;
		sv.NormL.y = VertexNormal.y * downScaleVNBy;
		sv.NormL.z = VertexNormal.z * downScaleVNBy;

		/*
		float scaleXBy = (1.0f / width);
		float scaleYBy = (1.0f / depth); // technically scale Z

		sv.Tex.x = sv.PosL.x * scaleXBy;
		sv.Tex.y = sv.PosL.z * scaleYBy;

		// tec coords now in range -1.0 to 1.0

		// need 0.0 to 1.0?
		sv.Tex.x = (sv.Tex.x + 1.0f) / 2.0f;
		sv.Tex.y = (sv.Tex.y + 1.0f) / 2.0f;
		*/

		sv.Tex.x = calculateTextureCoord(topLeft.x, bottomRight.x, sv.PosL.x);
		sv.Tex.y = calculateTextureCoord(topLeft.y, bottomRight.y, sv.PosL.z);

		vertsToSendToD3dBuffer.push_back(sv);
	}

	m_heightMap = hm;

	// correctVertexNormals(vertsToSendToD3dBuffer, indices);
	// altCorrectVertexNormals(vertsToSendToD3dBuffer, indices);

	// both funcs too slow

	// handle sending the data to actual d3d buffers


	// now move to vertex & index buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * vertsToSendToD3dBuffer.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &vertsToSendToD3dBuffer[0];


	// Geometry * rv = new Geometry();

	// m_geometry instead of: Geometry * rv = new Geometry();

	HRESULT hr = devicePtr->CreateBuffer(&bd, &InitData, &m_geometry.vertexBuffer);
	m_geometry.vertexBufferStride = sizeof(SimpleVertex);
	m_geometry.vertexBufferOffset = 0; // would be somethign else, if got to start with verts that don't stat at beginning of the buffer

	if (FAILED(hr))
	{
		return false;
	}


	// now create the index buffer, and set the number of indecies used by the buffer

	D3D11_BUFFER_DESC indBufdesc;
	ZeroMemory(&indBufdesc, sizeof(indBufdesc));

	indBufdesc.Usage = D3D11_USAGE_DEFAULT;
	indBufdesc.ByteWidth = sizeof(WORD) * indices.size();
	indBufdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indBufdesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indBufInitData;
	ZeroMemory(&indBufInitData, sizeof(indBufInitData));
	indBufInitData.pSysMem = &indices[0];
	hr = devicePtr->CreateBuffer(&indBufdesc, &indBufInitData, &m_geometry.indexBuffer);

	if (FAILED(hr))
	{
		return false;
	}

	m_geometry.numberOfIndices = indices.size();

	return true;
}

void Terrain::Update(float t)
{
	// just update the world matrix to be a translation matrix

	DirectX::XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z));
}

void Terrain::Draw(ID3D11DeviceContext * pImmediateContext)
{
	// just to logic on the game object class
	// Set vertex and index buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &m_geometry.vertexBuffer, &m_geometry.vertexBufferStride, &m_geometry.vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(m_geometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(m_geometry.numberOfIndices, 0, 0);
}

bool Terrain::isPositionOnTerrain(float x, float z)
{
	// first 
	// like AABB

	// height map position += m_position

	XMFLOAT2 actualTopLeftCoord;
	XMFLOAT2 actualBottomRight;
	actualTopLeftCoord.x = m_position.x + m_topLeftPoint.x;
	actualTopLeftCoord.y = m_position.z + m_topLeftPoint.y; // although its denoted as z the value is actually for the z axis

	actualBottomRight.x = m_position.x + m_bottomRightPoint.x;
	actualBottomRight.y = m_position.z + m_bottomRightPoint.y;

	// bottom right is actually far distance
	// top left actually the near distance
	float LowZ = actualTopLeftCoord.y;
	float highZ = actualBottomRight.y;
	

	// left handed coord sys
	if (x < actualTopLeftCoord.x)
	{
		return false;
	}
	else if (x > actualBottomRight.x)
	{
		return false;
	}
	else if (z < LowZ)
	{
		return false;
	}
	else if (z > highZ)
	{
		return false;
	}
	return true;
}

float Terrain::getHeightAtLocation(float x, float z)
{
	// after checking isPositionOnTerrain()
	// may need to biLerp

	// if this is called will asume that the point is on the terrain

	XMFLOAT2 actualTopLeftCoord;
	XMFLOAT2 actualBottomRight;
	actualTopLeftCoord.x = m_position.x + m_topLeftPoint.x;
	actualTopLeftCoord.y = m_position.z + m_topLeftPoint.y; // although its denoted as z the value is actually for the z axis

	actualBottomRight.x = m_position.x + m_bottomRightPoint.x;
	actualBottomRight.y = m_position.z + m_bottomRightPoint.y;

	/*

	int xStepsSoFar = 0;
	float xStartingPoint = actualTopLeftCoord.x;
	float xTraceStepLoc = xStartingPoint;
	bool keepGoingRight = true;
	
	while (keepGoingRight)
	{
		if (xTraceStepLoc >= x)
		{
			// got right enough to hit 
			keepGoingRight = false;
		}
		xStepsSoFar++;
		xTraceStepLoc = (float) xStepsSoFar * m_cellWidth;
	}

	// xStepsSoFar stores the index for the current cells height map value
	// need to calc the over shoot from previous cell
	float xOverShoot
	*/

	// calc dist from far right
	float xDistFromRight = actualBottomRight.x - x;

	float zDistFromBottom = actualBottomRight.y - z;

	// calculate nCells covered in X

	// calculate nCells covered in Z
	float xCellsCovered = xDistFromRight / m_cellWidth;
	float zCellsCovered = zDistFromBottom / m_cellDepth;

	unsigned int xCellsCoveredUIntForm = (unsigned int)xCellsCovered;
	unsigned int zCellsCoveredUIntForm = (unsigned int)zCellsCovered;

	/* (Old in-accurate code)
	float xBiLerpTxVal = xCellsCovered - (float)xCellsCoveredUIntForm;
	float zBiLerpTyVal = zCellsCovered - (float)zCellsCoveredUIntForm;

	// now get the cell heights
	unsigned char _00UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm, zCellsCoveredUIntForm), _10UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm + 1, zCellsCoveredUIntForm),
		_01UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm, zCellsCoveredUIntForm + 1), _11UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm + 1,zCellsCoveredUIntForm + 1 );

	// down scale from 0 - 255 to 0.0 - 1.0
	float _00F = (float)_00UC * (1.0f / 225.0f), _10F = (float)_10UC * (1.0f / 255.0f),
		_01F = (float)_01UC * (1.0f / 255.0f), _11F = (float)_11UC * (1.0f / 255.0f);

	// rescale based on m_scaleHeightBy
	_00F * m_scaleHeightBy;
	_10F * m_scaleHeightBy;
	_01F * m_scaleHeightBy;
	_11F * m_scaleHeightBy;

	float rv = MathFuncs::biLerp(_00F, _10F,
		_01F, _11F, xBiLerpTxVal, zBiLerpTyVal);

	return rv;

	*/

	unsigned char heightTL_UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm, zCellsCoveredUIntForm), 
		heightTR_UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm + 1, zCellsCoveredUIntForm),
		heightBL_UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm, zCellsCoveredUIntForm + 1), 
		heightBR_UC = m_heightMap->getHeightAt(xCellsCoveredUIntForm + 1, zCellsCoveredUIntForm + 1);

	

	float downScale = 1.0f / 255.0f;

	float heightTL_F = (float)heightTL_UC * downScale;
	float heightTR_F = (float)heightTR_UC * downScale;
	float heightBL_F = (float)heightBL_UC * downScale;
	float heightBR_F = (float)heightBR_UC * downScale;

	heightTL_F *= m_scaleHeightBy;
	heightTR_F *= m_scaleHeightBy;
	heightBL_F *= m_scaleHeightBy;
	heightBR_F *= m_scaleHeightBy;

	// is rescaled by m_scaleHeightBy



	// get the Heights for Triangles ABC & DCB
	Triangle abc, dcb;
	abc.v0.x = m_cellWidth * (float)xCellsCoveredUIntForm;
	abc.v0.y = heightTL_F;
	abc.v0.z = m_cellDepth * (float)zCellsCoveredUIntForm;
	
	abc.v1.x = m_cellWidth * (float)(xCellsCoveredUIntForm + 1);
	abc.v1.y = heightTR_F;
	abc.v1.z = m_cellDepth * (float)zCellsCoveredUIntForm;

	abc.v2.x = m_cellWidth * (float)xCellsCoveredUIntForm;
	abc.v2.y = heightBL_F;
	abc.v2.z = m_cellDepth * (float)(zCellsCoveredUIntForm + 1);

	// see the characters on which vertex on D.C.B maps to vetex on A.B.C
	dcb.v0.x = m_cellWidth * (float)(xCellsCoveredUIntForm + 1);
	dcb.v0.y = heightBR_F;
	dcb.v0.z = m_cellDepth * (float)(zCellsCoveredUIntForm + 1);

	dcb.v1 = abc.v2;
	dcb.v2 = abc.v1;

	// return -1.0f;// WIP
	
	// new code, based off WK1 L1 Slides 19 to 24, the above ABC & DCB triangles may be used

	float c = (x + 0.5f * m_width) / m_cellWidth; 
	float d = (z - 0.5f * m_depth) / -m_cellDepth;

	float row = floorf(d);
	float column = floorf(c);

	float s = c - column;
	float t = d - row;

	float sAndT = s + t; // if it's bigger than 2.0 some thing is wrong

	float stepSize = 0.02f; // may need to be bigger for speed (bigger steps = less steps = faster)

	

	if (s+t <= 1.0f) 
	{
		// do stuff in ABC

		XMFLOAT3 u, v;
		u.x = 0.0f;
		u.y = abc.v1.y - abc.v0.y;
		u.z = 0.0f;
		
		v.x = 0.0f;
		v.y = abc.v2.y - abc.v0.y;
		v.z = v.x;

		float rv = abc.v0.y + (s * u.y) + (t * v.y);
		return rv;
	}
	else
	{
		// do stuff in DCB
		XMFLOAT3 u, v;
		u.x = 0.0f;
		u.y = dcb.v1.y - dcb.v0.y;
		u.z = 0.0f;

		v.x = 0.0f;
		v.y = dcb.v2.y - dcb.v0.y;
		v.z = v.x;

		float rv = dcb.v0.y + ((1.0f - s) * u.y) + ((1.0f - t) * v.y);
		return rv;
	}

	// return biLerp of the cells
	
	return -1.0f;
}

float Terrain::calculateTextureCoord(float minPos, float maxPos, float pos)
{
	float stepSize = 0.01f;
	float currentStep = 0.0f;

	while (currentStep < 1.0f)
	{
		// float stepPos = Math::lerp(minPos, maxPos, currentStep);

		float stepPos = (1.0f - currentStep) * minPos + maxPos * currentStep;
		if (stepPos >= pos)
		{
			return currentStep;
		}
		currentStep += stepSize;
	}
	return 1.0f;
}

void Terrain::correctVertexNormals(std::vector<SimpleVertex> & toCorrect, std::vector<WORD> & indices)
{
	std::vector<Facet> f;
	// use the index buffer to determine the correct facets
	for (int i = 0; i < indices.size(); i += 3)
	{
		Facet tmpf;
		tmpf.v0 = toCorrect[indices[i]].PosL;
		tmpf.v1 = toCorrect[indices[i + 1]].PosL;
		tmpf.v2 = toCorrect[indices[i + 2]].PosL;
		// calculate the surface normal
		DirectX::XMFLOAT3 v0ToV1, v0ToV2;
		v0ToV1.x = tmpf.v1.x - tmpf.v0.x;
		v0ToV1.y = tmpf.v1.y - tmpf.v0.y;
		v0ToV1.z = tmpf.v1.z - tmpf.v0.z;

		v0ToV2.x = tmpf.v2.x - tmpf.v0.x;
		v0ToV2.y = tmpf.v2.y - tmpf.v0.y;
		v0ToV2.z = tmpf.v2.z - tmpf.v0.z;

		DirectX::XMVECTOR a, b;
		a = DirectX::XMLoadFloat3(&v0ToV1);
		b = DirectX::XMLoadFloat3(&v0ToV2);

		DirectX::XMVECTOR aCrossB = DirectX::XMVector3Cross(a, b);

		XMStoreFloat3(&tmpf.surfaceNormal, XMVector3Normalize(aCrossB));

		// add tmpF to f
		f.push_back(tmpf);
	}

	// make the corrections 
	for (int i = 0; i < toCorrect.size(); i++)
	{
		std::vector<Facet> connectedFacets = getConnectedFacets(toCorrect[i].PosL, f);
		std::vector<XMFLOAT3> facetNormals;
		for (int j = 0; j < connectedFacets.size(); j++)
		{
			facetNormals.push_back(connectedFacets[j].surfaceNormal);
		}
		// ditch the duplercate normals
		removeDuplicateNormals(facetNormals);

		XMFLOAT3 normalsSum(0.0f, 0.0f,0.0f);
		for (int j = 0; j < facetNormals.size(); j++)
		{
			normalsSum.x += facetNormals[j].x;
			normalsSum.y += facetNormals[j].y;
			normalsSum.z += facetNormals[j].z;
		}
		// re-normalise normalsSum
		float mag = sqrtf(normalsSum.x * normalsSum.x +
			normalsSum.y * normalsSum.y +
			normalsSum.z * normalsSum.z);
		float scaleBy = 1.0f / mag;
		normalsSum.x *= scaleBy;
		normalsSum.y *= scaleBy;
		normalsSum.z *= scaleBy;
		toCorrect[i].NormL = normalsSum;
	}
}

std::vector<Facet> Terrain::getConnectedFacets(XMFLOAT3 vertex, std::vector<Facet> & facets)
{
	std::vector<Facet> rv;
	for (int i = 0; i < facets.size(); i++)
	{
		if (vertex.x == facets[i].v0.x &&
			vertex.y == facets[i].v0.y &&
			vertex.z == facets[i].v0.z)
		{
			rv.push_back(facets[i]);
		}
		else if (vertex.x == facets[i].v1.x &&
			vertex.y == facets[i].v1.y &&
			vertex.z == facets[i].v1.z)
		{
			rv.push_back(facets[i]);
		}
		else if (vertex.x == facets[i].v2.x &&
			vertex.y == facets[i].v2.y &&
			vertex.z == facets[i].v2.z)
		{
			rv.push_back(facets[i]);
		}
	}
	return rv;
}

void Terrain::removeDuplicateNormals(std::vector<DirectX::XMFLOAT3> & n)
{
	for (int i = 0; i < n.size(); i++)
	{
		for (int j = 0; j < n.size(); j++)
		{
			if (i == j)
			{
				// skip the check
			}
			else if (n[i].x == n[j].x &&
				n[i].y == n[j].y &&
				n[i].z == n[j].z)
			{
				n.erase(n.begin() + j);
				i = j = 0;
			}
		}
	}
}

void Terrain::altCorrectVertexNormals(std::vector<SimpleVertex> & toCorrect, std::vector<WORD> & indices)
{
	std::vector<Facet> f;
	// use the index buffer to determine the correct facets
	for (int i = 0; i < indices.size(); i += 3)
	{
		Facet tmpf;
		tmpf.v0 = toCorrect[indices[i]].PosL;
		tmpf.v1 = toCorrect[indices[i + 1]].PosL;
		tmpf.v2 = toCorrect[indices[i + 2]].PosL;
		// calculate the surface normal
		DirectX::XMFLOAT3 v0ToV1, v0ToV2;
		v0ToV1.x = tmpf.v1.x - tmpf.v0.x;
		v0ToV1.y = tmpf.v1.y - tmpf.v0.y;
		v0ToV1.z = tmpf.v1.z - tmpf.v0.z;

		v0ToV2.x = tmpf.v2.x - tmpf.v0.x;
		v0ToV2.y = tmpf.v2.y - tmpf.v0.y;
		v0ToV2.z = tmpf.v2.z - tmpf.v0.z;

		DirectX::XMVECTOR a, b;
		a = DirectX::XMLoadFloat3(&v0ToV1);
		b = DirectX::XMLoadFloat3(&v0ToV2);

		DirectX::XMVECTOR aCrossB = DirectX::XMVector3Cross(a, b);

		XMStoreFloat3(&tmpf.surfaceNormal, XMVector3Normalize(aCrossB));

		// add tmpF to f
		f.push_back(tmpf);
	}


	// set the vertexNormals to be (0,0,0)
	// loop through all of the vetices add if its part of the facet, add the facet normal to the vertex normal
	for (int i = 0; i < toCorrect.size(); i++)
	{
		toCorrect[i].NormL.x = toCorrect[i].NormL.y = toCorrect[i].NormL.z = 0.0f;
		for (int j = 0; j < f.size(); j++)
		{
			if (toCorrect[i].PosL.x == f[j].v0.x &&
				toCorrect[i].PosL.y == f[j].v0.y &&
				toCorrect[i].PosL.z == f[j].v0.z)
			{
				toCorrect[i].NormL.x += f[j].surfaceNormal.x;
				toCorrect[i].NormL.y += f[j].surfaceNormal.y;
				toCorrect[i].NormL.z += f[j].surfaceNormal.z;
			}
			else if (toCorrect[i].PosL.x == f[j].v1.x &&
				toCorrect[i].PosL.y == f[j].v1.y &&
				toCorrect[i].PosL.z == f[j].v1.z)
			{
				toCorrect[i].NormL.x += f[j].surfaceNormal.x;
				toCorrect[i].NormL.y += f[j].surfaceNormal.y;
				toCorrect[i].NormL.z += f[j].surfaceNormal.z;
			}
			else if (toCorrect[i].PosL.x == f[j].v2.x &&
				toCorrect[i].PosL.y == f[j].v2.y &&
				toCorrect[i].PosL.z == f[j].v2.z)
			{
				toCorrect[i].NormL.x += f[j].surfaceNormal.x;
				toCorrect[i].NormL.y += f[j].surfaceNormal.y;
				toCorrect[i].NormL.z += f[j].surfaceNormal.z;
			}
		}
		// finally renormalise the vertex normals 
		//(may count the same facet normal multiple times, 
		// so not mathermatically correct, but should be more in the same general direction)

		float mag = sqrtf(toCorrect[i].NormL.x *toCorrect[i].NormL.x
			+ toCorrect[i].NormL.y *toCorrect[i].NormL.y
			+ toCorrect[i].NormL.z *toCorrect[i].NormL.z);
		float scaleBy = 1.0f / mag;
		toCorrect[i].NormL.x *= scaleBy;
		toCorrect[i].NormL.y *= scaleBy;
		toCorrect[i].NormL.z *= scaleBy;
	}
}

XMFLOAT3 Terrain::calcSurfaceNormal(XMVECTOR a, XMVECTOR b, XMVECTOR c)
{
	XMVECTOR ab = XMVectorSubtract(b, a);
	XMVECTOR ac = XMVectorSubtract(c, a);
	XMVECTOR abCrossAc = XMVector3Cross(ab, ac);
	XMFLOAT3 rv;
	
	XMStoreFloat3(&rv, XMVector3Normalize(abCrossAc));
	return rv;
}