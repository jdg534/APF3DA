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


#include <vector>

#include "GameObject.h"

#include "Terrain.h"

#include "Structures.h"

#include <chrono>

#include "ModelLoader.h"
#include "SkeletalModel.h"

class Application
{
	

private:
	

	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;

	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;

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


	ID3D11SamplerState * _pSamplerLinear = nullptr;

	Light basicLight;

	vector<GameObject *> _gameObjects;

	

	Camera * _camera;
	float _cameraOrbitRadius = 7.0f;
	float _cameraOrbitRadiusMin = 2.0f;
	float _cameraOrbitRadiusMax = 50.0f;
	float _cameraOrbitAngleXZ = 0.0f;
	float _cameraSpeed = 2.0f;

	Terrain testT;

	std::chrono::steady_clock::time_point m_timeAtStartOfFrame;
	std::chrono::steady_clock::time_point m_timeAtEndOfFrame;
	float m_secondsToProcessLastFrame;


	UINT _WindowHeight;
	UINT _WindowWidth;

	// Render dimensions - Change here to alter screen resolution
	UINT _renderHeight = 1080;
	UINT _renderWidth = 1920;

	ID3D11DepthStencilState* DSLessEqual;
	ID3D11RasterizerState* RSCullNone;

	ID3D11BlendState* Transparency;
	ID3D11RasterizerState* CCWcullMode;
	ID3D11RasterizerState* CWcullMode;

	ID3D11RasterizerState* wireframe;
	bool _wireFrame = false;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	Geometry * testTerrainData;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg);

	void Update();
	void Draw();


	// stuff for the skeletal model
	SkeletalModel testSM;
	std::vector<ID3D11ShaderResourceView  *> testSMTextures;
	std::vector<std::string> testSMTextureNames;

	ModelLoader * m_modelLoaderInstancePtr;

	
};

