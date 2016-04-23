#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"

#include "Camera.h"
#include "FlyingCamera.h"
#include "FirstPersonCamera.h"




#include "GameObject.h"

#include "Terrain.h"

#include "Structures.h"

#include <chrono>
#include <vector>

// #include "ModelLoader.h"
// #include "SkeletalModel.h"
#include "MD3Model.h"

#include "TextureManager.h"

#include "Renderer.h"

#include "HeightMapManager.h"

class Application
{
	

private:
	

	HINSTANCE               _hInst;
	HWND                    _hWnd;
	

	/*
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;

	// start of MD3 skeletal model rendering stuff
	ID3D11VertexShader * m_skeletalModelVertexShader;
	ID3D11PixelShader * m_skeletalModelPixelShader;
	ID3D11InputLayout * m_SkeletalModelVertexLayout;
	
	ID3D11Buffer * m_SkeletalModelConstantBuffer;
	ID3D11Buffer * m_SkeletalModelBonesConstantBuffer;

	// end of MD3 skeletal model rendering stuff




	

	ID3D11Buffer*           _pPlaneVertexBuffer;
	ID3D11Buffer*           _pPlaneIndexBuffer;

	ID3D11Buffer*           _pConstantBuffer;

	ID3D11DepthStencilView* _depthStencilView = nullptr;
	ID3D11Texture2D* _depthStencilBuffer = nullptr;

	ID3D11ShaderResourceView * _pTextureRV = nullptr;

	ID3D11ShaderResourceView * m_terrainLightDirtTex = nullptr; // for aprox sand
	ID3D11ShaderResourceView * m_terrainGrassTex = nullptr;
	ID3D11ShaderResourceView * m_terrainDarkDirtTex = nullptr;
	ID3D11ShaderResourceView * m_terrainStoneTex = nullptr;
	ID3D11ShaderResourceView * m_terrainSnowTex = nullptr;
	*/

	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;

	// ID3D11SamplerState * _pSamplerLinear = nullptr;

	Light basicLight;

	vector<GameObject *> _gameObjects;

	unsigned int m_activeHeightMap;

	Camera * _camera;
	float _cameraOrbitRadius = 7.0f;
	float _cameraOrbitRadiusMin = 2.0f;
	float _cameraOrbitRadiusMax = 50.0f;
	float _cameraOrbitAngleXZ = 0.0f;
	float _cameraSpeed = 2.0f;

	Terrain m_terrain;
	float m_scaleTerrainHeigtBy;

	void nextTerrain();
	

	HeightMapManager * m_heightMapManager;

	std::chrono::steady_clock::time_point m_timeAtStartOfFrame;
	std::chrono::steady_clock::time_point m_timeAtEndOfFrame;
	float m_secondsToProcessLastFrame;

	UINT m_frameRateCap = 60; // part of an attempt to fix skeletal animaton (the time point didn't appear to change on the animation class)


	UINT _WindowHeight;
	UINT _WindowWidth;

	// Render dimensions - Change here to alter screen resolution
	UINT _renderHeight = 1080;
	UINT _renderWidth = 1920;

	/*
	ID3D11DepthStencilState* DSLessEqual;
	ID3D11RasterizerState* RSCullNone;

	ID3D11BlendState* Transparency;
	ID3D11RasterizerState* CCWcullMode;
	ID3D11RasterizerState* CWcullMode;

	ID3D11RasterizerState* wireframe;
	*/
	bool _wireFrame = false;
	

	Renderer * m_rendererPtr;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	
	// HRESULT InitDevice();
	void Cleanup();
	
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	// HRESULT InitShadersAndInputLayout();
	
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	HRESULT InitAssets();

	Geometry * testTerrainData;


	


public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg);

	void Update();
	void Draw();


	// stuff for the skeletal model
	// SkeletalModel testSM;
	std::vector<ID3D11ShaderResourceView  *> testSMTextures;
	std::vector<std::string> testSMTextureNames;

	// ModelLoader * m_modelLoaderInstancePtr;

	TextureManager * m_textureManager;
	
	MD3ModelInstance * m_md3ModelInst;

};

