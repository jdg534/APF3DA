#ifndef _SKELETAL_MODEL_GEOMATRY_H_
#define _SKELETAL_MODEL_GEOMATRY_H_

#include <d3d11.h>

#include "Structures.h"
#include "SkeletalModelStructs.h"

class SkeletalModelGeomatry
{
public:
	SkeletalModelGeomatry();
	~SkeletalModelGeomatry();


	void setVertices(ID3D11Device* device, const SimpleVertex* vertices, unsigned int count);

	void SetIndices(ID3D11Device* device, const unsigned int * indices, unsigned int count);

	void SetSubsetTable(std::vector<SkeletalModelSubSet>& subsetTable);

	void draw(ID3D11DeviceContext * d3dDC, unsigned int nSubset);

private:
	SkeletalModelGeomatry(const SkeletalModelGeomatry & toCopy);
	// SkeletalModelGeomatry & operator =(const SkeletalModelGeomatry & toCopy);

	ID3D11Buffer * m_vbPtr;
	ID3D11Buffer * m_ibPtr;

	DXGI_FORMAT m_indexBufferFormat;
	unsigned int m_vertexStrideSize; // size of the data type used for vertices

	std::vector<SkeletalModelSubSet> m_subsets;
};


#endif