#include "SkeletalModelGeomatry.h"

SkeletalModelGeomatry::SkeletalModelGeomatry()
{
	m_vbPtr = nullptr;
	m_ibPtr = nullptr;
	m_indexBufferFormat = DXGI_FORMAT_R32_UINT;
	m_vertexStrideSize = 0;
}

SkeletalModelGeomatry::~SkeletalModelGeomatry()
{
	if (m_vbPtr != nullptr)
	{
		m_vbPtr->Release();
	}
	if (m_ibPtr->Release())
	{
		m_ibPtr->Release();
	}
}

void SkeletalModelGeomatry::setVertices(ID3D11Device* device, const SimpleVertex* vertices, unsigned int count)
{
	if (m_vbPtr != nullptr)
	{
		m_vbPtr->Release();
		m_vbPtr = nullptr;
	}
	m_vertexStrideSize = sizeof(SimpleVertex);

	

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(D3D11_BUFFER_DESC));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * count;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	// vbd.StructureByteStride = 0;
	vbd.StructureByteStride = m_vertexStrideSize;

	D3D11_SUBRESOURCE_DATA vinitData;
	ZeroMemory(&vinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	vinitData.pSysMem = vertices;

	device->CreateBuffer(&vbd, &vinitData, &m_vbPtr);
}

void SkeletalModelGeomatry::SetIndices(ID3D11Device* device, const unsigned int * indices, unsigned int count)
{
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(D3D11_BUFFER_DESC));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * count;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = sizeof(unsigned int);

	D3D11_SUBRESOURCE_DATA indBufinitData;
	ZeroMemory(&indBufinitData, sizeof(D3D11_SUBRESOURCE_DATA));
	indBufinitData.pSysMem = indices;

	device->CreateBuffer(&ibd, &indBufinitData, &m_ibPtr);
}

void SkeletalModelGeomatry::SetSubsetTable(std::vector<SkeletalModelSubSet>& subsetTable)
{
	m_subsets = subsetTable;
}

void SkeletalModelGeomatry::draw(ID3D11DeviceContext * d3dDC, unsigned int nSubset)
{
	unsigned int offset = 0;

	d3dDC->IASetVertexBuffers(0, 1, &m_vbPtr, &m_vertexStrideSize, &offset);
	d3dDC->IASetIndexBuffer(m_ibPtr, m_indexBufferFormat, offset);

	d3dDC->DrawIndexed(
		m_subsets[nSubset].FaceCount * 3,
		m_subsets[nSubset].FaceStart * 3,
		0);
}

SkeletalModelGeomatry::SkeletalModelGeomatry(const SkeletalModelGeomatry & toCopy)
{
	m_vbPtr = toCopy.m_vbPtr;
	m_ibPtr = toCopy.m_ibPtr;



	m_indexBufferFormat = toCopy.m_indexBufferFormat;
	m_vertexStrideSize = toCopy.m_vertexStrideSize;

	m_subsets = toCopy.m_subsets;
}

/*
SkeletalModelGeomatry & SkeletalModelGeomatry::operator =(const SkeletalModelGeomatry & toCopy)
{
	m_vbPtr = toCopy.m_vbPtr;
	m_ibPtr = toCopy.m_ibPtr;

	

	m_indexBufferFormat = toCopy.m_indexBufferFormat;
	m_vertexStrideSize = toCopy.m_vertexStrideSize;

	m_subsets = toCopy.m_subsets;
}
*/