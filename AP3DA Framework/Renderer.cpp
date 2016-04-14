#include "Renderer.h"

#include "TextureManager.h"
#include <DirectXMath.h>

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

HRESULT Renderer::init(HWND windowHandle)
{
	// move actions performed in: Application::InitDevice to this func
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;

	RECT windowRect;
	GetWindowRect(windowHandle, &windowRect);

	m_width = windowRect.right - windowRect.left;
	m_height = windowRect.bottom - windowRect.top;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	UINT sampleCount = 4;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m_width;
	sd.BufferDesc.Height = m_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = windowHandle;
	sd.SampleDesc.Count = sampleCount;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_swapChainPtr, &m_d3dDevicePtr, &m_featureLevel, &m_d3dDeviceContextPtr);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_swapChainPtr->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	if (FAILED(hr))
		return hr;

	hr = m_d3dDevicePtr->CreateRenderTargetView(pBackBuffer, nullptr, &m_RenderTargetViewPtr);
	pBackBuffer->Release();

	if (FAILED(hr))
		return hr;

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)m_width;
	vp.Height = (FLOAT)m_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_d3dDeviceContextPtr->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	// InitVertexBuffer();
	// InitIndexBuffer();

	// Set primitive topology
	m_d3dDeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_d3dDevicePtr->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	if (FAILED(hr))
		return hr;









	// now deal with the skeletal constant buffers
	D3D11_BUFFER_DESC skelMdlBufDesc;
	ZeroMemory(&skelMdlBufDesc, sizeof(skelMdlBufDesc));
	skelMdlBufDesc.Usage = D3D11_USAGE_DEFAULT;
	skelMdlBufDesc.ByteWidth = sizeof(MD3ModelConstBuffer);
	skelMdlBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	skelMdlBufDesc.CPUAccessFlags = 0;
	hr = m_d3dDevicePtr->CreateBuffer(&skelMdlBufDesc, nullptr, &m_SkeletalModelConstantBuffer);

	D3D11_BUFFER_DESC skelMdlBonesMatBD;
	ZeroMemory(&skelMdlBonesMatBD, sizeof(skelMdlBonesMatBD));
	skelMdlBonesMatBD.Usage = D3D11_USAGE_DEFAULT;
	skelMdlBonesMatBD.ByteWidth = sizeof(MD3ModelBoneMatrixConstBuffer);
	skelMdlBonesMatBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	skelMdlBonesMatBD.CPUAccessFlags = 0;
	hr = m_d3dDevicePtr->CreateBuffer(&skelMdlBonesMatBD, nullptr, &m_SkeletalModelBonesConstantBuffer);




	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = sampleCount;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	m_d3dDevicePtr->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	m_d3dDevicePtr->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	m_d3dDeviceContextPtr->OMSetRenderTargets(1, &m_RenderTargetViewPtr, _depthStencilView);

	// Rasterizer
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = m_d3dDevicePtr->CreateRasterizerState(&cmdesc, &m_RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	m_d3dDevicePtr->CreateDepthStencilState(&dssDesc, &m_DSLessEqual);

	// Blending 

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	m_d3dDevicePtr->CreateBlendState(&blendDesc, &m_Transparency);

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;
	// cmdesc.CullMode = D3D11_CULL_NONE;

	cmdesc.FrontCounterClockwise = true;
	hr = m_d3dDevicePtr->CreateRasterizerState(&cmdesc, &m_CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = m_d3dDevicePtr->CreateRasterizerState(&cmdesc, &m_CWcullMode);

	

	m_d3dDeviceContextPtr->RSSetState(m_CWcullMode);

	cmdesc.FillMode = D3D11_FILL_WIREFRAME;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = m_d3dDevicePtr->CreateRasterizerState(&cmdesc, &m_wireframe);



	bool gotTex = loadTerrainTextures();

	if (gotTex)
	{
		return S_OK;
	}
	return E_FAIL;
	
}

void Renderer::shutdown()
{

}

HRESULT Renderer::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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

			MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Some shader compliation failed", MB_OK);

		}
		if (pErrorBlob) pErrorBlob->Release();

		return hr;
	}

	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT Renderer::InitShadersAndInputLayout()
{
	HRESULT hr;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = m_d3dDevicePtr->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);



	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = m_d3dDevicePtr->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

	if (FAILED(hr))
		return hr;

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT,0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	

	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = m_d3dDevicePtr->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pVertexLayout);


	if (FAILED(hr))
		return hr;

	// Set the input layout
	m_d3dDeviceContextPtr->IASetInputLayout(_pVertexLayout);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_d3dDevicePtr->CreateSamplerState(&sampDesc, &_pSamplerLinear);



	// now the MD3 model shader stuff
	// first the vertex shader, m_skeletalModelVertexShader
	ID3DBlob * ptrSkelMdlVSBlob = nullptr;

	hr = CompileShaderFromFile(L"MD3_SHADER.fx", "VS", "vs_4_0", &ptrSkelMdlVSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"MD3_SHADER.fx file cannot be compiled (VS stage).  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// now create the actual shader
	hr = m_d3dDevicePtr->CreateVertexShader(ptrSkelMdlVSBlob->GetBufferPointer(), ptrSkelMdlVSBlob->GetBufferSize(), nullptr, &m_skeletalModelVertexShader);

	if (FAILED(hr))
	{
		ptrSkelMdlVSBlob->Release();
		return hr;
	}

	// second the pixel shader, m_skeletalModelPixelShader
	// Compile the pixel shader
	ID3DBlob* ptrSkelMdlPSBlob = nullptr;
	hr = CompileShaderFromFile(L"MD3_SHADER.fx", "PS", "ps_4_0", &ptrSkelMdlPSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"MD3_SHADER.fx cannot be compiled (PS stage).  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = m_d3dDevicePtr->CreatePixelShader(ptrSkelMdlPSBlob->GetBufferPointer(), ptrSkelMdlPSBlob->GetBufferSize(), nullptr, &m_skeletalModelPixelShader);
	// ptrSkelMdlPSBlob->Release();

	if (FAILED(hr))
		return hr;


	// third the vertex input layout m_SkeletalModelVertexLayout
	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC skelMeshVertLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT nElementsSkelMeshLayout = ARRAYSIZE(skelMeshVertLayout);

	hr = m_d3dDevicePtr->CreateInputLayout(skelMeshVertLayout, nElementsSkelMeshLayout, ptrSkelMdlVSBlob->GetBufferPointer(),
		ptrSkelMdlVSBlob->GetBufferSize(), &m_SkeletalModelVertexLayout);


	if (FAILED(hr))
		return hr;



	// relese the blobs

	pVSBlob->Release();
	// pPSBlob->Release(); // is already relesed

	ptrSkelMdlVSBlob->Release();
	ptrSkelMdlPSBlob->Release();

	return hr;
}

bool Renderer::loadTerrainTextures()
{
	TextureManager * tmInst = TextureManager::getInstance();
	
	tmInst->init(m_d3dDevicePtr);
	
	tmInst->addTexture("Textures\\lightdirt.dds");

	Texture * tTmpPtr;

	tTmpPtr = tmInst->getTextureWithID("Textures\\lightdirt.dds");

	m_terrainLightDirtTex = tTmpPtr->imageMapPtr;

	tmInst->addTexture("Textures\\grass.dds");
	tTmpPtr = tmInst->getTextureWithID("Textures\\grass.dds");
	m_terrainGrassTex = tTmpPtr->imageMapPtr;

	tmInst->addTexture("Textures\\darkdirt.dds");
	tTmpPtr = tmInst->getTextureWithID("Textures\\darkdirt.dds");
	m_terrainDarkDirtTex = tTmpPtr->imageMapPtr;

	tmInst->addTexture("Textures\\stone.dds");
	tTmpPtr = tmInst->getTextureWithID("Textures\\stone.dds");
	m_terrainStoneTex = tTmpPtr->imageMapPtr;

	tmInst->addTexture("Textures\\snow.dds");
	tTmpPtr = tmInst->getTextureWithID("Textures\\snow.dds");
	m_terrainSnowTex = tTmpPtr->imageMapPtr;
	return true;
}

void Renderer::startDrawing(float * clearColor, Camera * camForFrame, bool wireFrame)
{
	m_d3dDeviceContextPtr->ClearRenderTargetView(m_RenderTargetViewPtr, clearColor);
	m_d3dDeviceContextPtr->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (camForFrame != nullptr)
	{
		m_activeCamera = camForFrame;
	}

	int cullMode = m_activeRenderMode;
	/*
	0 wire frame,
	1 cull front,
	2 cull back,
	3 no cull
	*/
	if (cullMode == 0)
	{
		m_d3dDeviceContextPtr->RSSetState(m_wireframe);
	}
	else if (cullMode == 1)
	{
		m_d3dDeviceContextPtr->RSSetState(m_CWcullMode);
	}
	else if (cullMode == 2)
	{
		m_d3dDeviceContextPtr->RSSetState(m_CCWcullMode);
	}
	else if (cullMode == 3)
	{
		m_d3dDeviceContextPtr->RSSetState(m_RSCullNone);
	}
}

void Renderer::drawGameObjects(std::vector<GameObject *> & toDraw)
{
	m_d3dDeviceContextPtr->IASetInputLayout(_pVertexLayout);

	m_d3dDeviceContextPtr->VSSetShader(_pVertexShader, nullptr, 0);
	m_d3dDeviceContextPtr->PSSetShader(_pPixelShader, nullptr, 0);

	m_d3dDeviceContextPtr->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetSamplers(0, 1, &_pSamplerLinear);



	ConstantBuffer cb;

	XMFLOAT4X4 viewAsFloats = m_activeCamera->GetView();
	XMFLOAT4X4 projectionAsFloats = m_activeCamera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);

	cb.light = basicLight;
	cb.EyePosW = m_activeCamera->GetPosition();

	// Render all scene objects
	for (auto gameObject : toDraw)
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

			m_d3dDeviceContextPtr->PSSetShaderResources(0, 1, &textureRV);


			cb.HasTexture = 1.0f;
		}
		else
		{
			cb.HasTexture = 0.0f;
		}

		cb.drawingMode = 0.0f;
		cb.terrainScaledBy = 0.0f;

		// Update constant buffer
		m_d3dDeviceContextPtr->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		// Draw object
		gameObject->Draw(m_d3dDeviceContextPtr);
	}
}

