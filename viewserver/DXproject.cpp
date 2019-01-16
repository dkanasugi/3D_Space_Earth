/*
Daichi Kanasugi
CST 325: Graphics Programming
This program is about translating, rotating, and resizing 3d objects and 2d objects with unique lighting
*/

//	-------------------------------------	organization	------------------------------------------
//		includes
//		global var
///			//program entry
//		DirectX global var						
//
//		winmain()
//			{
//			register
//			create win
//			InitDevice()
//				init DirectX (all global obj)	
//			message loop -> winproc
//				render()						
//			}
///			//message functions
//		OnLBD(..)
//			{
//			stuff
//			}
//		OnMMN(..)
///			//message loop function
//		winproc()
//			{
//			switch-case
//				(OnCreate,OnTimer,OnPaint, OnMM,OnLBD, OnLBU,..)
//			}
//  --------------------------------------------------------------------------------------------------




// ==================================================================================================
//                                 !! IGNORE THIS FOR NOW !!! 
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#include "ground.h"
//	defines
#define MAX_LOADSTRING 1000
#define TIMER1 111
//	global variables
HINSTANCE hInst;											//	program number = instance
TCHAR szTitle[MAX_LOADSTRING];								//	name in window title
TCHAR szWindowClass[MAX_LOADSTRING];						//	class name of window
HWND hMain = NULL;											//	number of windows = handle window = hwnd
static char MainWin[] = "MainWin";							//	class name
HBRUSH  hWinCol = CreateSolidBrush(RGB(180, 180, 180));		//	a color

//global variables for the keys
bool upKey = 0;
bool downKey = 0;
bool leftKey = 0;
bool rightKey = 0;
bool k_key = 0;
bool j_key = 0;


HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

//	+++++++++++++++++++++++++++++++++++++++++++++++++
//	DIRECTX stuff follows here
//
//	first the global variables (DirectX Objects)
//
//	second the initdevice(..) function where we load all DX stuff
//
//	third the Render() function, the equivalent to the OnPaint(), but not from Windows, but from DirectX
//
//	lets go:

// GLOBALS:

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;

// device context
ID3D11Device*           g_pd3dDevice = NULL;			// is for initialization and loading things (pictures, models, ...) <- InitDevice()
ID3D11DeviceContext*    g_pImmediateContext = NULL;		// is for render your models w/ pics on the screen					<- Render()
														// page flipping:
IDXGISwapChain*         g_pSwapChain = NULL;
//screen <- thats our render target
ID3D11RenderTargetView* g_pRenderTargetView = NULL;

//depth stencli buffer
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;

//for transparency:
ID3D11BlendState*					g_BlendState;

//Texture Sampler
ID3D11SamplerState*                 g_Sampler = NULL;
//rasterizer states
ID3D11RasterizerState				*rs_CW, *rs_CCW, *rs_NO, *rs_Wire;
//depth state
ID3D11DepthStencilState				*ds_on, *ds_off;
//	structures we need later

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                                !!! IGNORE THIS FOR NOW !!! 
// ==================================================================================================



//how a vertex looks like
ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11InputLayout*		g_pVertexLayoutRGB = NULL;

//our model (array of vertices on the GPU MEM)
ID3D11Buffer*           g_pVertexBufferSphere = NULL; //our plane
ID3D11Buffer*			g_pVertexBufferBoxTextured = NULL;
ID3D11Buffer*			g_pVertexBufferBoxRGB = NULL;
int						g_vertices_sphere;
int						g_vertices_box_textured;
int						g_vertices_box_rgb;

//exchange of data, e.g. sending mouse coordinates to the GPU
ID3D11Buffer*			g_pConstantBuffer11 = NULL;

// function on the GPU what to do with the model exactly
ID3D11VertexShader*     g_pVertexShader = NULL;
ID3D11VertexShader*		g_pVertexShaderRGB = NULL;
ID3D11PixelShader*      g_pPixelShader = NULL;
ID3D11PixelShader*		g_pPixelShaderRGB = NULL;
ID3D11PixelShader*		g_pPixelShaderSky = NULL;


//Texture
ID3D11ShaderResourceView*           g_sky = NULL;
ID3D11ShaderResourceView*           g_earth = NULL;
ID3D11ShaderResourceView*			g_texture = NULL;


