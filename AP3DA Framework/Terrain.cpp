#include "Terrain.h"

#include "Structures.h"

#include "MathFuncs.h"

#include <fstream>

float diff(float a, float b)
{
	if (a>b)
	{
		return a - b;
	}
	return b - a;
}

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
	// should be done in a shutdown function
	m_geometry.vertexBuffer->Release();
	m_geometry.indexBuffer->Release();
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

	DirectX::XMFLOAT2 t;
	t.x = -(m_width / 2.0f);
	t.y = m_depth / 2.0f;

	int k = 0;

	std::vector<DirectX::XMFLOAT3> verts;
	verts.resize(nVerts);

	for (auto i = 0; i < mRows; i++)
	{
		for (auto j = 0; j < nColumns; j++)
		{
			verts[k].x = j * cellWidth + (-m_width * 0.5f);
			verts[k].y = 0.0f;
			verts[k].z = -(i * cellDepth) + (m_depth * 0.5f);
			k++;
		}
	}

	std::vector<unsigned int> indices;

	// for the ijth Quad


	// code based off slide 9 of first lecture

	for (int i = 0; i < mRows - 1; i++)
	{
		for (int j = 0; j < nColumns - 1; j++)
		{
			DirectX::XMINT3 abc;
			abc.x = i * mRows + j;
			abc.y = i * mRows + j + 1;
			abc.z = (i + 1) * mRows + j;

			DirectX::XMINT3 cbd;
			cbd.x = (i + 1) * mRows + j;
			cbd.y = i * mRows + j + 1;
			cbd.z = (i + 1) * mRows + j + 1;

			// add to the vector as WORD use static_cast<WORD>()
			indices.push_back(static_cast<unsigned int>(abc.x));
			indices.push_back(static_cast<unsigned int>(abc.y));
			indices.push_back(static_cast<unsigned int>(abc.z));

			indices.push_back(static_cast<unsigned int>(cbd.x));
			indices.push_back(static_cast<unsigned int>(cbd.y));
			indices.push_back(static_cast<unsigned int>(cbd.z));
		}
	}

	/*
	0,0---1,0
	|		|
	0,1---1,1
	*/

	DirectX::XMFLOAT2 topLeft(99999.99f, 99999.99f), bottomRight(-topLeft.x, -topLeft.y);

	for (auto i = 0; i < verts.size(); i++)
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
	for (auto i = 0; i < verts.size(); i++)
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
	indBufdesc.ByteWidth = sizeof(unsigned int) * indices.size();
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

	DirectX::XMFLOAT2 t;
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

	std::vector<unsigned int> indices;

	// for the ijth Quad


	// code partly based off slide 9 of first lecture

	for (int i = 0; i < m_rows - 1; i++)
	{
		for (int j = 0; j < m_columns - 1; j++)
		{
			// treat each tri as a facet
			
			DirectX::XMINT3 abc;
			abc.x = i * m_rows + j;
			abc.y = i * m_rows + j + 1;
			abc.z = (i + 1) * m_rows + j;

			DirectX::XMFLOAT3 abcSn = calcSurfaceNormal(XMLoadFloat3(&verts[abc.x].pos),
				XMLoadFloat3(&verts[abc.y].pos),
				XMLoadFloat3(&verts[abc.z].pos));

			
						

			DirectX::XMINT3 cbd;
			cbd.x = (i + 1) * m_rows + j;
			cbd.y = i * m_rows + j + 1;
			cbd.z = (i + 1) * m_rows + j + 1;

			DirectX::XMFLOAT3 cbdSn = calcSurfaceNormal(XMLoadFloat3(&verts[cbd.x].pos),
				XMLoadFloat3(&verts[cbd.y].pos),
				XMLoadFloat3(&verts[cbd.z].pos));

			verts[abc.x].surfaceNormals.push_back(abcSn);
			verts[abc.y].surfaceNormals.push_back(abcSn);
			verts[abc.z].surfaceNormals.push_back(abcSn);

			verts[cbd.x].surfaceNormals.push_back(cbdSn);
			verts[cbd.y].surfaceNormals.push_back(cbdSn);
			verts[cbd.z].surfaceNormals.push_back(cbdSn);

			// add to the vector as WORD use static_cast<WORD>()
			indices.push_back(static_cast<unsigned int>(abc.x));
			indices.push_back(static_cast<unsigned int>(abc.y));
			indices.push_back(static_cast<unsigned int>(abc.z));

			indices.push_back(static_cast<unsigned int>(cbd.x));
			indices.push_back(static_cast<unsigned int>(cbd.y));
			indices.push_back(static_cast<unsigned int>(cbd.z));
		}
	}



	/*
	0,0---1,0
	|		|
	0,1---1,1
	*/

	DirectX::XMFLOAT2 topLeft(99999.99f, 99999.99f), bottomRight(-topLeft.x, -topLeft.y);

	for (auto i = 0; i < verts.size(); i++)
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
	for (auto i = 0; i < verts.size(); i++)
	{
		SimpleVertex sv;
		sv.PosL.x = verts[i].pos.x;
		sv.PosL.y = verts[i].pos.y;
		sv.PosL.z = verts[i].pos.z;



		// this has to be changed
		// will just have a normals correction func, calc surface normal, then 
		
		DirectX::XMFLOAT3 VertexNormal(0.0, 0.0, 0.0);
		for (auto j = 0; j < verts[i].surfaceNormals.size(); j++)
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
	indBufdesc.ByteWidth = sizeof(unsigned int) * indices.size();
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

bool Terrain::resetShapeViaHeightMap(HeightMap * hm, float scaleHeightBy, ID3D11Device * d3dDPtr, ID3D11DeviceContext * d3dDC, float width, float depth)
{
	// this is really a modified version of initViaHeightMap

	m_rows = hm->getWidth();
	m_columns = hm->getDepth();
	m_cellRows = m_rows - 1;
	m_cellColumns = m_columns - 1;

	m_scaleHeightBy = scaleHeightBy;

	m_width = width;
	m_depth = depth;

	m_cellWidth = m_width / (float)m_rows;
	m_cellDepth = m_depth / (float)m_columns;

	int nCells = m_cellRows * m_cellColumns;
	int nTriangles = nCells * 2;
	int nVerts = m_rows * m_columns;

	DirectX::XMFLOAT2 t;
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

	std::vector<unsigned int> indices;

	// for the ijth Quad


	// code partly based off slide 9 of first lecture

	for (int i = 0; i < m_rows - 1; i++)
	{
		for (int j = 0; j < m_columns - 1; j++)
		{
			// treat each tri as a facet

			DirectX::XMINT3 abc;
			abc.x = i * m_rows + j;
			abc.y = i * m_rows + j + 1;
			abc.z = (i + 1) * m_rows + j;

			DirectX::XMFLOAT3 abcSn = calcSurfaceNormal(XMLoadFloat3(&verts[abc.x].pos),
				XMLoadFloat3(&verts[abc.y].pos),
				XMLoadFloat3(&verts[abc.z].pos));




			DirectX::XMINT3 cbd;
			cbd.x = (i + 1) * m_rows + j;
			cbd.y = i * m_rows + j + 1;
			cbd.z = (i + 1) * m_rows + j + 1;

			DirectX::XMFLOAT3 cbdSn = calcSurfaceNormal(XMLoadFloat3(&verts[cbd.x].pos),
				XMLoadFloat3(&verts[cbd.y].pos),
				XMLoadFloat3(&verts[cbd.z].pos));

			verts[abc.x].surfaceNormals.push_back(abcSn);
			verts[abc.y].surfaceNormals.push_back(abcSn);
			verts[abc.z].surfaceNormals.push_back(abcSn);

			verts[cbd.x].surfaceNormals.push_back(cbdSn);
			verts[cbd.y].surfaceNormals.push_back(cbdSn);
			verts[cbd.z].surfaceNormals.push_back(cbdSn);

			// add to the vector as WORD use static_cast<WORD>()
			indices.push_back(static_cast<unsigned int>(abc.x));
			indices.push_back(static_cast<unsigned int>(abc.y));
			indices.push_back(static_cast<unsigned int>(abc.z));

			indices.push_back(static_cast<unsigned int>(cbd.x));
			indices.push_back(static_cast<unsigned int>(cbd.y));
			indices.push_back(static_cast<unsigned int>(cbd.z));
		}
	}



	/*
	0,0---1,0
	|		|
	0,1---1,1
	*/

	DirectX::XMFLOAT2 topLeft(99999.99f, 99999.99f), bottomRight(-topLeft.x, -topLeft.y);

	for (auto i = 0; i < verts.size(); i++)
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
	for (auto i = 0; i < verts.size(); i++)
	{
		SimpleVertex sv;
		sv.PosL.x = verts[i].pos.x;
		sv.PosL.y = verts[i].pos.y;
		sv.PosL.z = verts[i].pos.z;



		// this has to be changed
		// will just have a normals correction func, calc surface normal, then 

		DirectX::XMFLOAT3 VertexNormal(0.0, 0.0, 0.0);
		for (auto j = 0; j < verts[i].surfaceNormals.size(); j++)
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

	m_heightMap = hm; // height maps are actually deleted in the height map manager class (don't worry about memory leaks)

	





	// handle sending the data to actual d3d buffers, (override, NOT create!)
	
	/*
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

	HRESULT hr = d3dDPtr->CreateBuffer(&bd, &InitData, &m_geometry.vertexBuffer);
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
	indBufdesc.ByteWidth = sizeof(unsigned int) * indices.size();
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
	*/

	// above is the original code (for creating the buffer with the correct data)

	d3dDC->UpdateSubresource(m_geometry.vertexBuffer, 0, NULL, &vertsToSendToD3dBuffer[0], 0, 0);

	d3dDC->UpdateSubresource(m_geometry.indexBuffer, 0, NULL, &indices[0], 0, 0);
	

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
	pImmediateContext->IASetIndexBuffer(m_geometry.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	pImmediateContext->DrawIndexed(m_geometry.numberOfIndices, 0, 0);
}

bool Terrain::isPositionOnTerrain(float x, float z)
{
	// first 
	// like AABB

	// height map position += m_position

	DirectX::XMFLOAT2 actualTopLeftCoord;
	DirectX::XMFLOAT2 actualBottomRight;
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



	// DirectX::XMFLOAT2 xzRelativeToTheTopLeft = positionOnHeightMap(x, z);
	
	// cells stored by terrain class in system of 0,0 bottom left?

	float leftSideWorldSpace = m_position.x + m_topLeftPoint.x;
	float rightSideWorldSpace = m_position.x + m_bottomRightPoint.y;

	float topSideWorldSpace = m_position.z + m_topLeftPoint.y;
	float bottomSideWorldSpace = m_position.z + m_bottomRightPoint.y;

	float distFromLeftSide = diff(x, leftSideWorldSpace);
	float distFromRighSide = diff(x, rightSideWorldSpace);

	float distFromTopSide = diff(z, topSideWorldSpace);
	float distFromBottomSide = diff(z, bottomSideWorldSpace);

	// calculate nCells covered in X

	// calculate nCells covered in Z
	float xCellsCovered = 0.0f;
	// float zCellsCovered = zDistFromBottom / m_cellDepth;
	float zCellsCovered = 0.0f;

	// testing with 0,0 top left (terrain space)
	xCellsCovered = distFromLeftSide / m_cellWidth;
	zCellsCovered = distFromTopSide / m_cellDepth;

	// try 0,0 bottom left
	zCellsCovered = distFromBottomSide / m_cellDepth;

	unsigned int xCellsCoveredUIntForm = (unsigned int)xCellsCovered;
	unsigned int zCellsCoveredUIntForm = (unsigned int)zCellsCovered;
	
	if (xCellsCoveredUIntForm >= m_cellColumns)
	{
		xCellsCoveredUIntForm = m_cellColumns - 2;
	}
	if (zCellsCoveredUIntForm >= m_cellRows)
	{
		zCellsCoveredUIntForm = m_cellRows - 2;
	}

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

	
	// to conform to the notation in the frank lunk book:
	/*
	A---B
	|  /|
	| / |
	|/	|
	C---D
	
	*/
	
	float A, B,
		C, D;
	A = heightTL_F;
	B = heightTR_F;
	C = heightBL_F;
	D = heightBR_F;

	if (A < 0.0f ||
		B < 0.0f ||
		C < 0.0f ||
		D < 0.0f)
	{
		// something went very wrong
		char breakPointVar = 'E';
	}


	if (s+t <= 1.0f) 
	{
		// do stuff in ABC

		float uy, vy, rv;
		uy = B - A;
		vy = C - A;
		rv = A + s * uy + t * vy;
		return rv;
	}
	else
	{
		// do stuff in DCB
		float uy, vy, rv;
		uy = C - D;
		vy = B - D;
		rv = D + (1.0f - s)*uy + (1.0f - t)*vy;


		return rv;
	}

}

DirectX::XMFLOAT2 Terrain::positionOnHeightMap(float x, float z)
{
	float leftOfTerrain = m_topLeftPoint.x + m_position.x;

	float bottomOfTerrain = m_bottomRightPoint.y + m_position.z; // note distance going down same as the distance going up

	float topOfTerrain = -bottomOfTerrain;

	// topOfTerrain = m_topLeftPoint.y + m_position.z; the size from mid point to top should be the same as the mid point to top

	DirectX::XMFLOAT2 rv;

	rv.x = diff(leftOfTerrain, x);

	rv.y = diff(topOfTerrain, z);

	return rv;
}

float Terrain::calculateTextureCoord(float minPos, float maxPos, float pos)
{
	// float stepSize = 0.01f;
	// float currentStep = 0.0f;

	/*
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
	*/
	// better way
	float diffMinMax = diff(minPos, maxPos);
	float diffMinPos = diff(minPos, pos);
	if (diffMinMax == 0.0f)
	{
		return 1.0f;
	}

	return diffMinPos / diffMinMax;

	return 1.0f;
}

void Terrain::correctVertexNormals(std::vector<SimpleVertex> & toCorrect, std::vector<unsigned int> & indices)
{
	std::vector<Facet> f;
	// use the index buffer to determine the correct facets
	for (auto i = 0; i < indices.size(); i += 3)
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

		DirectX::XMStoreFloat3(&tmpf.surfaceNormal, DirectX::XMVector3Normalize(aCrossB));

		// add tmpF to f
		f.push_back(tmpf);
	}

	// make the corrections 
	for (auto i = 0; i < toCorrect.size(); i++)
	{
		std::vector<Facet> connectedFacets = getConnectedFacets(toCorrect[i].PosL, f);
		std::vector<DirectX::XMFLOAT3> facetNormals;
		for (int j = 0; j < connectedFacets.size(); j++)
		{
			facetNormals.push_back(connectedFacets[j].surfaceNormal);
		}
		// ditch the duplercate normals
		removeDuplicateNormals(facetNormals);

		DirectX::XMFLOAT3 normalsSum(0.0f, 0.0f, 0.0f);
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

std::vector<Facet> Terrain::getConnectedFacets(DirectX::XMFLOAT3 vertex, std::vector<Facet> & facets)
{
	std::vector<Facet> rv;
	for (auto i = 0; i < facets.size(); i++)
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
	for (auto i = 0; i < n.size(); i++)
	{
		for (auto j = 0; j < n.size(); j++)
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

void Terrain::altCorrectVertexNormals(std::vector<SimpleVertex> & toCorrect, std::vector<unsigned int> & indices)
{
	std::vector<Facet> f;
	// use the index buffer to determine the correct facets
	for (auto i = 0; i < indices.size(); i += 3)
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

		DirectX::XMStoreFloat3(&tmpf.surfaceNormal, DirectX::XMVector3Normalize(aCrossB));

		// add tmpF to f
		f.push_back(tmpf);
	}


	// set the vertexNormals to be (0,0,0)
	// loop through all of the vetices add if its part of the facet, add the facet normal to the vertex normal
	for (auto i = 0; i < toCorrect.size(); i++)
	{
		toCorrect[i].NormL.x = toCorrect[i].NormL.y = toCorrect[i].NormL.z = 0.0f;
		for (auto j = 0; j < f.size(); j++)
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

DirectX::XMFLOAT3 Terrain::calcSurfaceNormal(DirectX::XMVECTOR a, DirectX::XMVECTOR b, DirectX::XMVECTOR c)
{
	DirectX::XMVECTOR ab = DirectX::XMVectorSubtract(b, a);
	DirectX::XMVECTOR ac = DirectX::XMVectorSubtract(c, a);
	DirectX::XMVECTOR abCrossAc = DirectX::XMVector3Cross(ab, ac);
	DirectX::XMFLOAT3 rv;
	
	DirectX::XMStoreFloat3(&rv, DirectX::XMVector3Normalize(abCrossAc));
	return rv;
}