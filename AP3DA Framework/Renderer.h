#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <d3d11.h>

#include "Structures.h"

#include "Camera.h"

// #include "GameObject.h"
#include "Terrain.h"

#include "SkeletalModel.h"
// #include "MD3Model.h"

class Renderer
{
public:
	Renderer();

	~Renderer();

	HRESULT init(HWND windowHandle);
	void shutdown();

	ID3D11Device * getDevicePtr(){ return m_d3dDevicePtr; }
	ID3D11DeviceContext * getDeviceContextPtr(){ return m_d3dDeviceContextPtr; }

	void changeRenderMode()
	{
		m_activeRenderMode++;
		if (m_activeRenderMode >= 4)
		{
			m_activeRenderMode = 0;
		}
	}

	void startDrawing(float * clearColor, Camera * camForFrame, bool wireFrame);

	// void drawGameObjects(std::vector<GameObject *> & toDraw);

	void drawTerrain(Terrain * toDraw);

	//void drawMD5Model(SkeletalModel * toDraw);
	//void drawMD3Model(MD3ModelInstance * toDraw);
	void drawSkeletalModel(SkeletalModelInstance * toDraw);

	void finshDrawing();

	void setLight(Light l){ m_basicLight = l; }

private:
	ID3D11Device * m_d3dDevicePtr;
	ID3D11DeviceContext * m_d3dDeviceContextPtr;


	D3D_DRIVER_TYPE         m_driverType;
	D3D_FEATURE_LEVEL       m_featureLevel;
	IDXGISwapChain*         m_swapChainPtr;
	ID3D11RenderTargetView* m_RenderTargetViewPtr;
	
	
	// start of 
	ID3D11VertexShader*     m_vertexShaderPtr;
	ID3D11PixelShader*      m_pixelShaderPtr;
	ID3D11InputLayout*      m_vertexLayoutPtr;

	// ID3D11Buffer*           _pPlaneVertexBuffer;
	// ID3D11Buffer*           _pPlaneIndexBuffer;

	ID3D11Buffer*           m_constantBufferPtr;

	ID3D11DepthStencilView* m_depthStencilViewPtr = nullptr;
	ID3D11Texture2D* m_depthStencilBufferPtr = nullptr;

	//ID3D11ShaderResourceView * m_renderTargetPtr = nullptr;

	ID3D11ShaderResourceView * m_terrainLightDirtTex = nullptr; // for aprox sand
	ID3D11ShaderResourceView * m_terrainGrassTex = nullptr;
	ID3D11ShaderResourceView * m_terrainDarkDirtTex = nullptr;
	ID3D11ShaderResourceView * m_terrainStoneTex = nullptr;
	ID3D11ShaderResourceView * m_terrainSnowTex = nullptr;


	ID3D11SamplerState * m_samplerPtr = nullptr;

	Light m_basicLight;
	Camera * m_activeCamera;


	UINT m_height;
	UINT m_width;

	ID3D11DepthStencilState* m_DSLessEqual;
	

	ID3D11BlendState* m_Transparency;
	
	ID3D11RasterizerState* m_RSCullNone;
	ID3D11RasterizerState* m_CCWcullMode;
	ID3D11RasterizerState* m_CWcullMode;
	ID3D11RasterizerState* m_wireframe;
	// ID3D11RasterizerState * m_noneCull;
	int m_activeRenderMode = 0;


	bool m_IsWireFrame = false;

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();

	bool loadTerrainTextures();
};


#endif 