XMMATRIX g_world;//model: per object position and rotation and scaling of the object
XMMATRIX g_view;//camera: position and rotation of the camera
XMMATRIX g_projection;//perspective: angle of view, near plane / far plane

camera cam;

struct VS_CONSTANT_BUFFER
{
	// more constant buffer stuff?
	XMFLOAT4 lightposition;
	XMFLOAT4 camposition;
	XMMATRIX world;//model: per object position and rotation and scaling of the object
	XMMATRIX view;//camera: position and rotation of the camera
	XMMATRIX projection;//perspective: angle of view, near plane / far plane
};	
//note: we can only copy chunks of 16 byte to the GPU
VS_CONSTANT_BUFFER VsConstData;		//global object of this structure


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{



	// ==================================================================================================
	//                                 !! IGNORE THIS FOR NOW !!! 
	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hMain, &rc);	//getting the windows size into a RECT structure
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
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

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hMain;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
		}
	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	//                                !!! IGNORE THIS FOR NOW !!! 
	// ==================================================================================================



	// *******************************  FOR TEXTURES ********************************
	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "VShader", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;


	// *******************************  FOR RGB VALUES ********************************
	pVSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "VShaderRGB", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShaderRGB);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = ARRAYSIZE(layout2);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout2, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayoutRGB);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;


	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "PShader", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
		{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
		}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// more pixel shaders to create?
	// Compile the pixel shader
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "PShaderRGB", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderRGB);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Compile the pixel shader
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "PShaderSky", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderSky);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	//load the drawn shape into the vertex buffer:
	D3D11_BUFFER_DESC bd;
	D3D11_SUBRESOURCE_DATA InitData;

	// Cube with textures
	// vertices and memory allocation
	g_vertices_box_textured = 36;
	SimpleVertex* verticesBoxTextured = new SimpleVertex[g_vertices_box_textured];

	// Front face
	verticesBoxTextured[0].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxTextured[0].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[0].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxTextured[1].Pos = XMFLOAT3(1.0f, 1.0f, -1.0f);
	verticesBoxTextured[1].Tex = XMFLOAT2(1.0f, 0.0f);
	verticesBoxTextured[1].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxTextured[2].Pos = XMFLOAT3(1.0f, -1.0f, -1.0f);
	verticesBoxTextured[2].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[2].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);

	verticesBoxTextured[3].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxTextured[3].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[3].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxTextured[4].Pos = XMFLOAT3(1.0f, -1.0f, -1.0f);
	verticesBoxTextured[4].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[4].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxTextured[5].Pos = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	verticesBoxTextured[5].Tex = XMFLOAT2(0.0f, 1.0f);
	verticesBoxTextured[5].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Left Face
	verticesBoxTextured[6].Pos = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	verticesBoxTextured[6].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[6].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxTextured[7].Pos = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	verticesBoxTextured[7].Tex = XMFLOAT2(1.0f, 0.0f);
	verticesBoxTextured[7].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxTextured[8].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxTextured[8].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[8].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	verticesBoxTextured[9].Pos = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	verticesBoxTextured[9].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[9].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxTextured[10].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxTextured[10].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[10].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxTextured[11].Pos = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	verticesBoxTextured[11].Tex = XMFLOAT2(0.0f, 1.0f);
	verticesBoxTextured[11].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	// Back Face
	verticesBoxTextured[12].Pos = XMFLOAT3(1.0f, 1.0f, 1.0f);
	verticesBoxTextured[12].Tex = XMFLOAT2(1.0f, 0.0f);
	verticesBoxTextured[12].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxTextured[13].Pos = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	verticesBoxTextured[13].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[13].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxTextured[14].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxTextured[14].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[14].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);

	verticesBoxTextured[15].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxTextured[15].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[15].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxTextured[16].Pos = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	verticesBoxTextured[16].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[16].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxTextured[17].Pos = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	verticesBoxTextured[17].Tex = XMFLOAT2(0.0f, 1.0f);
	verticesBoxTextured[17].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);

	// Right Face
	verticesBoxTextured[18].Pos = XMFLOAT3(1.0f, 1.0f, 1.0f);
	verticesBoxTextured[18].Tex = XMFLOAT2(1.0f, 0.0f);
	verticesBoxTextured[18].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxTextured[19].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxTextured[19].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[19].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxTextured[20].Pos = XMFLOAT3(1.0f, 1.0f, -1.0f);
	verticesBoxTextured[20].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[20].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);

	verticesBoxTextured[21].Pos = XMFLOAT3(1.0f, 1.0f, -1.0f);
	verticesBoxTextured[21].Tex = XMFLOAT2(1.0f, 1.0f);
	verticesBoxTextured[21].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxTextured[22].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxTextured[22].Tex = XMFLOAT2(0.0f, 0.0f);
	verticesBoxTextured[22].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxTextured[23].Pos = XMFLOAT3(1.0f, -1.0f, -1.0f);
	verticesBoxTextured[23].Tex = XMFLOAT2(0.0f, 1.0f);
	verticesBoxTextured[23].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);

	// Set aside memory for vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * g_vertices_box_textured;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticesBoxTextured;
	// Create vertex buffer
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBufferBoxTextured);
	if (FAILED(hr))
		return hr;


	// Cube with RGB values
	// vertices and memory allocation
	g_vertices_box_rgb = 36;
	SimpleVertexRGB* verticesBoxRGB = new SimpleVertexRGB[g_vertices_box_textured];

	// Front face
	verticesBoxRGB[0].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxRGB[0].Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	verticesBoxRGB[0].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxRGB[1].Pos = XMFLOAT3(1.0f, 1.0f, -1.0f);
	verticesBoxRGB[1].Col = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	verticesBoxRGB[1].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxRGB[2].Pos = XMFLOAT3(1.0f, -1.0f, -1.0f);
	verticesBoxRGB[2].Col = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	verticesBoxRGB[2].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);

	verticesBoxRGB[3].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxRGB[3].Col = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	verticesBoxRGB[3].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxRGB[4].Pos = XMFLOAT3(1.0f, -1.0f, -1.0f);
	verticesBoxRGB[4].Col = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	verticesBoxRGB[4].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);
	verticesBoxRGB[5].Pos = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	verticesBoxRGB[5].Col = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	verticesBoxRGB[5].Norm = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Left Face
	verticesBoxRGB[6].Pos = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	verticesBoxRGB[6].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[6].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxRGB[7].Pos = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	verticesBoxRGB[7].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[7].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxRGB[8].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxRGB[8].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[8].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	verticesBoxRGB[9].Pos = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	verticesBoxRGB[9].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[9].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxRGB[10].Pos = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	verticesBoxRGB[10].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[10].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	verticesBoxRGB[11].Pos = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	verticesBoxRGB[11].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[11].Norm = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	// Back Face
	verticesBoxRGB[12].Pos = XMFLOAT3(1.0f, 1.0f, 1.0f);
	verticesBoxRGB[12].Col = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	verticesBoxRGB[12].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxRGB[13].Pos = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	verticesBoxRGB[13].Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	verticesBoxRGB[13].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxRGB[14].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxRGB[14].Col = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	verticesBoxRGB[14].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);

	verticesBoxRGB[15].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxRGB[15].Col = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	verticesBoxRGB[15].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxRGB[16].Pos = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	verticesBoxRGB[16].Col = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	verticesBoxRGB[16].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);
	verticesBoxRGB[17].Pos = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	verticesBoxRGB[17].Col = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	verticesBoxRGB[17].Norm = XMFLOAT3(0.0f, 0.0f, 1.0f);

	// Right Face
	verticesBoxRGB[18].Pos = XMFLOAT3(1.0f, 1.0f, 1.0f);
	verticesBoxRGB[18].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[18].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxRGB[19].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxRGB[19].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[19].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxRGB[20].Pos = XMFLOAT3(1.0f, 1.0f, -1.0f);
	verticesBoxRGB[20].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[20].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);

	verticesBoxRGB[21].Pos = XMFLOAT3(1.0f, 1.0f, -1.0f);
	verticesBoxRGB[21].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[21].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxRGB[22].Pos = XMFLOAT3(1.0f, -1.0f, 1.0f);
	verticesBoxRGB[22].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[22].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);
	verticesBoxRGB[23].Pos = XMFLOAT3(1.0f, -1.0f, -1.0f);
	verticesBoxRGB[23].Col = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	verticesBoxRGB[23].Norm = XMFLOAT3(1.0f, 0.0f, 0.0f);

	// Set aside memory for vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertexRGB) * g_vertices_box_rgb;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticesBoxRGB;
	// Create vertex buffer
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBufferBoxRGB);
	if (FAILED(hr))
		return hr;

	//load the sphere model into the vertex buffer:
	Load3DS("sphere.3ds", g_pd3dDevice, &g_pVertexBufferSphere, &g_vertices_sphere, FALSE);

	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &VsConstData;
	// Create the buffer.
	hr = g_pd3dDevice->CreateBuffer(&cbDesc, &InitData, &g_pConstantBuffer11);
	if (FAILED(hr))
		return hr;


	//Init texture
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"sky.jpg", NULL, NULL, &g_sky, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"earth.jpg", NULL, NULL, &g_earth, NULL);
	if (FAILED(hr))
		return hr;

	// more textures to load?
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"sky.jpg", NULL, NULL, &g_texture, NULL);
	if (FAILED(hr))
		return hr;



	// ==================================================================================================
	//                                 !! IGNORE THIS FOR NOW !!! 
	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	//SAmpler init
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_Sampler);
	if (FAILED(hr))
		return hr;

	//blendstate init

	//blendstate:
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
	g_pd3dDevice->CreateBlendState(&blendStateDesc, &g_BlendState);

	//matrices

	//world
	g_world = XMMatrixIdentity();

	//view:
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);//camera position
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);//look at
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);// normal vector on at vector (always up)
	g_view = XMMatrixLookAtLH(Eye, At, Up);

	//perspective:
	// Initialize the projection matrix
	g_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 10000.0f);

	//depth buffer:

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;


	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	//rasterizer states:
	//setting the rasterizer:
	D3D11_RASTERIZER_DESC			RS_CW, RS_Wire;

	RS_CW.AntialiasedLineEnable = FALSE;
	RS_CW.CullMode = D3D11_CULL_BACK;
	RS_CW.DepthBias = 0;
	RS_CW.DepthBiasClamp = 0.0f;
	RS_CW.DepthClipEnable = true;
	RS_CW.FillMode = D3D11_FILL_SOLID;
	RS_CW.FrontCounterClockwise = false;
	RS_CW.MultisampleEnable = FALSE;
	RS_CW.ScissorEnable = false;
	RS_CW.SlopeScaledDepthBias = 0.0f;
	//rasterizer state clockwise triangles
	g_pd3dDevice->CreateRasterizerState(&RS_CW, &rs_CW);
	//rasterizer state counterclockwise triangles
	RS_CW.CullMode = D3D11_CULL_FRONT;
	g_pd3dDevice->CreateRasterizerState(&RS_CW, &rs_CCW);
	RS_Wire = RS_CW;
	RS_Wire.CullMode = D3D11_CULL_NONE;
	//rasterizer state seeing both sides of the triangle
	g_pd3dDevice->CreateRasterizerState(&RS_Wire, &rs_NO);
	//rasterizer state wirefrime
	RS_Wire.FillMode = D3D11_FILL_WIREFRAME;
	g_pd3dDevice->CreateRasterizerState(&RS_Wire, &rs_Wire);

	//init depth stats:
	//create the depth stencil states for turning the depth buffer on and of:
	D3D11_DEPTH_STENCIL_DESC		DS_ON, DS_OFF;
	DS_ON.DepthEnable = true;
	DS_ON.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DS_ON.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	DS_ON.StencilEnable = true;
	DS_ON.StencilReadMask = 0xFF;
	DS_ON.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	DS_ON.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DS_ON.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	DS_ON.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DS_ON.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil state
	DS_OFF = DS_ON;
	DS_OFF.DepthEnable = false;
	g_pd3dDevice->CreateDepthStencilState(&DS_ON, &ds_on);
	g_pd3dDevice->CreateDepthStencilState(&DS_OFF, &ds_off);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render function