void Renderer::drawTerrain(Terrain * toDraw)
{
	m_d3dDeviceContextPtr->IASetInputLayout(_pVertexLayout);

	m_d3dDeviceContextPtr->VSSetShader(_pVertexShader, nullptr, 0);
	m_d3dDeviceContextPtr->PSSetShader(_pPixelShader, nullptr, 0);

	m_d3dDeviceContextPtr->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetSamplers(0, 1, &_pSamplerLinear);



	ConstantBuffer cb;

	XMFLOAT4X4 viewAsFloats = m_activeCamera->GetView();
	XMFLOAT4X4 projectionAsFloats = m_activeCamera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);

	cb.light = basicLight;
	cb.EyePosW = m_activeCamera->GetPosition();

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

	cb.surface.AmbientMtrl = terrainMat.ambient;
	cb.surface.DiffuseMtrl = terrainMat.diffuse;
	cb.surface.SpecularMtrl = terrainMat.specular;
	cb.light.SpecularPower = terrainMat.specularPower;

	m_d3dDeviceContextPtr->PSSetShaderResources(1, 1, &m_terrainLightDirtTex);
	m_d3dDeviceContextPtr->PSSetShaderResources(2, 1, &m_terrainGrassTex);
	m_d3dDeviceContextPtr->PSSetShaderResources(3, 1, &m_terrainDarkDirtTex);
	m_d3dDeviceContextPtr->PSSetShaderResources(4, 1, &m_terrainStoneTex);
	m_d3dDeviceContextPtr->PSSetShaderResources(5, 1, &m_terrainSnowTex);

	cb.drawingMode = 1.0f;
	cb.World = XMMatrixTranspose(toDraw->getWorldMat());
	cb.terrainScaledBy = toDraw->getHeightScaledBy();

	m_d3dDeviceContextPtr->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	toDraw->Draw(m_d3dDeviceContextPtr);
}

