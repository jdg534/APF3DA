#ifndef _MD3_MODEL_GEOMATRY_H_
#define _MD3_MODEL_GEOMATRY_H_

#include <d3d11.h>

#include "Structures.h"
#include "MD3ModelStructs.h"

class MD3ModelGeomatry
{
public:
	MD3ModelGeomatry();
	~MD3ModelGeomatry();


	void setVertices(ID3D11Device* device, const SkinnedVertex* vertices, unsigned int count);

	void SetIndices(ID3D11Device* device, const unsigned int * indices, unsigned int count);

	void SetSubsetTable(std::vector<MD3ModelSubSet>& subsetTable);

	void draw(ID3D11DeviceContext * d3dDC, unsigned int nSubset);

private:
	MD3ModelGeomatry(const MD3ModelGeomatry & toCopy);
	// MD3ModelGeomatry & operator =(const MD3ModelGeomatry & toCopy);

	ID3D11Buffer * m_vbPtr;
	ID3D11Buffer * m_ibPtr;

	DXGI_FORMAT m_indexBufferFormat;
	unsigned int m_vertexStrideSize; // size of the data type used for vertices

	std::vector<MD3ModelSubSet> m_subsets;
};


#endif