//--------------------------------------------------------------------------------------

void Render()
{
	// Set increasing angle
	static float angle = 0;
	angle = angle + 0.001;

	// Clear the back buffer 
	float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // red,green,blue,alpha
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	float blendFactor[] = { 0, 0, 0, 0 };
	UINT sampleMask = 0xffffffff;
	g_pImmediateContext->OMSetBlendState(g_BlendState, blendFactor, sampleMask);

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_Sampler);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_Sampler);

	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	//                                !!! IGNORE THIS FOR NOW !!! 
	// ==================================================================================================

	// More constant buffer information?
	VsConstData.lightposition = XMFLOAT4(1000 * sin(angle), 0, 1000 * cos(angle), 0);
	VsConstData.camposition = XMFLOAT4(cam.pos.x, cam.pos.y, cam.pos.z, 0);


	// ************			RENDERING SHAPES WITH TEXTURES:				********************
	// Initial setup:

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Set vertex and pixel shaders
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShaderSky, NULL, 0);

	// Set vertex buffer, setting the model
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBufferSphere, &stride, &offset);

	//change the world matrix
	XMMATRIX V = g_view;
	V = cam.calculate_view(V);

	// ************			render the skysphere:				********************
	XMMATRIX Tv = XMMatrixTranslation(cam.pos.x, cam.pos.y, -cam.pos.z);
	XMMATRIX Rx = XMMatrixRotationX(XM_PIDIV2);
	XMMATRIX S = XMMatrixScaling(0.008, 0.008, 0.008);
	XMMATRIX Msky = S*Rx*Tv; //from left to right
	VsConstData.world = Msky;
	VsConstData.view = V;
	VsConstData.projection = g_projection;
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer11, 0, 0, &VsConstData, 0, 0);	//copying it freshly into the GPU buffer from VsConstData
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the VertexShader
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the PixelShader
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_sky);

	g_pImmediateContext->RSSetState(rs_CCW);//to see it from the inside
	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);//no depth writing
	g_pImmediateContext->Draw(g_vertices_sphere, 0);		//render sky

	g_pImmediateContext->RSSetState(rs_CW);//reset to default
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);//reset to default

	// ************			render the planet:				********************
	XMMATRIX T = XMMatrixTranslation(0, 0, 20);
	Rx = XMMatrixRotationX(XM_PIDIV2);

	XMMATRIX Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(0.01, 0.01, 0.01);
	Msky = S*Rx*Ry*T; //from left to right

	VsConstData.world = Msky;
	VsConstData.view = V;
	VsConstData.projection = g_projection;

	// initialize new kind of pixel shader?
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer11, 0, 0, &VsConstData, 0, 0);	//copying it freshly into the GPU buffer from VsConstData
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the VertexShader
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the PixelShader
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_earth);
	g_pImmediateContext->Draw(g_vertices_sphere, 0);  //render planet

	// ************			render the cube:				********************
	Msky = XMMatrixIdentity();
	T = XMMatrixTranslation(3, 0, 6); //start point of the cube
	Msky = T;

	//displays the objects
	VsConstData.world = Msky;
	VsConstData.view = V;
	VsConstData.projection = g_projection;

	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBufferBoxTextured, &stride, &offset);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer11, 0, 0, &VsConstData, 0, 0);	//copying it freshly into the GPU buffer from VsConstData
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the VertexShader
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the PixelShader
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_texture);
	g_pImmediateContext->Draw(g_vertices_box_textured, 0);  //render planet

	

	// ************			RENDERING SHAPES WITH RGB VALUES:				********************
	// Initial setup:

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayoutRGB);

	// Set vertex and pixel shaders
	g_pImmediateContext->VSSetShader(g_pVertexShaderRGB, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShaderRGB, NULL, 0);

	// Set vertex buffer, setting the model
	stride = sizeof(SimpleVertexRGB);
	offset = 0;

	//global variables to shift the translation points and sizing points
	static float off_SetX = -3;
	static float off_SetZ = 6;
	static float off_setScaling = 1;

	XMMATRIX translate = XMMatrixIdentity();
	XMMATRIX scaling = XMMatrixIdentity();

	//implement the movement with key offsetx += 0.01f;
	//implement the scaling with the keys += 0.001f;
	if (leftKey == 1) {
		off_SetX -= 0.01f; 
	}
	else if (rightKey == 1) {
		off_SetX += 0.01f;
	}
	else if (upKey == 1) {
		off_SetZ += 0.01f;
	}
	else if (downKey == 1) {
		off_SetZ -= 0.01f;
	}
	else if (j_key == 1) {
		off_setScaling += 0.001f;
	}
	else if (k_key == 1) {
		off_setScaling = 1.0f;
	}

	//rendering the environment
	translate = XMMatrixTranslation(off_SetX, 1, off_SetZ);
	scaling = XMMatrixScaling(off_setScaling, 1, 1);
	g_world = translate * scaling;

	Msky = XMMatrixIdentity();
	Msky = translate;

	VsConstData.world = Msky * scaling;
	VsConstData.view = V;
	VsConstData.projection = g_projection;
	
	 

	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBufferBoxRGB, &stride, &offset);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer11, 0, 0, &VsConstData, 0, 0);	//copying it freshly into the GPU buffer from VsConstData
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the VertexShader
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer11);					//setting it enable for the PixelShader
	g_pImmediateContext->Draw(g_vertices_box_rgb, 0);  //render planet

	// Present the information rendered to the back buffer to the front buffer (the screen)
	g_pSwapChain->Present(0, 0);
}