void Renderer::drawMD5Model(SkeletalModel * toDraw)
{
	ConstantBuffer cb;

	m_d3dDeviceContextPtr->IASetInputLayout(_pVertexLayout);

	m_d3dDeviceContextPtr->VSSetShader(_pVertexShader, nullptr, 0);
	m_d3dDeviceContextPtr->PSSetShader(_pPixelShader, nullptr, 0);

	m_d3dDeviceContextPtr->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetSamplers(0, 1, &_pSamplerLinear);

	XMFLOAT4X4 viewAsFloats = m_activeCamera->GetView();
	XMFLOAT4X4 projectionAsFloats = m_activeCamera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);

	cb.light = basicLight;
	cb.EyePosW = m_activeCamera->GetPosition();

	cb.World = toDraw->getWorldMat();
	cb.World = DirectX::XMMatrixTranspose(cb.World);

	cb.drawingMode = 0.0f;
	cb.terrainScaledBy = 0.0f;
	

	// material
	SurfaceInfo whiteMat;
	float ambCol = 0.2f;
	float difCol = 0.6f;
	float speCol = 1.0f - difCol - ambCol;
	
	whiteMat.AmbientMtrl = DirectX::XMFLOAT4(ambCol, ambCol, ambCol, 1.0f);
	whiteMat.DiffuseMtrl = DirectX::XMFLOAT4(difCol, difCol, difCol, 1.0f);
	whiteMat.SpecularMtrl = DirectX::XMFLOAT4(speCol, speCol, speCol, 1.0f);
	cb.surface = whiteMat;

	for (int i = 0; i < toDraw->nSubsets; i++)
	{
		if (toDraw->m_subsets[i].diffuseMap != nullptr)
		{
			cb.HasTexture = 1.0f;
			m_d3dDeviceContextPtr->PSSetShaderResources(0, 1, &toDraw->m_subsets[i].diffuseMap);
		}
		else
		{
			cb.HasTexture = 0.0f;
		}
		m_d3dDeviceContextPtr->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		toDraw->m_subsets[i].draw(m_d3dDeviceContextPtr);
	}

	

	// toDraw->draw(m_d3dDeviceContextPtr);
	
	//m_d3dDeviceContextPtr->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// testSM.draw(m_d3dDeviceContextPtr);
}


