#ifndef _SKELETAL_MODEL_H_
#define _SKELETAL_MODEL_H_

#include "Structures.h"

// #include <Windows.h>

#include <d3d11.h>

#include "GameObject.h"

// this as based off: http://www.braynzarsoft.net/viewtutorial/q16390-27-loading-an-md5-model

struct SkeletalModelSubset
{
	int texureArrayIndex;
	int nTriangles;
	int nVerts;
	int nWeights;

	std::vector<SimpleVertex> vertices;
	std::vector<WORD> indices;
	std::vector<Weight> weights;

	std::vector<DirectX::XMFLOAT3> positions;

	// ID3D11Buffer * vertexBuffer;
	// ID3D11Buffer * indexBuffer;

	Geometry geomatry;

	DirectX::XMFLOAT4X4 worldMat;

	void draw(ID3D11DeviceContext * dc)
	{
		dc->IASetVertexBuffers(0,1,&geomatry.vertexBuffer, &geomatry.vertexBufferStride, &geomatry.vertexBufferOffset);
		dc->IASetIndexBuffer(geomatry.indexBuffer, DXGI_FORMAT_R16_UINT,0);
		dc->DrawIndexed(geomatry.numberOfIndices, 0 ,0);
	}

	DirectX::XMFLOAT4X4 m_worldMat;
	DirectX::XMFLOAT3 m_pos;
};

struct SkeletalModel // will make into a class later
{
	SkeletalModel()
	{
		m_pos.x = m_pos.y = m_pos.z = 0.0f;

		XMStoreFloat4x4(&m_worldMat, XMMatrixIdentity());
	}

	int nJoints;
	int nSubsets;

	std::vector<Joint> m_joints;
	std::vector<SkeletalModelSubset> m_subsets;

	void draw(ID3D11DeviceContext * deviceContext)
	{
		for (int i = 0; i < m_subsets.size(); i++)
		{
			m_subsets[i].draw(deviceContext);
		}
	}

	void update(float dt)
	{
		XMStoreFloat4x4(&m_worldMat, XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z));

		for (int i = 0; i < m_subsets.size(); i++)
		{

		}
	}

	XMMATRIX getWorldMat()
	{
		return XMLoadFloat4x4(&m_worldMat);
	}

	DirectX::XMFLOAT4X4 m_worldMat;
	DirectX::XMFLOAT3 m_pos;
};


#endif