// ==================================================================================================
//                                 !! IGNORE THIS FOR NOW !!! 
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);	//message loop function (containing all switch-case statements

int WINAPI wWinMain(				//	the main function in a window program. program starts here
	HINSTANCE hInstance,			//	here the program gets its own number
	HINSTANCE hPrevInstance,		//	in case this program is called from within another program
	LPTSTR    lpCmdLine,
	int       nCmdShow)
	{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst = hInstance;												//						save in global variable for further use
	MSG msg;

	// Globale Zeichenfolgen initialisieren
	LoadString(hInstance, 103, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, 104, szWindowClass, MAX_LOADSTRING);
	//register Window													<<<<<<<<<<			STEP ONE: REGISTER WINDOW						!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	WNDCLASSEX wcex;												//						=> Filling out struct WNDCLASSEX
	BOOL Result = TRUE;
	wcex.cbSize = sizeof(WNDCLASSEX);								//						size of this struct (don't know why
	wcex.style = CS_HREDRAW | CS_VREDRAW;							//						?
	wcex.lpfnWndProc = (WNDPROC)WndProc;							//						The corresponding Proc File -> Message loop switch-case file
	wcex.cbClsExtra = 0;											//
	wcex.cbWndExtra = 0;											//
	wcex.hInstance = hInstance;										//						The number of the program
	wcex.hIcon = LoadIcon(hInstance, NULL);							//
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);						//
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);				//						Background color
	wcex.lpszMenuName = NULL;										//
	wcex.lpszClassName = L"TutorialWindowClass";									//						Name of the window (must the the same as later when opening the window)
	wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);					//
	Result = (RegisterClassEx(&wcex) != 0);							//						Register this struct in the OS

																	//													STEP TWO: OPENING THE WINDOW with x,y position and xlen, ylen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	RECT rc = { 0, 0, 1920, 1080 };//640,480 ... 1280,720
	hMain = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 2: Rendering a Triangle",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (hMain == 0)	return 0;

	ShowWindow(hMain, nCmdShow);
	UpdateWindow(hMain);


	if (FAILED(InitDevice()))
		{
		return 0;
		}

	// STEP THREE: Going into the infinity message loop							  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Main message loop
	msg = { 0 };
	while (WM_QUIT != msg.message)
		{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		else
			{
			Render();
			}
		}

	return (int)msg.wParam;
	}
