#include "Application.h"

#include "Math.h"

#include "HeightMapGenerator.h"

#include "MoveOnTerrainGO.h"



float calculateTexCoord(float minPoint, float maxPoint, float vertPos) // this function is to be used at load time
{
	float stepSize = 0.001f;
	float currentStep = 0.0f;

	while (currentStep < 1.0f)
	{
		// float stepPos = Math::lerp(minPoint, maxPoint, currentStep);

		float stepPos = (1.0f - currentStep) * minPoint + maxPoint * currentStep;

		if (stepPos >= vertPos)
		{
			return currentStep;
		}
		currentStep += stepSize;
	}
	return 1.0f;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool Application::HandleKeyboard(MSG msg)
{
	XMFLOAT3 cameraPosition = _camera->GetPosition();

	switch (msg.wParam)
	{
	case VK_UP:
		_cameraOrbitRadius = max(_cameraOrbitRadiusMin, _cameraOrbitRadius - (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_DOWN:
		_cameraOrbitRadius = min(_cameraOrbitRadiusMax, _cameraOrbitRadius + (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_RIGHT:
		_cameraOrbitAngleXZ += _cameraSpeed;
		return true;
		break;

	case VK_LEFT:
		_cameraOrbitAngleXZ -= _cameraSpeed;
		return true;
		break;

	case VK_SPACE:
		if (msg.message == WM_KEYDOWN)
			//_wireFrame = !_wireFrame;
			m_rendererPtr->changeRenderMode();
		
		//m_rendererPtr->setWireFrameMode(_wireFrame);
		return true;
		break;
	case VK_TAB:
		if (msg.message == WM_KEYDOWN)
			nextTerrain();
		return true;
		break;
	}

	return false;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	/*
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;

	DSLessEqual = nullptr;
	RSCullNone = nullptr;
	*/


	m_secondsToProcessLastFrame = 0.0f;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}



    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

	m_rendererPtr = new Renderer();

    // if (FAILED(InitDevice()))
	if (FAILED(m_rendererPtr->init(_hWnd)))
	{
        Cleanup();

        return E_FAIL;
    }

	ID3D11Device * _pd3dDevice = m_rendererPtr->getDevicePtr();

	m_textureManager = TextureManager::getInstance();
	m_textureManager->init(_pd3dDevice);

	Texture * tTmpPtr = nullptr;

	m_textureManager->addTexture("Resources\\Crate_COLOR.dds");
	tTmpPtr = m_textureManager->getTextureWithID("Resources\\Crate_COLOR.dds");
	// _pTextureRV = tTmpPtr->imageMapPtr;
	// CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_COLOR.dds", nullptr, &_pTextureRV);
	// now the terrain textures

	/*
	HRESULT texHres= CreateDDSTextureFromFile(_pd3dDevice, L"Textures\\lightdirt.dds", nullptr, &m_terrainLightDirtTex);
	if (FAILED(texHres))
	{
		return texHres;
	}
	*/


	// file loading code
	m_modelLoaderInstancePtr = ModelLoader::getInstance();
	m_modelLoaderInstancePtr->init(_pd3dDevice);


	if (!m_modelLoaderInstancePtr->loadMD5Mesh("bob_lamp_update.md5mesh", testSM, testSMTextures, testSMTextureNames))
	{
		return E_FAIL;
	}

	if (!m_modelLoaderInstancePtr->loadMD5Animation("bob_lamp_update.md5anim", testSM))
	{
		return E_FAIL;
	}

	m_textureManager = TextureManager::getInstance();
	m_textureManager->init(_pd3dDevice);

	testSM.m_animationIndex = 0; // start with the first animation



	m_md3ModelInst = new MD3ModelInstance();
	m_md3ModelInst->theModel = new MD3Model(_pd3dDevice, m_textureManager, "soldier.m3d", L"");
	m_md3ModelInst->timePoint = 0.0f;
	m_md3ModelInst->currentAnimationClipName = "Take1";
	m_md3ModelInst->finalTransforms.resize(m_md3ModelInst->theModel->m_skinnedMeshSkeleton.getBoneCount());

	// init the model's world matrix
	XMMATRIX mdlTrans, mdlScale, mdlRot;
	mdlScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
	mdlRot = XMMatrixRotationY(XM_PI);
	mdlTrans = XMMatrixTranslation(0.0, 2.0, 0.0);

	// XMStoreFloat4x4(&m_md3ModelInst->WorldMat, mdlScale * mdlRot * mdlTrans);

	// starting with Identity matrix
	// XMStoreFloat4x4(&m_md3ModelInst->WorldMat, mdlTrans);

	XMStoreFloat4x4(&m_md3ModelInst->WorldMat, mdlScale*mdlRot*mdlTrans);
	
	
	// Setup Camera
	XMFLOAT3 eye = XMFLOAT3(0.0f, 2.0f, -15.0f);//0.0f, 0.0f, 0.0f);
	XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	//_camera = new Camera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f);
	// _camera = new FlyingCamera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f);

	// declaring the camera pointer later when got the terrain


	// Setup the scene's light
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 20.0f;
	basicLight.LightVecW = XMFLOAT3(0.0f, 1.0f, -1.0f);

	m_rendererPtr->setLight(basicLight);


	// setup the vertex Buffers
	InitVertexBuffer();
	InitIndexBuffer();

	Geometry cubeGeometry;
	cubeGeometry.indexBuffer = _pIndexBuffer;
	cubeGeometry.vertexBuffer = _pVertexBuffer;
	cubeGeometry.numberOfIndices = 36;
	cubeGeometry.vertexBufferOffset = 0;
	cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	/*
	Geometry planeGeometry;
	planeGeometry.indexBuffer = _pPlaneIndexBuffer;
	planeGeometry.vertexBuffer = _pPlaneVertexBuffer;
	planeGeometry.numberOfIndices = 6;
	planeGeometry.vertexBufferOffset = 0;
	planeGeometry.vertexBufferStride = sizeof(SimpleVertex);
	*/


	Material shinyMaterial;
	shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	shinyMaterial.specularPower = 10.0f;

	Material noSpecMaterial;
	noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	noSpecMaterial.specularPower = 0.0f;
	
	GameObject * gameObject;
	
	/*gameObject = new GameObject("Floor", planeGeometry, noSpecMaterial);
	gameObject->SetPosition(0.0f, 0.0f, 0.0f);
	gameObject->SetScale(15.0f, 15.0f, 15.0f);
	gameObject->SetRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);
	gameObject->SetTextureRV(_pTextureRV);
	
	_gameObjects.push_back(gameObject);
	*/


	gameObject = new GameObject("Cube 1", cubeGeometry, shinyMaterial);
	gameObject->SetPosition(0.0f, 2.0f, 0.0f);
	gameObject->SetTextureRV(tTmpPtr->imageMapPtr);

	_gameObjects.push_back(gameObject);

	gameObject = new GameObject("Cube 2", cubeGeometry, shinyMaterial);
	gameObject->SetScale(0.5f, 0.5f, 0.5f);
	gameObject->SetPosition(3.0f, 2.0f, 0.0f);
	gameObject->SetTextureRV(tTmpPtr->imageMapPtr);

	_gameObjects.push_back(gameObject);

	

	

	//delete m_terrainerrainData;

	HeightMap *hm;

	/*
	Advice: deal with the terrain generation algorithms first (Diamond Square, then Perlin Noise)

	then go back to fixing the Terrain::getHeightAt()

	then do the skeletal animation
	*/

	// height map stuff here

	m_heightMapManager = new HeightMapManager();

	if (!m_heightMapManager->loadFromDeffinitionFile("HeightMapsIndex.txt"))
	{
		return E_FAIL;
	}


	HeightMapGenerator hmg;

	// hm = hmg.generateFaultFormation(10, 3);
	// hm = hmg.generateHillCircle(100, 25, 3, 10, 15);



	/*
	hm = new HeightMap();
	hm->setWidth(3);
	hm->setDepth(3);
	std::vector<unsigned char> flatHeight;
	for (int i = 0; i < 3 * 3; i++)
	{
		flatHeight.push_back(0);
	}
	hm->setheightValues(flatHeight);
	*/

	//hm = new HeightMap();
	//hm->loadTerrainFromRAWFile("Textures/terrain.raw");
	// hm.loadTerrainFromBMPFile("Textures/testHM.bmp");
	// hm->loadTerrainFromBMPFile("Textures/_20x20Test.bmp");
	// hm->loadTerrainFromBMPFile("testHM24Bit25x25.bmp");
	//hm->loadTerrainFromBMPFile("tt.bmp");

	// hm = hmg.generateHillCircle(512, 5, 5, 25, 15);
	// hm = hmg.generateDiamonSquare(50, 0.5f, 200);

	// m_terrain.initAsFlatTerrain(5, 5, 5.0f, 5.0f, _pd3dDevice);


	// CHANGE, alter initViaHeightMap to resetViaHeightMap
	// m_terrain.initAsFlatTerrain() // START with this then, reset the shape

	// temp soluton assign first HM encountered in the manager
	std::vector<string> hmIDs = m_heightMapManager->getIDs();

	m_activeHeightMap = 0;

	hm = m_heightMapManager->getHeightMapWithID(hmIDs[m_activeHeightMap]);

	int heightMapWidthDepthVal = 0;

	bool allHeightMapsHaveSameWidthDepthVal = m_heightMapManager->getWidthDepthValForHeightMaps(heightMapWidthDepthVal);

	if (!allHeightMapsHaveSameWidthDepthVal)
	{
		// it doesn't work
		return E_FAIL;
	}

	float cellWidth = 100.0f / (float) heightMapWidthDepthVal;

	m_terrain.initAsFlatTerrain(heightMapWidthDepthVal, heightMapWidthDepthVal, cellWidth, cellWidth, _pd3dDevice);

	ID3D11DeviceContext * dc = m_rendererPtr->getDeviceContextPtr();

	m_scaleTerrainHeigtBy = 10.0f;

	m_terrain.resetShapeViaHeightMap(hm, m_scaleTerrainHeigtBy, _pd3dDevice, dc, heightMapWidthDepthVal, heightMapWidthDepthVal);
	

	// m_terrain.initViaHeightMap(hm, 10.0f, _pd3dDevice,100.0f, 100.0f);
	m_terrain.setPosition(0.0f, 0.0f, 0.0f);
	// m_terrain.setPosition(0.0f, -512.5f, 10.0f);
	
	
	gameObject = new MoveOnTerrainGameObject("MoveingCube1", cubeGeometry, shinyMaterial);
	gameObject->SetScale(0.5f, 0.5f, 0.5f);
	gameObject->SetPosition(0.0f, 0.0f, 0.0f);
	gameObject->SetTextureRV(tTmpPtr->imageMapPtr);

	MoveOnTerrainGameObject * tmpmotgp = (MoveOnTerrainGameObject *)gameObject;
	tmpmotgp->setMoveOn(&m_terrain);

	_gameObjects.push_back(gameObject);
	
	//MoveOnTerrainGameObject * goPtr = new MoveOnTerrainGameObject("MoveingCube1", cubeGeometry, shinyMaterial);
	//goPtr->setMoveOn(&m_terrain);

	// _gameObjects.push_back(goPtr);


	//float additionalCamHeight = 5.0f;
	float additionalCamHeight = 0.0f;

	// _camera = new FirstPersonCamera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f, &m_terrain, additionalCamHeight);

	_camera = new FlyingCamera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f);
	// _camera = new Camera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f);




	return S_OK;
}

void Application::nextTerrain()
{
	m_activeHeightMap++;
	std::vector<std::string> heightMapIDs = m_heightMapManager->getIDs();
	if (m_activeHeightMap >= heightMapIDs.size())
	{
		m_activeHeightMap = 0;
	}
	HeightMap * hm = m_heightMapManager->getHeightMapWithID(heightMapIDs[m_activeHeightMap]);
	int widthDepthVal = 1;
	bool heghtMapSameWidthDepth = m_heightMapManager->getWidthDepthValForHeightMaps(widthDepthVal);

	ID3D11Device * d3dD = m_rendererPtr->getDevicePtr();
	ID3D11DeviceContext * d3dDC = m_rendererPtr->getDeviceContextPtr();

	if (heghtMapSameWidthDepth)
	{
		m_terrain.resetShapeViaHeightMap(hm, m_scaleTerrainHeigtBy, d3dD, d3dDC, widthDepthVal, widthDepthVal);
	}
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

	ID3D11Device * _pd3dDevice = m_rendererPtr->getDevicePtr();

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

	

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

	ID3D11Device * _pd3dDevice = m_rendererPtr->getDevicePtr();

    // Create index buffer
    unsigned int indices[] =
    {
		3, 1, 0,
		2, 1, 3,

		6, 4, 5,
		7, 4, 6,

		11, 9, 8,
		10, 9, 11,

		14, 12, 13,
		15, 12, 14,

		19, 17, 16,
		18, 17, 19,

		22, 20, 21,
		23, 20, 22
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(unsigned int) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitAssets()
{
	// put code for loading: textures & models in this func

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
	RECT rc = { 0, 0, _renderWidth, _renderHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Semester 2 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
		{
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			
			MessageBoxA(_hWnd, (char*)pErrorBlob->GetBufferPointer(), "Some shader compliation failed", MB_OK);
			
		}
        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

void Application::Cleanup()
{
	if (m_rendererPtr != nullptr)
	{
		m_rendererPtr->shutdown();
		delete m_rendererPtr;
	}

	if (_camera)
	{
		delete _camera;
		_camera = nullptr;
	}

	for (auto gameObject : _gameObjects)
	{
		if (gameObject)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}

	m_textureManager->shutdown();
	delete m_textureManager;
}

void Application::Update()
{
	m_timeAtStartOfFrame = std::chrono::steady_clock::now();

    
	// Update camera
	/*
	float angleAroundZ = XMConvertToRadians(_cameraOrbitAngleXZ);

	float x = _cameraOrbitRadius * cos(angleAroundZ);
	float z = _cameraOrbitRadius * sin(angleAroundZ);

	XMFLOAT3 cameraPos = _camera->GetPosition();
	cameraPos.x = x;
	cameraPos.z = z;
	
	_camera->SetPosition(cameraPos);
	*/

	_camera->updateLogic(m_secondsToProcessLastFrame);
	_camera->Update();

	// Update objects

	for (auto gameObject : _gameObjects)
	{
		gameObject->Update(m_secondsToProcessLastFrame);
	}

	
	m_terrain.Update(0.0f);

	// test the terrain heightAt functions
	float x = 0.0;
	float z = 0.0;
	if (m_terrain.isPositionOnTerrain(x, z))
	{
		float terrainHeigth = m_terrain.getHeightAtLocation(x, z);
	}

	ID3D11DeviceContext * _pImmediateContext = m_rendererPtr->getDeviceContextPtr();

	// update the sleletal model
	
	testSM.update(m_secondsToProcessLastFrame, _pImmediateContext);
	XMStoreFloat4x4(&testSM.m_worldMat, XMMatrixTranslation(0.0f,10.0f,0.0f));


	m_md3ModelInst->update(m_secondsToProcessLastFrame);
}

void Application::Draw()
{
    //
    // Clear buffers
    //
	
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha
	
	m_rendererPtr->startDrawing(ClearColor, _camera, _wireFrame);
	// drawing methords here
	m_rendererPtr->drawGameObjects(_gameObjects);
	m_rendererPtr->drawTerrain(&m_terrain);

	m_rendererPtr->drawMD5Model(&testSM);
	m_rendererPtr->drawMD3Model(m_md3ModelInst);

	m_rendererPtr->finshDrawing();



	/*
	_pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //
    // Setup buffers and render scene
    //

	if (_wireFrame)
	{
		_pImmediateContext->RSSetState(wireframe);
	}
	else
	{
		_pImmediateContext->RSSetState(CWcullMode);
	}



	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);



    ConstantBuffer cb;

	XMFLOAT4X4 viewAsFloats = _camera->GetView();
	XMFLOAT4X4 projectionAsFloats = _camera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	
	cb.light = basicLight;
	cb.EyePosW = _camera->GetPosition();

	// Render all scene objects
	for (auto gameObject : _gameObjects)
	{
		// Get render material
		Material material = gameObject->GetMaterial();

		// Copy material to shader
		cb.surface.AmbientMtrl = material.ambient;
		cb.surface.DiffuseMtrl = material.diffuse;
		cb.surface.SpecularMtrl = material.specular;

		// Set world matrix
		cb.World = XMMatrixTranspose(gameObject->GetWorldMatrix());

		// Set texture
		if (gameObject->HasTexture())
		{
			ID3D11ShaderResourceView * textureRV = gameObject->GetTextureRV();
			// set the texture here

			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);


			cb.HasTexture = 1.0f;
		}
		else
		{
			cb.HasTexture = 0.0f;
		}

		cb.drawingTerrain = 0.0f;
		cb.terrainScaledBy = 0.0f;

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		// Draw object
		gameObject->Draw(_pImmediateContext);
	}

	/*
	// render the terrain
	Material terrainMat;
	terrainMat.ambient.x = 0.2f;
	terrainMat.ambient.y = 0.2f;
	terrainMat.ambient.z = 0.2f;
	terrainMat.ambient.w = 0.2f;

	terrainMat.diffuse.x = 0.6f;
	terrainMat.diffuse.y = 0.6f;
	terrainMat.diffuse.z = 0.6f;
	terrainMat.diffuse.w = 0.6f;

	terrainMat.specular.x = 0.1f;
	terrainMat.specular.y = 0.1f;
	terrainMat.specular.z = 0.1f;
	terrainMat.specular.w = 0.1f;
	
	terrainMat.specularPower = 20.0f;

	//_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	*/



	//cb.World = XMMatrixTranspose(m_terrain.getWorldMat());
	//cb.drawingTerrain = 1.0f;
	//cb.terrainScaledBy = m_terrain.getHeightScaledBy();

	//// _pImmediateContext->PSSetShaderResources(0, 1, &m_);
	///*
	//m_terrainLightDirtTex : Texture2D terrainLightDirtTex : register(t1);
	//m_terrainGrassTex : Texture2D terrainGrassTex : register(t2);
	//m_terrainDarkDirtTex : Texture2D terrainDarkDirtTex : register(t3);
	//m_terrainStoneTex : Texture2D terrainStoneTex : register(t4);
	//m_terrainSnowTex : Texture2D terrainSnowTex : register(t5);
	//
	//*/

	//_pImmediateContext->PSSetShaderResources(1, 1, &m_terrainLightDirtTex);
	//_pImmediateContext->PSSetShaderResources(2, 1, &m_terrainGrassTex);
	//_pImmediateContext->PSSetShaderResources(3, 1, &m_terrainDarkDirtTex);
	//_pImmediateContext->PSSetShaderResources(4, 1, &m_terrainStoneTex);
	//_pImmediateContext->PSSetShaderResources(5, 1, &m_terrainSnowTex);


	//_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	////m_terrain.Draw(_pImmediateContext);
	//
	//// now try the skeletal model
	//// update the CB
	//cb.drawingTerrain = 0.0f;
	//cb.HasTexture = 0.0f;
	//cb.terrainScaledBy = 0.0f;
	//cb.World = XMMatrixTranspose(testSM.getWorldMat());
	//_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// testSM.draw(_pImmediateContext);






	// MD3 stuff below









	/*
	// Figure out what need to be done to get, MD3 skeletal model to work 
	_pImmediateContext->IASetInputLayout(m_SkeletalModelVertexLayout);

	_pImmediateContext->VSSetShader(m_skeletalModelVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(m_skeletalModelPixelShader, nullptr, 0);

	MD3ModelConstBuffer cbForMd3Mesh;
	MD3ModelBoneMatrixConstBuffer cbForMd3Bones;


	_pImmediateContext->VSSetConstantBuffers(0, 1, &m_SkeletalModelConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &m_SkeletalModelConstantBuffer);
	// now the bone index constant buffers
	_pImmediateContext->VSSetConstantBuffers(1, 1, &m_SkeletalModelBonesConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(1, 1, &m_SkeletalModelBonesConstantBuffer);


	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

	
	
	cbForMd3Mesh.World = XMLoadFloat4x4(&m_md3ModelInst->WorldMat);
	cbForMd3Mesh.World = XMMatrixTranspose(cbForMd3Mesh.World);
	cbForMd3Mesh.View = cb.View;
	cbForMd3Mesh.Projection = cb.Projection;
	
	cbForMd3Mesh.light = cb.light;
	cbForMd3Mesh.EyePosW = cb.EyePosW;
	

	XMVECTOR worldMatrixDeterminate = XMMatrixDeterminant(cbForMd3Mesh.World);
	cbForMd3Mesh.WorldInverseTranspose = XMMatrixInverse(&worldMatrixDeterminate, cbForMd3Mesh.World);
	cbForMd3Mesh.WorldInverseTranspose = XMMatrixTranspose(cbForMd3Mesh.WorldInverseTranspose);

	// set the bones constant buffer
	XMMATRIX idMat = XMMatrixIdentity();

	for (unsigned int i = 0; i < 96; i++)
	{
		// cbForMd3Bones.boneMatrices[i] = idMat; // code if ment to pass an XMMATRIX
		XMStoreFloat4x4(&cbForMd3Bones.boneMatrices[i], XMMatrixIdentity());
	}

	for (unsigned int i = 0; i < m_md3ModelInst->finalTransforms.size(); i++)
	{
		// 96 elements cbForMd3Bones.boneMatrices
		if (i < 96)
		{
			// cbForMd3Bones.boneMatrices[i] = XMLoadFloat4x4(&m_md3ModelInst->finalTransforms[i]);
			// above if the bone transform matrices are to be stored as XMMATRIX

			// below if they are ment to be stored as XMFLOAT4X4
			cbForMd3Bones.boneMatrices[i] = m_md3ModelInst->finalTransforms[i];
		}
	}
	
	// update the bone constant buffer
	_pImmediateContext->UpdateSubresource(m_SkeletalModelBonesConstantBuffer, 1, nullptr, &cbForMd3Bones, 0, 0);


	for (UINT i = 0; i < m_md3ModelInst->theModel->m_nSubsets; i++)
	{
		// set the materials, textures, then draw it
		
		cbForMd3Mesh.surface = m_md3ModelInst->theModel->m_materials[i];
		// set the diffuse map
		_pImmediateContext->PSSetShaderResources(0, 1, &m_md3ModelInst->theModel->m_diffuseMaps[i]);

		cbForMd3Mesh.HasTexture = 1.0f;
		// Update constant buffer
		_pImmediateContext->UpdateSubresource(m_SkeletalModelConstantBuffer, 0, nullptr, &cbForMd3Mesh, 0, 0);

		// now the actual drawing
		m_md3ModelInst->theModel->m_modelGeomatry.draw(_pImmediateContext, i);
	}
	
	*/
	
	




    //
    // Present our back buffer to our front buffer
    //
    // _pSwapChain->Present(0, 0);
	
	using namespace std::chrono;

	m_timeAtEndOfFrame = steady_clock::now();
	steady_clock::duration timeTaken = m_timeAtEndOfFrame - m_timeAtStartOfFrame;

	std::chrono::milliseconds ms = duration_cast<milliseconds>(timeTaken);
	m_secondsToProcessLastFrame = ms.count();
	m_secondsToProcessLastFrame / 1000.0f;// convert back to seconds
}