#include "Terrain.h"

#include "Structures.h"

#include "Math.h"

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

bool Terrain::initViaHeightMap(HeightMap * hm, float scaleHeightBy, ID3D11Device * devicePtr)
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
	m_cellWidth = 1.0f; // should have as a parameter
	m_cellDepth = 1.0f;
	m_width = m_rows * m_cellWidth;
	m_depth = m_columns * m_cellDepth;


	int nCells = m_cellRows * m_cellColumns;
	int nTriangles = nCells * 2;
	int nVerts = m_rows * m_columns;

	XMFLOAT2 t;
	t.x = -(m_width / 2.0f);
	t.y = m_depth / 2.0f;

	int k = 0;

	std::vector<XMFLOAT3> verts;
	verts.resize(nVerts);

	for (int i = 0; i < m_rows; i++)
	{
		for (int j = 0; j < m_columns; j++)
		{
			verts[k].x = j * m_cellWidth + (-m_width * 0.5f);
			// verts[k].y = 0.0f; original

			unsigned char hmVal = hm->getHeightAt(i, j);

			// hmVal is 0 - 255, scale to 0.0 - 1.0
			float downScale = 1.0f / 255.0f;
			float downScaled = static_cast<float>(hmVal)* downScale;

			// rescale to the scale use requested

			verts[k].y = downScaled * scaleHeightBy;

			verts[k].z = -(i * m_cellDepth) + (m_depth * 0.5f);
			k++;
		}
	}

	std::vector<WORD> indices;

	// for the ijth Quad


	// code based off slide 9 of first lecture

	for (int i = 0; i < m_rows - 1; i++)
	{
		for (int j = 0; j < m_columns - 1; j++)
		{
			XMINT3 abc;
			abc.x = i * m_rows + j;
			abc.y = i * m_rows + j + 1;
			abc.z = (i + 1) * m_rows + j;

			XMINT3 cbd;
			cbd.x = (i + 1) * m_rows + j;
			cbd.y = i * m_rows + j + 1;
			cbd.z = (i + 1) * m_rows + j + 1;

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



		// this has to be changed
		// will just have a normals correction func, calc surface normal, then 
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

	m_heightMap = hm;

	correctVertexNormals(vertsToSendToD3dBuffer, indices);

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

float Terrain::calculateTextureCoord(float minPos, float maxPos, float pos)
{
	float stepSize = 0.001f;
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
			facetNormals.push_back(connectedFacets[i].surfaceNormal);
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