void Renderer::drawMD3Model(MD3ModelInstance * toDraw)
{
	// MD3 stuff below

	// Figure out what need to be done to get, MD3 skeletal model to work
	m_d3dDeviceContextPtr->IASetInputLayout(m_SkeletalModelVertexLayout);

	m_d3dDeviceContextPtr->VSSetShader(m_skeletalModelVertexShader, nullptr, 0);
	m_d3dDeviceContextPtr->PSSetShader(m_skeletalModelPixelShader, nullptr, 0);

	MD3ModelConstBuffer cbForMd3Mesh;
	MD3ModelBoneMatrixConstBuffer cbForMd3Bones;


	m_d3dDeviceContextPtr->VSSetConstantBuffers(0, 1, &m_SkeletalModelConstantBuffer);
	m_d3dDeviceContextPtr->PSSetConstantBuffers(0, 1, &m_SkeletalModelConstantBuffer);
	// now the bone index constant buffers
	m_d3dDeviceContextPtr->VSSetConstantBuffers(1, 1, &m_SkeletalModelBonesConstantBuffer);
	m_d3dDeviceContextPtr->PSSetConstantBuffers(1, 1, &m_SkeletalModelBonesConstantBuffer);


	m_d3dDeviceContextPtr->PSSetSamplers(0, 1, &_pSamplerLinear);



	cbForMd3Mesh.World = XMLoadFloat4x4(&toDraw->WorldMat);
	cbForMd3Mesh.World = XMMatrixTranspose(cbForMd3Mesh.World);
	
	
	XMFLOAT4X4 viewAsFloats = m_activeCamera->GetView();
	XMFLOAT4X4 projectionAsFloats = m_activeCamera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cbForMd3Mesh.View = XMMatrixTranspose(view);
	cbForMd3Mesh.Projection = XMMatrixTranspose(projection);

	cbForMd3Mesh.light = basicLight;
	cbForMd3Mesh.EyePosW = m_activeCamera->GetPosition();


	XMMATRIX worldMat = XMLoadFloat4x4(&toDraw->WorldMat);

	XMVECTOR worldMatrixDeterminate = XMMatrixDeterminant(worldMat);
	cbForMd3Mesh.WorldInverseTranspose = XMMatrixInverse(&worldMatrixDeterminate, worldMat);
	cbForMd3Mesh.WorldInverseTranspose = XMMatrixTranspose(cbForMd3Mesh.WorldInverseTranspose);

	// transpose cbForMd3Mesh.WorldInverseTranspose as seconds time (all previous matrices had were transposed)
	cbForMd3Mesh.WorldInverseTranspose = XMMatrixTranspose(cbForMd3Mesh.WorldInverseTranspose);

	// set the bones constant buffer
	XMMATRIX idMat = XMMatrixIdentity();

	for (unsigned int i = 0; i < 96; i++)
	{
		
		XMMATRIX worldMat = XMMatrixIdentity();
		// transpose it (all previous matrices were transposed)
		worldMat = XMMatrixTranspose(worldMat);
		XMStoreFloat4x4(&cbForMd3Bones.boneMatrices[i], worldMat);
	}

	for (unsigned int i = 0; i < toDraw->finalTransforms.size(); i++)
	{
		// 96 elements cbForMd3Bones.boneMatrices, overkill?
		if (i < 96)
		{
			// cbForMd3Bones.boneMatrices[i] = XMLoadFloat4x4(&m_md3ModelInst->finalTransforms[i]);
			// above if the bone transform matrices are to be stored as XMMATRIX

			// below if they are ment to be stored as XMFLOAT4X4

			// transpose?
			XMMATRIX boneTransform = XMLoadFloat4x4(&toDraw->finalTransforms[i]);
			bool transpose = true;
			if (transpose)
			{
				boneTransform = XMMatrixTranspose(boneTransform);
			}

			XMStoreFloat4x4(&cbForMd3Bones.boneMatrices[i], boneTransform);
		}
	}

	// update the bone constant buffer
	m_d3dDeviceContextPtr->UpdateSubresource(m_SkeletalModelBonesConstantBuffer, 1, nullptr, &cbForMd3Bones, 0, 0);


	for (UINT i = 0; i < toDraw->theModel->m_nSubsets; i++)
	{
		// set the materials, textures, then draw it

		cbForMd3Mesh.surface = toDraw->theModel->m_materials[i];
		// set the diffuse map
		m_d3dDeviceContextPtr->PSSetShaderResources(0, 1, &toDraw->theModel->m_diffuseMaps[i]);

		cbForMd3Mesh.HasTexture = 1.0f;
		
		// Update constant buffer
		m_d3dDeviceContextPtr->UpdateSubresource(m_SkeletalModelConstantBuffer, 0, nullptr, &cbForMd3Mesh, 0, 0);

		// now the actual drawing
		toDraw->theModel->m_modelGeomatry.draw(m_d3dDeviceContextPtr, i);
	}

	
}

