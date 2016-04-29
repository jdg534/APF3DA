#include "Application.h"

#include "Math.h"

#include "HeightMapGenerator.h"

#include <cfloat> // for rounding 0.0 seconds to smallest
// #include <thread> // for std::thread::sleep()

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
	XMFLOAT3 cameraPosition = m_camera->GetPosition();

	switch (msg.wParam)
	{
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
	m_hInst = nullptr;
	m_hWnd = nullptr;
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


	m_secondsToProcessLastFrame = 0.2f; // avoide triggering a false positive on an asertion
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
    GetClientRect(m_hWnd, &rc);
    m_WindowWidth = rc.right - rc.left;
    m_WindowHeight = rc.bottom - rc.top;

	m_rendererPtr = new Renderer();

    // if (FAILED(InitDevice()))
	if (FAILED(m_rendererPtr->init(m_hWnd)))
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
	// m_modelLoaderInstancePtr = ModelLoader::getInstance();
	// m_modelLoaderInstancePtr->init(_pd3dDevice);

	/*
	if (!m_modelLoaderInstancePtr->loadMD5Mesh("bob_lamp_update.md5mesh", testSM, testSMTextures, testSMTextureNames))
	{
		return E_FAIL;
	}

	if (!m_modelLoaderInstancePtr->loadMD5Animation("bob_lamp_update.md5anim", testSM))
	{
		return E_FAIL;
	}
	*/


	m_textureManager = TextureManager::getInstance();
	m_textureManager->init(_pd3dDevice);

	// testSM.m_animationIndex = 0; // start with the first animation

	

	m_skeletalModelInst = new SkeletalModelInstance();
	// m_skeletalModelInst->theModel = new SkeletalModel(_pd3dDevice, m_textureManager, "soldier.m3d", L"");
	m_skeletalModelInst->theModel = new SkeletalModel(_pd3dDevice, m_textureManager, "soldier.m3d", L"");
	m_skeletalModelInst->timePoint = 0.0f;
	m_skeletalModelInst->currentAnimationClipName = "Take1";
	m_skeletalModelInst->finalTransforms.resize(m_skeletalModelInst->theModel->m_skinnedMeshSkeleton.getBoneCount());

	// init the model's world matrix
	XMMATRIX mdlTrans, mdlScale, mdlRot;
	mdlScale = XMMatrixScaling(0.5f, 0.5f, -0.5f);
	//mdlScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	mdlRot = XMMatrixRotationY(XM_PI);
	mdlTrans = XMMatrixTranslation(0.0, 2.0, 0.0);

	// XMStoreFloat4x4(&m_skeletalModelInst->WorldMat, mdlScale * mdlRot * mdlTrans);

	// starting with Identity matrix
	// XMStoreFloat4x4(&m_skeletalModelInst->WorldMat, mdlTrans);
	
	// XMStoreFloat4x4(&m_skeletalModelInst->WorldMat, XMMatrixIdentity());
	// XMStoreFloat4x4(&m_skeletalModelInst->WorldMat, mdlScale);
	XMStoreFloat4x4(&m_skeletalModelInst->WorldMat, mdlScale*mdlRot*mdlTrans);
	
	
	// Setup Camera
	XMFLOAT3 eye = XMFLOAT3(0.0f, 2.0f, -15.0f);//0.0f, 0.0f, 0.0f);
	XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	//_camera = new Camera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f);
	// _camera = new FlyingCamera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f);

	// declaring the camera pointer later when got the terrain


	// Setup the scene's light
	m_basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_basicLight.SpecularPower = 20.0f;
	m_basicLight.LightVecW = XMFLOAT3(0.0f, 1.0f, -1.0f);

	m_rendererPtr->setLight(m_basicLight);


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
	std::vector<std::string> hmIDs = m_heightMapManager->getIDs();

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

	m_scaleTerrainHeigtBy = 100.0f;

	m_terrain.resetShapeViaHeightMap(hm, m_scaleTerrainHeigtBy, _pd3dDevice, dc, heightMapWidthDepthVal, heightMapWidthDepthVal);
	

	// m_terrain.initViaHeightMap(hm, 10.0f, _pd3dDevice,100.0f, 100.0f);
	m_terrain.setPosition(0.0f, 0.0f, 0.0f);
	// m_terrain.setPosition(0.0f, -512.5f, 10.0f);
	
	

	//float additionalCamHeight = 5.0f;
	float additionalCamHeight = 0.0f;

	// _camera = new FirstPersonCamera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f, &m_terrain, additionalCamHeight);

	m_camera = new FlyingCamera(eye, at, up, (float)m_renderWidth, (float)m_renderHeight, 0.01f, 1000.0f);
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
    m_hInst = hInstance;
	RECT rc = { 0, 0, m_renderWidth, m_renderHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    m_hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Semester 2 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!m_hWnd)
		return E_FAIL;

    ShowWindow(m_hWnd, nCmdShow);

    return S_OK;
}

