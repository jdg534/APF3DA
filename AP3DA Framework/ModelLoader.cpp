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
	std::string commandLineStr = "";
	int md5Version = 0;
	while (!theFile.eof())
	{
		theFile >> currentBit;
		if (currentBit == "MD5Version")
		{
			theFile >> md5Version;
		}
		else if (currentBit == "commandline")
		{
			std::getline(theFile, commandLineStr);
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
				
				// MD5 is right handed, this system is left handed
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
					std::string textureMapString = "";
					std::getline(theFile, textureMapString);
					
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

						DWORD vertIndex;  // TODO: Change this??
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
			for (auto i = 0; i < tmpSMS.vertices.size(); i++)
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
			
			for (auto i = 0; i < tmpSMS.vertices.size(); i++)
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

bool ModelLoader::loadMD5Animation(std::string fileLoc, SkeletalModel & md5MdlOut)
{
	std::ifstream animationFile(fileLoc);

	if (!animationFile.good())
	{
		return false;
	}

	ModelAnimation ma;

	std::string currentBit = "";
	while (!animationFile.eof())
	{
		animationFile >> currentBit;
		if (currentBit == "MD5Version")
		{
			// ignor it, just dealing with version 10
		}
		else if (currentBit == "commandline")
		{
			// also ignor it
		}
		else if (currentBit == "numFrames")
		{
			animationFile >> ma.nFrames;
		}
		else if (currentBit == "numJoints")
		{
			animationFile >> ma.nJoints;
		}
		else if (currentBit == "frameRate")
		{
			animationFile >> ma.fps;
		}
		else if (currentBit == "numAnimatedComponents")
		{
			animationFile >> ma.nAnimatedComponents;
		}
		else if (currentBit == "hierarchy")
		{
			AnimationJointInformation aji;
			animationFile >> currentBit; // {
			for (int i = 0; i < ma.nJoints; i++)
			{
				std::string s;
				animationFile >> s;
				if (s[0] == '"')
				{
					s = s.substr(1, s.size() - 2);
				}

				int parentIndex;

				int flags;
				int startIndex;

				animationFile >> parentIndex;
				animationFile >> flags;
				animationFile >> startIndex;

				aji.name = s;
				aji.parentID = parentIndex;
				aji.flags = flags;
				aji.startIndex = startIndex;

				bool matchedWithModelHierarchy = false;
				for (int  j = 0; j < md5MdlOut.nJoints; j++)
				{
					if (md5MdlOut.m_joints[j].name == aji.name) // string names match
					{
						if (md5MdlOut.m_joints[j].parentID == aji.parentID) // parent joints match
						{
							matchedWithModelHierarchy = true;
							ma.jointInfo.push_back(aji);
						}
					}
				}

				if (!matchedWithModelHierarchy)
				{
					return false; // animation file hierarchy not compaterble with the model
				}

				std::getline(animationFile, currentBit);

			}
		}
		else if (currentBit == "bounds")
		{
			// the bounding box for the model for the frame
			animationFile >> currentBit; // skip {
			for (int i = 0; i < ma.nFrames; i++)
			{
				animationFile >> currentBit; // skip (

				BoundingBox bb;

				animationFile >> bb.min.x >> bb.min.z >> bb.min.y;
				animationFile >> currentBit // skip )
					>> currentBit; // skip (
				animationFile >> bb.max.x >> bb.max.z >> bb.max.y;

				std::getline(animationFile, currentBit); // skip ) & rest of the line
				ma.frameBoundingBox.push_back(bb);
			}

		}
		else if (currentBit == "baseframe")
		{
			// this is the skeleton that the skeletons 
			animationFile >> currentBit;// {
			for (int i = 0; i < ma.nJoints; i++)
			{
				Joint jTmp;
				animationFile >> currentBit; // (
				animationFile >> jTmp.pos.x >> jTmp.pos.z >> jTmp.pos.y;
				animationFile >> currentBit // )
					>> currentBit; // (
				animationFile >> jTmp.orientation.x >> jTmp.orientation.z >> jTmp.orientation.y;
				std::getline(animationFile, currentBit); // skip ) & rest of line

				ma.baseFrameJoints.push_back(jTmp);
			}
		}
		else if (currentBit == "frame")
		{
			FrameData fd;
			animationFile >> fd.frameIndex;
			animationFile >> currentBit;// {
			for (int i = 0; i < ma.nAnimatedComponents; i++)
			{
				float tempD;
				animationFile >> tempD;
				fd.frameData.push_back(tempD);
			}
			ma.frameData.push_back(fd);

			// create frame skeleton
			std::vector<Joint> tempSkel;

			for (auto i = 0; i < ma.jointInfo.size(); i++)
			{
				int k = 0;
				// start with base frame joints
				Joint jTmp = ma.baseFrameJoints[i];

				jTmp.parentID = ma.jointInfo[i].parentID;

				// use flags to determine if need to reswap y & z

				// just going with the tutorial: http://www.braynzarsoft.net/viewtutorial/q16390-28-skeletal-animation-based-on-the-md5-format
				if (ma.jointInfo[i].flags & 1)
				{
					jTmp.pos.x = fd.frameData[ma.jointInfo[i].startIndex + k++];
				}
				if (ma.jointInfo[i].flags & 2)
				{
					// note in RH coord sys would be setting Y NOT Z
					jTmp.pos.z = fd.frameData[ma.jointInfo[i].startIndex + k++];
				}
				if (ma.jointInfo[i].flags & 4)
				{
					jTmp.pos.y = fd.frameData[ma.jointInfo[i].startIndex + k++];
				}
				if (ma.jointInfo[i].flags & 8)
				{
					jTmp.orientation.x = fd.frameData[ma.jointInfo[i].startIndex + k++];
				}
				if (ma.jointInfo[i].flags & 16)
				{
					jTmp.orientation.z = fd.frameData[ma.jointInfo[i].startIndex + k++];
				}
				if (ma.jointInfo[i].flags & 32)
				{
					jTmp.orientation.y = fd.frameData[ma.jointInfo[i].startIndex + k++];
				}
				
				// now the Quaternion W component, rember w != Theta
				float t = 1.0f - (jTmp.orientation.x * jTmp.orientation.x)
					- (jTmp.orientation.y * jTmp.orientation.y)
					- (jTmp.orientation.z * jTmp.orientation.z);
				if (t < 0.0f)
				{
					jTmp.orientation.w = 0.0f;
				}
				else
				{
					jTmp.orientation.w = -sqrtf(t);
				}

				// deal with the transformation, relative to patent
				if (jTmp.parentID >= 0) // The root Joint have -1 as its parent index
				{
					Joint jTmpParent = tempSkel[jTmp.parentID];

					XMVECTOR parentJointOrienation = XMVectorSet(jTmpParent.orientation.x, jTmpParent.orientation.y, jTmpParent.orientation.z, jTmpParent.orientation.w);
					XMVECTOR jointpos = XMLoadFloat3(&jTmp.pos);

					XMVECTOR parentOrientationJointCondugate = XMVectorSet(-jTmpParent.orientation.x, -jTmpParent.orientation.y, -jTmpParent.orientation.z, jTmpParent.orientation.w);;

					// position
					XMFLOAT3 rotatedPosition;
					XMStoreFloat3(&rotatedPosition, XMQuaternionMultiply(XMQuaternionMultiply(parentJointOrienation, jointpos), parentOrientationJointCondugate));

					jTmp.pos.x = rotatedPosition.x + jTmpParent.pos.x;
					jTmp.pos.y = rotatedPosition.y + jTmpParent.pos.y;
					jTmp.pos.z = rotatedPosition.z + jTmpParent.pos.z;


					// orientation
					XMVECTOR jTmpOri = XMVectorSet(jTmp.orientation.x, jTmp.orientation.y, jTmp.orientation.z, jTmp.orientation.w);
					jTmpOri = XMQuaternionMultiply(parentJointOrienation, jTmpOri);


					// re normalise
					jTmpOri = XMQuaternionNormalize(jTmpOri);

					// set the actual value
					XMStoreFloat4(&jTmp.orientation, jTmpOri);
				}
				// add the joint
				tempSkel.push_back(jTmp);
			}
			ma.frameSkeleton.push_back(tempSkel);
			animationFile >> currentBit; // the "}"
		}
	}

	// setup the animation variables
	ma.frameTime = 1.0f / ma.fps; // frameTime to store the time deticated to each frame
	ma.totalAnimationTime = ma.nFrames * ma.frameTime; // total amount of time spent on the entire animation
	ma.currentAnimationTime = 0.0f;// starts at the actual start of the animation


	// add the animation to the models's class
	md5MdlOut.m_animations.push_back(ma);


	return true;
}