void Renderer::altDrawMD3Model(MD3ModelInstance * toDraw)
{
	// just use the original vertex shader (modded to all in one)

	ConstantBuffer cb;
	MD3ModelBoneMatrixConstBuffer boneCB;

	m_d3dDeviceContextPtr->IASetInputLayout(_pVertexLayout);

	m_d3dDeviceContextPtr->VSSetShader(_pVertexShader, nullptr, 0);
	m_d3dDeviceContextPtr->PSSetShader(_pPixelShader, nullptr, 0);

	m_d3dDeviceContextPtr->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	m_d3dDeviceContextPtr->PSSetSamplers(0, 1, &_pSamplerLinear);

	m_d3dDeviceContextPtr->VSSetConstantBuffers(1, 1, &m_SkeletalModelBonesConstantBuffer);
	m_d3dDeviceContextPtr->PSSetConstantBuffers(1, 1, &m_SkeletalModelBonesConstantBuffer);

	// init the bone constant buffer with the boen matrices, start with identity matrices
	for (UINT i = 0; i < 96; i++)
	{
		XMStoreFloat4x4(&boneCB.boneMatrices[i], XMMatrixIdentity());
	}

	for (UINT i = 0; i < toDraw->finalTransforms.size(); i++)
	{
		boneCB.boneMatrices[i] = toDraw->finalTransforms[i];
	}

	m_d3dDeviceContextPtr->UpdateSubresource(m_SkeletalModelBonesConstantBuffer, 1, nullptr, &boneCB, 0, 0);

	// now the main constant buffer

	cb.World = XMLoadFloat4x4(&toDraw->WorldMat);

	XMFLOAT4X4 viewAsFloats = m_activeCamera->GetView();
	XMFLOAT4X4 projectionAsFloats = m_activeCamera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	
	
	XMVECTOR determinant = XMMatrixDeterminant(cb.World);
	
	cb.WorldInverseTranspose = XMMatrixInverse(&determinant,cb.World);
	cb.WorldInverseTranspose = XMMatrixTranspose(cb.WorldInverseTranspose);
	

	cb.light = basicLight;

	cb.EyePosW = m_activeCamera->GetPosition();
	
	cb.drawingMode = 2.0f; // 0.0 for normal rendering (), 1.0  for terrain, 2.0 for the m3d as per the frank luna book
	cb.terrainScaledBy = 0.0f;
	
	for (auto i = 0; i < toDraw->theModel->m_nSubsets; i++)
	{
		// figure out if the subset has a texture
		// set the texture
		SurfaceInfo surface;
		float HasTexture;
		cb.surface = toDraw->theModel->m_materials[i];
		cb.HasTexture = 1.0f;
		m_d3dDeviceContextPtr->PSSetShaderResources(0, 1, &toDraw->theModel->m_diffuseMaps[i]);

		// now update the constant buffer.
		m_d3dDeviceContextPtr->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		toDraw->theModel->m_modelGeomatry.draw(m_d3dDeviceContextPtr, i);
	}

	

}

void Renderer::finshDrawing()
{
	try
	{
		HRESULT drawResults = m_swapChainPtr->Present(0, 0); /// error present if more than three keys on the keyboard are pressed
		if (FAILED(drawResults))
		{
			throw "Failed to Draw Frame";
		}
	}
	catch (...)
	{

	}

}