void Application::Cleanup()
{
	if (m_rendererPtr != nullptr)
	{
		m_rendererPtr->shutdown();
		delete m_rendererPtr;
	}

	if (m_camera)
	{
		delete m_camera;
		m_camera = nullptr;
	}

	/*
	for (auto gameObject : _gameObjects)
	{
		if (gameObject)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}
	*/

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

	m_camera->updateLogic(m_secondsToProcessLastFrame);
	m_camera->Update();

	// Update objects


	
	m_terrain.Update(m_secondsToProcessLastFrame);

	// test the terrain heightAt functions
	float x = 0.0;
	float z = 0.0;
	if (m_terrain.isPositionOnTerrain(x, z))
	{
		float terrainHeigth = m_terrain.getHeightAtLocation(x, z);
	}

	ID3D11DeviceContext * _pImmediateContext = m_rendererPtr->getDeviceContextPtr();

	// update the sleletal model
	
	//testSM.update(m_secondsToProcessLastFrame, _pImmediateContext);
	// XMStoreFloat4x4(&testSM.m_worldMat, XMMatrixTranslation(0.0f,10.0f,0.0f));


	m_skeletalModelInst->update(m_secondsToProcessLastFrame);
}

void Application::Draw()
{
    //
    // Clear buffers
    //
	
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha
	
	m_rendererPtr->startDrawing(ClearColor, m_camera, m_wireFrame);
	// drawing methords here
	// m_rendererPtr->drawGameObjects(_gameObjects);
	m_rendererPtr->drawTerrain(&m_terrain);

	// m_rendererPtr->drawMD5Model(&testSM);
	// change to the alt version
	// m_rendererPtr->drawMD3Model(m_skeletalModelInst);
	m_rendererPtr->drawSkeletalModel(m_skeletalModelInst);
	m_rendererPtr->finshDrawing();



	using namespace std::chrono;

	m_timeAtEndOfFrame = steady_clock::now();
	steady_clock::duration timeTaken = m_timeAtEndOfFrame - m_timeAtStartOfFrame;

	std::chrono::milliseconds ms = duration_cast<milliseconds>(timeTaken); // need higher percission
	// std::chrono::microseconds microSec = duration_cast<microseconds>(timeTaken); // need higher percission
	// std::chrono::nanoseconds ns = duration_cast<microseconds>(timeTaken); // need higher percission?
	
	//double microSecAsDouble = (double)microSec.count();
	// double inSeconds = microSecAsDouble / 1000000.0;
	// double nsAsDbl = (double)ns.count();
	//double inSeconds = nsAsDbl / 1000000000.0;

	// can't use nano seconds, the controls became unresponsive

	double msFlt = ms.count();
	double inSeconds = msFlt / 1000.0;

	m_secondsToProcessLastFrame = inSeconds;

	// assert(m_secondsToProcessLastFrame > 0.0f);

	if (m_secondsToProcessLastFrame == 0.0f)
	{
		// set m_secondsToProcessLastFrame to be smallest non zero value that a float can represent

		m_secondsToProcessLastFrame = FLT_MIN;
	}

	//m_secondsToProcessLastFrame = ms.count(); // currently in Milliseconds
	//m_secondsToProcessLastFrame / 1000.0f;// convert back to seconds
}