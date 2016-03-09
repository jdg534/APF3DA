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
	std::vector<unsigned int> indices;
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
		for (auto i = 0; i < m_subsets.size(); i++)
		{
			m_subsets[i].draw(deviceContext);
		}
	}

	void update(float dt, ID3D11DeviceContext * d3dDCptr)
	{
		XMStoreFloat4x4(&m_worldMat, XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z));
		updateAnimation(dt, m_animationIndex, d3dDCptr);
	}

	void updateAnimation(float dt, int activeAnimation, ID3D11DeviceContext * d3dDeviceContextPtr)
	{
		m_animations[activeAnimation].currentAnimationTime += dt;
		if (m_animations[activeAnimation].currentAnimationTime > m_animations[activeAnimation].totalAnimationTime)
		{
			m_animations[activeAnimation].currentAnimationTime = 0.0f;
		}

		// determine the current frame
		float currentFrame = m_animations[activeAnimation].currentAnimationTime * m_animations[activeAnimation].fps;

		int frameA = (int)floorf(currentFrame);
		int frameB = frameA + 1;
		if (frameA == m_animations[activeAnimation].nFrames-1)
		{
			frameB = 0;
		}

		float interpolationWeight = currentFrame - frameA;

		std::vector<Joint> calculatedSkel;

		for (int i = 0; i < m_animations[activeAnimation].nJoints; i++)
		{
			Joint tempJ;
			Joint jA = m_animations[activeAnimation].frameSkeleton[frameA][i];
			Joint jB = m_animations[activeAnimation].frameSkeleton[frameB][i];
			tempJ.parentID = jA.parentID;

			// get the orientation Quaternions
			XMVECTOR jaOri = XMVectorSet(jA.orientation.x, jA.orientation.y, jA.orientation.z, jA.orientation.w);
			XMVECTOR jbOri = XMVectorSet(jB.orientation.x, jB.orientation.y, jB.orientation.z, jB.orientation.w);

			// lerp the positions
			// XMVECTOR jaPos = XMLoadFloat3(&jA.pos);
			// XMVECTOR jbPos = XMLoadFloat3(&jB.pos);

			// XMVECTOR lerpedPos = XMVectorLerp(jaPos, jbPos, interpolationWeight);

			// XMStoreFloat3(&tempJ.pos, lerpedPos);

			tempJ.pos.x = jA.pos.x + (interpolationWeight * (jB.pos.x - jA.pos.x));
			tempJ.pos.y = jA.pos.y + (interpolationWeight * (jB.pos.y - jA.pos.y));
			tempJ.pos.z = jA.pos.z + (interpolationWeight * (jB.pos.z - jA.pos.z));



			// SLERP the orientation Quaternions
			XMStoreFloat4(&tempJ.orientation, XMQuaternionSlerp(jaOri, jbOri, interpolationWeight));

			calculatedSkel.push_back(tempJ);
		}

		for (auto currentSubset = 0; currentSubset < nSubsets; currentSubset++)
		{
			for (auto currentVertex = 0; currentVertex < m_subsets[currentSubset].vertices.size(); currentVertex++)
			{
				SimpleVertex tempSV = m_subsets[currentSubset].vertices[currentVertex];
				tempSV.PosL = XMFLOAT3(0.0f, 0.0f, 0.0f);
				tempSV.NormL = XMFLOAT3(0.0f,0.0f,0.0f);

				// now to Sum the joints & weights, for the position & normal
				for (int currentWeight = 0; currentWeight < tempSV.weightCount; currentWeight++)
				{
					Weight wtemp = m_subsets[currentSubset].weights[tempSV.startWeight + currentWeight];
					Joint tempJointFromModel = calculatedSkel[wtemp.jointID];

					// To XMVECTOR form
					XMVECTOR tjfmOri = XMVectorSet(tempJointFromModel.orientation.x, tempJointFromModel.orientation.y, tempJointFromModel.orientation.z, tempJointFromModel.orientation.w);
					XMVECTOR tjfmPos = XMVectorSet(tempJointFromModel.pos.x, tempJointFromModel.pos.y, tempJointFromModel.pos.z, 0.0f); // XMLoadFloat3 would have been faster, but better to be clear in the w value

					// calc the Conjugate for the orientation, still following: http://www.braynzarsoft.net/viewtutorial/q16390-28-skeletal-animation-based-on-the-md5-format
					XMVECTOR tjfmOriConjugate = XMQuaternionInverse(tjfmOri);

					XMFLOAT3 rotatedPoint;
					XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(tjfmOri, tjfmPos), tjfmOriConjugate));

					// add to tempSV.pos (rember the bias)
					tempSV.PosL.x += (tempJointFromModel.pos.x + rotatedPoint.x) * wtemp.bias;
					tempSV.PosL.y += (tempJointFromModel.pos.y + rotatedPoint.y) * wtemp.bias;
					tempSV.PosL.z += (tempJointFromModel.pos.z + rotatedPoint.z) * wtemp.bias;

					XMVECTOR wtempNormal = XMVectorSet(wtemp.normal.x, wtemp.normal.y, wtemp.normal.z, 0.0f);

					// rotate the normal, note value stored in "rotatedPoint"
					XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(tjfmOri, wtempNormal), tjfmOriConjugate));

					tempSV.NormL.x -= rotatedPoint.x * wtemp.bias;
					tempSV.NormL.y -= rotatedPoint.y * wtemp.bias;
					tempSV.NormL.z -= rotatedPoint.z * wtemp.bias;

				}
				m_subsets[currentSubset].positions[currentVertex] = tempSV.PosL;
				m_subsets[currentSubset].vertices[currentVertex].NormL = tempSV.NormL;
				XMStoreFloat3(&m_subsets[currentSubset].vertices[currentVertex].NormL, XMVector3Normalize(XMLoadFloat3(&m_subsets[currentSubset].vertices[currentVertex].NormL)));
			}

			// set the positions for vertices via values stored in m_subsets[n].positions
			for (auto i = 0; i < m_subsets[currentSubset].vertices.size(); i++)
			{
				m_subsets[currentSubset].vertices[i].PosL = m_subsets[currentSubset].positions[i];
			}

			// finally update the vertex buffer with the Vertex buffer for each subset, index buffer can stay the same

			// Lock the Buffer
			D3D11_MAPPED_SUBRESOURCE mappedVertBuf;
			HRESULT hr = d3dDeviceContextPtr->Map(m_subsets[currentSubset].geomatry.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertBuf);

			// copy data to mappedVertBuf, changes the value in the vertex buffer
			memcpy(mappedVertBuf.pData, &m_subsets[currentSubset].vertices[0], (sizeof(SimpleVertex) * m_subsets[currentSubset].vertices.size()));

			d3dDeviceContextPtr->Unmap(m_subsets[currentSubset].geomatry.vertexBuffer, 0);

			// correct this later, see: http://www.braynzarsoft.net/viewtutorial/q16390-28-skeletal-animation-based-on-the-md5-format
		}
	}

	XMMATRIX getWorldMat()
	{
		return XMLoadFloat4x4(&m_worldMat);
	}

	DirectX::XMFLOAT4X4 m_worldMat;
	DirectX::XMFLOAT3 m_pos;

	// Added for the Skeletal animation
	std::vector<ModelAnimation> m_animations;
	unsigned short m_animationIndex;
};


#endif