///////////////////////////////////////////////////
void redr_win_full(HWND hwnd, bool erase)
	{
	RECT rt;
	GetClientRect(hwnd, &rt);
	InvalidateRect(hwnd, &rt, erase);
	}

///////////////////////////////////
//		This Function is called every time the Left Mouse Button is down
///////////////////////////////////
void OnLBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{

	}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is down
///////////////////////////////////
void OnRBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{

	}
///////////////////////////////////
//		This Function is called every time a character key is pressed
///////////////////////////////////
void OnChar(HWND hwnd, UINT ch, int cRepeat)
	{

	}
///////////////////////////////////
//		This Function is called every time the Left Mouse Button is up
///////////////////////////////////
void OnLBU(HWND hwnd, int x, int y, UINT keyFlags)
	{
	if (x > 250)
		{
		PostQuitMessage(0);
		}

	}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is up
///////////////////////////////////
void OnRBU(HWND hwnd, int x, int y, UINT keyFlags)
	{


	}
///////////////////////////////////
//		This Function is called every time the Mouse Moves
///////////////////////////////////


void OnMM(HWND hwnd, int x, int y, UINT keyFlags)
	{
	if (x < 400) {
		cam.ak = 1;
	}
	else if (x > 400 && x < 1200) {
		cam.ak = 0;
		cam.dk = 0;
	}
	else {
		cam.dk = 1;
	}

	if ((keyFlags & MK_LBUTTON) == MK_LBUTTON)
		{
		}

	if ((keyFlags & MK_RBUTTON) == MK_RBUTTON)
		{
		}
	}
