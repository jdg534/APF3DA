#include "ModelLoader.h"

#include <fstream>

ModelLoader * fileScopeInstancePtr = nullptr;

ModelLoader::ModelLoader()
{
	m_devicePtr = nullptr;
}

ModelLoader::~ModelLoader()
{
	fileScopeInstancePtr = nullptr;
}

ModelLoader * ModelLoader::getInstance()
{
	if (fileScopeInstancePtr == nullptr)
	{
		fileScopeInstancePtr = new ModelLoader();
	}
	return fileScopeInstancePtr;
}

void ModelLoader::init(ID3D11Device * devicePtr)
{
	if (m_devicePtr == nullptr)
	{
		m_devicePtr = devicePtr;
	}
}

bool ModelLoader::loadMD5Mesh(std::string fileLoc,
	SkeletalModel & md5MdlOut,
	std::vector<ID3D11ShaderResourceView*> & shaders,
	std::vector<std::string> texFileNames)
{
	std::ifstream theFile(fileLoc);
	if (!theFile.good())
	{
		return false;
	}

	md5MdlOut.m_joints.clear();
	md5MdlOut.m_subsets.clear();
	md5MdlOut.nJoints = 0;
	md5MdlOut.nSubsets = 0;

	std::string currentBit = "";
	while (!theFile.eof())
	{
		theFile >> currentBit;
		if (currentBit == "MD5Version")
		{
			// should be 10
		}
		else if (currentBit == "commandline")
		{
			// will be ignored
		}
		else if (currentBit == "numJoints")
		{
			theFile >> md5MdlOut.nJoints;
		}
		else if (currentBit == "numMeshes")
		{
			theFile >> md5MdlOut.nSubsets;
		}
		else if (currentBit == "joints")
		{
			theFile >> currentBit;// {
			for (int i = 0; i < md5MdlOut.nJoints; i++)
			{
				// example: "origin"	-1 ( -0.000000 0.001643 -0.000604 ) ( -0.707107 -0.000242 -0.707107 )		// 

				Joint j;
				
				theFile >> j.name;
				if (j.name[0] == '"')
				{
					j.name = j.name.substr(1, j.name.size() - 2);
				}

				
				theFile >> j.parentID;
				
				// (
				theFile >> currentBit;

				// the position
				// theFile >> j.pos.x >> j.pos.y >> j.pos.z;
				theFile >> j.pos.x >> j.pos.z >> j.pos.y;
				
				// )
				theFile >> currentBit;

				// (
				theFile >> currentBit;

				// the orientation
				// theFile >> j.orientation.x >> j.orientation.y >> j.orientation.z;
				
				// MD5 is right handed
				theFile >> j.orientation.x >> j.orientation.z >> j.orientation.y;

				// calc the rotation for w (see: http://www.braynzarsoft.net/viewtutorial/q16390-27-loading-an-md5-model)
				float t = 1.0f - (j.orientation.x * j.orientation.x)
					- (j.orientation.y * j.orientation.y)
					- (j.orientation.z * j.orientation.z);
				if (t < 0.0f)
				{
					j.orientation.w = 0.0f;
				}
				else
				{
					j.orientation.w = -sqrtf(t);
				}

				// ) & optional comments
				std::getline(theFile, currentBit);

				md5MdlOut.m_joints.push_back(j);
			}
			theFile >> currentBit; // }
		}
		else if (currentBit == "mesh")
		{
			SkeletalModelSubset tmpSMS;
			theFile >> currentBit; // {

			

			while (currentBit != "}")
			{
				theFile >> currentBit;
				
				if (currentBit == "shader")
				{
					// texture?
					// have a material manager, that gives then material or a given material name

				}
				else if (currentBit == "numverts")
				{
					theFile >> tmpSMS.nVerts;
					
					for (int i = 0; i < tmpSMS.nVerts; i++)
					{
						SimpleVertex tempV;
						theFile >> currentBit // vert
							>> currentBit // vertex index
							>> currentBit; // (
						theFile >> tempV.Tex.x;
						theFile >> tempV.Tex.y;

						theFile >> currentBit; // )

						theFile >> tempV.startWeight;
						theFile >> tempV.weightCount;
						//std::getline(theFile, currentBit);

						tmpSMS.vertices.push_back(tempV);
					}
				}
				else if (currentBit == "numtris")
				{
					theFile >> tmpSMS.nTriangles;
					for (int i = 0; i < tmpSMS.nTriangles; i++)
					{
						theFile >> currentBit; // tri
						theFile >> currentBit; // triangle index

						DWORD vertIndex;
						for (int j = 0; j < 3; j++)
						{
							theFile >> vertIndex;
							tmpSMS.indices.push_back(vertIndex);
						}
						std::getline(theFile, currentBit);
					}
				}
				else if (currentBit == "numweights")
				{
					theFile >> tmpSMS.nWeights;
					for (int i = 0; i < tmpSMS.nWeights; i++)
					{
						// example line: weight 0 16 0.333333 ( -0.194917 0.111128 -0.362937 )
						Weight w;

						theFile >> currentBit >> currentBit; // weight <weight index>
						theFile >> w.jointID;

						theFile >> w.bias;

						theFile >> currentBit; // (
						theFile >> w.position.x >> w.position.z >> w.position.y;

						std::getline(theFile, currentBit);// )
						tmpSMS.weights.push_back(w);
					}
				}
			}
			
			// now figure out the actual vertex position, via the joints & weights
			for (int i = 0; i < tmpSMS.vertices.size(); i++)
			{
				SimpleVertex svTmp = tmpSMS.vertices[i];
				svTmp.PosL = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				for (int j = 0; j < svTmp.weightCount; j++)
				{
					Weight tempW = tmpSMS.weights[svTmp.startWeight + j];
					Joint tempJ = md5MdlOut.m_joints[tempW.jointID];

					XMVECTOR tempJointOrientation = XMLoadFloat4(&tempJ.orientation);
					XMVECTOR tempWeightPos = XMVectorSet(tempW.position.x, tempW.position.y, tempW.position.z, 0.0f);

					// calc the condugate of joint orientation Quaternion
					XMVECTOR tempJointOrientationConjugate = XMVectorSet(-tempJ.orientation.x, -tempJ.orientation.y, -tempJ.orientation.z, tempJ.orientation.w);

					XMFLOAT3 rotatedPoint;
					XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(tempJointOrientation, tempWeightPos), tempJointOrientationConjugate));

					svTmp.PosL.x += (tempJ.pos.x + rotatedPoint.x) * tempW.bias;
					svTmp.PosL.y += (tempJ.pos.y + rotatedPoint.y) * tempW.bias;
					svTmp.PosL.z += (tempJ.pos.z + rotatedPoint.z) * tempW.bias;


				}
				tmpSMS.positions.push_back(svTmp.PosL);
			}
			
			for (int i = 0; i < tmpSMS.vertices.size(); i++)
			{
				tmpSMS.vertices[i].PosL = tmpSMS.positions[i];
			}
			
			// now the vertex normals
			std::vector<XMFLOAT3> tempNormals;

			XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

			float vx = 0.0f, vy = 0.0f, vz = 0.0f;

			XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			// calc face normals
			for (int i = 0; i < tmpSMS.nTriangles; i++)
			{
				// 
				//Get the vector describing one edge of our triangle (edge 0,2)
				vx = tmpSMS.vertices[tmpSMS.indices[(i * 3)]].PosL.x - tmpSMS.vertices[tmpSMS.indices[(i * 3) + 2]].PosL.x;
				vy = tmpSMS.vertices[tmpSMS.indices[(i * 3)]].PosL.y - tmpSMS.vertices[tmpSMS.indices[(i * 3) + 2]].PosL.y;
				vz = tmpSMS.vertices[tmpSMS.indices[(i * 3)]].PosL.z - tmpSMS.vertices[tmpSMS.indices[(i * 3) + 2]].PosL.z;
				edge1 = XMVectorSet(vx, vy, vz, 0.0f);    //Create our first edge

				//Get the vector describing another edge of our triangle (edge 2,1)
				vx = tmpSMS.vertices[tmpSMS.indices[(i * 3) + 2]].PosL.x - tmpSMS.vertices[tmpSMS.indices[(i * 3) + 1]].PosL.x;
				vy = tmpSMS.vertices[tmpSMS.indices[(i * 3) + 2]].PosL.y - tmpSMS.vertices[tmpSMS.indices[(i * 3) + 1]].PosL.y;
				vz = tmpSMS.vertices[tmpSMS.indices[(i * 3) + 2]].PosL.z - tmpSMS.vertices[tmpSMS.indices[(i * 3) + 1]].PosL.z;
				edge2 = XMVectorSet(vx, vy, vz, 0.0f);    //Create our second edge

				XMVECTOR crossed = XMVector3Cross(edge1, edge2);
				crossed = XMVector3Normalize(crossed);


				// cross edge1 with edge2
				XMStoreFloat3(&unnormalized, crossed);



				tempNormals.push_back(unnormalized);
			}



			// now vertex normal averaging
			XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			int facesVertexUsedIn = 0;

			float tx, ty, tz;
			for (int i = 0; i < tmpSMS.vertices.size(); i++)
			{
				// determine which faces the vertex[i] is part of
				for (int j = 0; j < tmpSMS.nTriangles; j++) 
				{
					if (tmpSMS.indices[j * 3] == i ||
						tmpSMS.indices[(j * 3) + 1] == i ||
						tmpSMS.indices[(j * 3) + 2] == i)
					{
						tx = XMVectorGetX(normalSum) + tempNormals[j].x;
						ty = XMVectorGetY(normalSum) + tempNormals[j].y;
						tz = XMVectorGetZ(normalSum) + tempNormals[j].z;

						normalSum = XMVectorSet(tx, ty, tz, 0.0f);
						facesVertexUsedIn++;
					}
				}

				normalSum = normalSum / facesVertexUsedIn;

				normalSum = XMVector3Normalize(normalSum);


				tmpSMS.vertices[i].NormL.x = -XMVectorGetX(normalSum);
				tmpSMS.vertices[i].NormL.y = -XMVectorGetX(normalSum);
				tmpSMS.vertices[i].NormL.z = -XMVectorGetX(normalSum);

				// clear normalSum and facesVertexUsedIn for the next normal
				normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
				facesVertexUsedIn = 0;
			}

			// now Move Stuff to D3D buffers
			
			Geometry g;
			g.numberOfIndices = tmpSMS.indices.size();
			g.vertexBufferOffset = 0;
			g.vertexBufferStride = sizeof(SimpleVertex);
			// now the actual buffers


			// index
			D3D11_BUFFER_DESC indBufDesc;

			ZeroMemory(&indBufDesc, sizeof(indBufDesc));

			indBufDesc.Usage = D3D11_USAGE_DEFAULT;
			indBufDesc.ByteWidth = sizeof(DWORD) * tmpSMS.indices.size();
			indBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indBufDesc.CPUAccessFlags = 0;
			indBufDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA indBufInitData;
			indBufInitData.pSysMem = &tmpSMS.indices[0];
			m_devicePtr->CreateBuffer(&indBufDesc, &indBufInitData, &g.indexBuffer);

			// vertex
			D3D11_BUFFER_DESC vertBufDesc;
			ZeroMemory(&vertBufDesc, sizeof(vertBufDesc));

			vertBufDesc.Usage = D3D11_USAGE_DYNAMIC;
			vertBufDesc.ByteWidth = sizeof(SimpleVertex) * tmpSMS.vertices.size();
			vertBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allows CPU the write data to the Vewrtex Buffer
			vertBufDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA vertBufInitData;
			ZeroMemory(&vertBufInitData, sizeof(vertBufInitData));
			vertBufInitData.pSysMem = &tmpSMS.vertices[0];

			HRESULT hr;

			hr = m_devicePtr->CreateBuffer(&vertBufDesc, &vertBufInitData, &g.vertexBuffer);

			if (FAILED(hr))
			{
				return false;
			}

			tmpSMS.geomatry = g;

			// add tmpSMS to md5MdlOut
			md5MdlOut.m_subsets.push_back(tmpSMS);
		}

	}

	return true;
}