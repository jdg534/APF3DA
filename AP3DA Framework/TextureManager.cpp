#include "TextureManager.h"
#include "DDSTextureLoader.h"

TextureManager * f_tmInst = nullptr;

TextureManager * TextureManager::getInstance()
{
	if (f_tmInst == nullptr)
	{
		f_tmInst = new TextureManager();
	}
	return f_tmInst;
}

TextureManager::TextureManager()
{
	m_devicePtr = nullptr;
}

TextureManager::~TextureManager()
{
	// shutdown handles the deletions
}

bool TextureManager::addTexture(std::string filePath)
{
	Texture * t = new Texture;
	
	t->ID = filePath;

	std::wstring fileWSTR;
	fileWSTR.assign(filePath.begin(), filePath.end());
	
	if (textureAlreadyPresent(filePath))
	{
		return true;
	}

	HRESULT res = DirectX::CreateDDSTextureFromFile(m_devicePtr, fileWSTR.c_str(), nullptr, &t->imageMapPtr);

	if (FAILED(res))
	{
		t->imageMapPtr->Release();
		delete t;
		return false;
	}
	m_textures.push_back(t);
	return true;
}

Texture * TextureManager::getTextureWithID(std::string ID)
{
	for (auto i = 0; i < m_textures.size(); i++)
	{
		if (m_textures[i]->ID == ID)
		{
			return m_textures[i];
		}
	}
	return nullptr;
}

void TextureManager::init(ID3D11Device * d3dDevicePtr)
{
	m_devicePtr = d3dDevicePtr;
}

void TextureManager::shutdown()
{
	for (auto i = 0; i < m_textures.size(); i++)
	{
		m_textures[i]->imageMapPtr->Release();
		delete m_textures[i];
	}
	m_textures.clear();
}

bool TextureManager::textureAlreadyPresent(std::string texID)
{
	for (auto i = 0; i < m_textures.size(); i++)
	{
		if (m_textures[i]->ID == texID)
			return true;
	}
	return false;
}