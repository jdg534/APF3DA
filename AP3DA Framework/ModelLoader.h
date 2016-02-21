#ifndef _MODEL_LOADER_H_
#define _MODEL_LOADER_H_

#include "GameObject.h"
#include "SkeletalModel.h"

class ModelLoader
{
public:
	
	~ModelLoader();

	static ModelLoader * getInstance();

	void init(ID3D11Device * devicePtr);

	bool initialised()
	{
		if (m_devicePtr == nullptr)
		{
			return false;
		}
		return true;
	}

	bool loadMD5Mesh(std::string fileLoc, 
		SkeletalModel & md5MdlOut,
		std::vector<ID3D11ShaderResourceView*> & shaders,
		std::vector<std::string> texFileNames);

	bool loadMD5Animation(std::string fileLoc, SkeletalModel & md5MdlOut);

private:
	ModelLoader();

	ID3D11Device * m_devicePtr;
};


#endif