///////////////////////////////////
//		This Function is called once at the begin of a program
///////////////////////////////////
#define TIMER1 1

BOOL OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
	{
	hMain = hwnd;
	return TRUE;
	}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
	HWND hwin;

	switch (id)
		{
			default:
				break;
		}

	}
//************************************************************************
void OnTimer(HWND hwnd, UINT id)
	{

	}
//************************************************************************
///////////////////////////////////
//		This Function is called every time the window has to be painted again
///////////////////////////////////


void OnPaint(HWND hwnd)
	{


	}
//****************************************************************************

//*************************************************************************
void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{

	switch (vk)
		{
			case 81:	//q
				cam.qk = 1;
				break;
			case 69:	//e
				cam.ek = 1;
				break;
			case 87:	//w
				cam.wk = 1;
				break;
			case 83:	//s
				cam.sk = 1;
				break;
			case 65://a
				cam.ak = 1;
				break;
			case 68://d
				cam.dk = 1;
				break;
			case 37: // left
				leftKey = true;
				break;
			case 38: // up
				upKey = true;
				break;
			case 39: // right
				rightKey = true;
				break;
			case 40: // down
				downKey = true;
				break;
			case 74: //j
				j_key = true;
				break;
			case 75: //k
				k_key = true;
				break;

			default:break;

		}
	}

