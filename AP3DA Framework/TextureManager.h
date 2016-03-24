#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#include <d3d11.h>

#include <vector>
#include <string>

struct Texture
{
	ID3D11ShaderResourceView * imageMapPtr;
	std::string ID; // file path is used as an ID
};

class TextureManager
{
public:
	static TextureManager * getInstance();

	~TextureManager();

	bool addTexture(std::string filePath);

	Texture * getTextureWithID(std::string ID);

	void init(ID3D11Device * d3dDevicePtr);

	void shutdown();

private:
	std::vector<Texture *> m_textures;
	ID3D11Device * m_devicePtr;

	TextureManager();
};


#endif