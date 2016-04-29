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




#include "Terrain.h"

#include "Structures.h"

#include <chrono>
#include <vector>

// #include "ModelLoader.h"
#include "SkeletalModel.h"
//#include "MD3Model.h"

#include "TextureManager.h"

#include "Renderer.h"

#include "HeightMapManager.h"

class Application
{
	

private:
	

	HINSTANCE               m_hInst;
	HWND                    m_hWnd;
	

	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;

	// ID3D11SamplerState * _pSamplerLinear = nullptr;

	Light m_basicLight;

	// vector<GameObject *> _gameObjects;

	unsigned int m_activeHeightMap;

	Camera * m_camera;
	//float _cameraOrbitRadius = 7.0f;
	//float _cameraOrbitRadiusMin = 2.0f;
	//float _cameraOrbitRadiusMax = 50.0f;
	//float _cameraOrbitAngleXZ = 0.0f;
	//float _cameraSpeed = 2.0f;

	Terrain m_terrain;
	float m_scaleTerrainHeigtBy;

	void nextTerrain();
	

	HeightMapManager * m_heightMapManager;

	std::chrono::steady_clock::time_point m_timeAtStartOfFrame;
	std::chrono::steady_clock::time_point m_timeAtEndOfFrame;
	float m_secondsToProcessLastFrame;

	// UINT m_frameRateCap = 60; // part of an attempt to fix skeletal animaton (the time point didn't appear to change on the animation class)


	UINT m_WindowHeight;
	UINT m_WindowWidth;

	// Render dimensions - Change here to alter screen resolution
	UINT m_renderWidth = 800;
	UINT m_renderHeight = 600;
	
	bool m_wireFrame = false;
	

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

	//Geometry * testTerrainData;


	


public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg);

	void Update();
	void Draw();


	// stuff for the skeletal model
	// SkeletalModel testSM;
	// std::vector<ID3D11ShaderResourceView  *> testSMTextures;
	// std::vector<std::string> testSMTextureNames;

	// ModelLoader * m_modelLoaderInstancePtr;

	TextureManager * m_textureManager;
	
	SkeletalModelInstance * m_skeletalModelInst;

};