//*************************************************************************
void OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{
	switch (vk)
		{
			case 81:	//q
				cam.qk = 0;
				break;
			case 69:	//e
				cam.ek = 0;
				break;
			case 87:	//w
				cam.wk = 0;
				break;
			case 83:	//s
				cam.sk = 0;
				break;
			case 65://a
				cam.ak = 0;
				break;
			case 68://d
				cam.dk = 0;
				break;
			case 37: // left
				leftKey = false;
				break;
			case 38: // up
				upKey = false;
				break;
			case 39: // right
				rightKey = false;
				break;
			case 40: // down
				downKey = false;
				break;
			case 74: //j
				j_key = false;
				break;
			case 75: //k
				k_key = false;
				break;
			default:break;

		}

	}


//**************************************************************************
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{

	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
		{



		/*
		#define HANDLE_MSG(hwnd, message, fn)    \
		case (message): return HANDLE_##message((hwnd), (wParam), (lParam), (fn))
		*/

		HANDLE_MSG(hwnd, WM_CHAR, OnChar);			// when a key is pressed and its a character
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLBD);	// when pressing the left button
		HANDLE_MSG(hwnd, WM_LBUTTONUP, OnLBU);		// when releasing the left button
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMM);		// when moving the mouse inside your window
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);		// called only once when the window is created
													//HANDLE_MSG(hwnd, WM_PAINT, OnPaint);		// drawing stuff
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);	// not used
		HANDLE_MSG(hwnd, WM_KEYDOWN, OnKeyDown);	// press a keyboard key
		HANDLE_MSG(hwnd, WM_KEYUP, OnKeyUp);		// release a keyboard key
		HANDLE_MSG(hwnd, WM_TIMER, OnTimer);		// timer
			case WM_PAINT:
				hdc = BeginPaint(hMain, &ps);
				EndPaint(hMain, &ps);
				break;
			case WM_ERASEBKGND:
				return (LRESULT)1;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hwnd, message, wParam, lParam);
		}
	return 0;
	}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
		{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
		}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
	}