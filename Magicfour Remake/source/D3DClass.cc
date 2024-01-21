#include "../include/D3DClass.hh"

#include "../third-party/DirectXTex.h"
#include "../include/GameException.hh"

#pragma comment(lib, "third-party/DirectXTex.lib")

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

#include <stdio.h>

D3DClass::D3DClass(int screenWidth, int screenHeight,
	bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	//DXGI : DirectX Graphic Infrastructure

	HRESULT result;
	ComPtr<IDXGIFactory> factory;			// DXGI ��ü���� ���� ����
	ComPtr<IDXGIAdapter> adapter;			// HW/SW ����� ����ȭ�� ��
	// (�ϳ� �̻��� GPU, DAC, ���� �޸𸮸� �����ϴ� ���÷��� ����ý���)
	ComPtr<IDXGIOutput> adapterOutput;		// ����� ���(����� ��)

	unsigned int numModes = 0, numerator = 0, denominator = 1;
	DXGI_ADAPTER_DESC adapterDesc;

	// ���� ����ȭ ���� ����
	m_vsync_enabled = vsync;

	// �׷��� �������̽� ���丮�� �����?
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factory.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create DirectX Graphic Interface Factory.", WFILE, __LINE__);

	// ���丮 ��ü�� ����Ͽ� ù��° �׷��� ī�� �������̽��� ���� �ƴ��͸� ����ϴ�.
	result = factory->EnumAdapters(0, adapter.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create adapter.", WFILE, __LINE__);

	// ����� ��¿� ���� ù ��° �ƴ��� ����
	result = adapter->EnumOutputs(0, adapterOutput.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// DXGI_FORMAT_R8G8B8A8_UNORM ����� ��� ���÷��� ���˿� �´� ����� ������ ���մϴ�.
	result = adapterOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ����� - �׷���ī�� ���� ������ ����Ʈ
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	result = adapterOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ���� ��ħ ���� ��������
	for (unsigned int i = 0; i < numModes; i++)
	{
		auto& curr = displayModeList[i];
		if (curr.Width == screenWidth && curr.Height == screenHeight)
		{
			numerator = curr.RefreshRate.Numerator;
			denominator = curr.RefreshRate.Denominator;
		}
	}

	// �����(�׷���ī��) Description ��������
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// �׷���ī���� �޸� �뷮�� ��������(MB ����)
	m_videoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory) / 1'024 / 1'024;

	// �׷��� ī���� �̸� ��������
	size_t stringLenght;
	int error = wcstombs_s(&stringLenght, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);


	// ������ ��� ���� ����ߴ� ����ü ���� ����
	delete[] displayModeList;
	displayModeList = nullptr;

	// ���� ü�� Description �ʱ�ȭ
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// �� ���� ����, �ʺ�, ����, �����̽� ����(�Ϲ����� 32bit �����̽���) ����
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_vsync_enabled)
	{
		// ������� ���ΰ�ħ ������ ������� �ֻ�����ŭ ����!
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		//������� ���ΰ�ħ ������ ���Ѵ�� ����!
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// ������� �뵵, ������ �ڵ� ����
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;

	// ��Ƽ���ø� ����(�װ� ����)
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Ǯ��ũ�����
	swapChainDesc.Windowed = (fullscreen) ? false : true;

	// ��ĵ������ ���İ� ��ĵ���̴��� �������� ��������(unspecified) �����մϴ�.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	// ��µ� ������ ������� ������ ����
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0; // �߰� �ɼ� ����


	// ����ü��, Direct3D ��ġ, Direct3D ��ġ ���ؽ�Ʈ �����
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(), NULL, m_deviceContext.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3D swap chain.", WFILE, __LINE__);

	// ������� ������ �޾ƿ���
	ID3D11Texture2D* backBufferPtr;
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)) throw GameException(L"Failed to get buffer of swap chain.", WFILE, __LINE__);


	// �� �����ͷ� ���� Ÿ�� �� ����
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, m_renderTargetView.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create render target view.", WFILE, __LINE__);


	backBufferPtr->Release();
	backBufferPtr = nullptr;

	// ���� ������ description �ۼ�
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// �� description���� ���� ���� �ؽ��� ����(ȭ�鿡 �׷����� �� 2D�ϱ� 2D�� ����)
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, 
		m_depthStencilBuffer.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ����-���ٽ� description �ۼ�(� ���� �׽�Ʈ�� ���� ���� �� �ְ�)
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR; // �� �ٸ���?
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// ����-���ٽ� ���� ����
	result = m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);
	// ����-���ٽ� ���� ����(����̽��� �ƴ϶� ���ؽ�Ʈ)
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	// ����-���ٽ� "��" description �ʱ�ȭ
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(),
		&depthStencilViewDesc, m_depthStencilView.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ���ݲ� ���� ����-���ٽ� �並 ��� ������ ���������ο� ���ε�
	m_deviceContext->OMSetRenderTargets(1,
		m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	// ������ȭ�� ����(������ ��� �ȼ��� �׷�������) e.g. ���̾������� or �յ޸�
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = m_device->CreateRasterizerState(&rasterDesc, m_rasterState.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);
	// ������ ���� ����
	m_deviceContext->RSSetState(m_rasterState.Get());

	D3D11_VIEWPORT viewport;
	// ����Ʈ ����(������ ��ü ũ��� �����ϰ�)
	viewport.Width = static_cast<float>(screenWidth);
	viewport.Height = static_cast<float>(screenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = viewport.TopLeftY = 0.0f;
	// ����Ʈ ����
	m_deviceContext->RSSetViewports(1, &viewport);

	// ���� ���(3D -> 2D) ���� �� ����
	float fieldOfView = 3.141592654f / 4.0f;
	float screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	m_worldMatrix = XMMatrixIdentity(); // ���� ��ķ� �ʱ�ȭ

	// ���� ���� ���(UI �� 2D ��� �׸���)
	m_orthoMatrix = XMMatrixOrthographicLH(
		static_cast<float>(screenWidth), static_cast<float>(screenHeight), screenNear, screenDepth);


	// ���� desc���� DepthEnable�� false�� �ٲٱ�!
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc = depthStencilDesc;
	depthDisabledStencilDesc.DepthEnable = false;

	result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, m_depthDisabledStencilState.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE; // ���İ� ����
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // ���İ� ����
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	result = m_device->CreateBlendState(&blendStateDescription,
		m_alphaEnableBlendingState.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ���İ� ������
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	result = m_device->CreateBlendState(&blendStateDescription,
		m_alphaDisableBlendingState.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to initialize DirectXTex library.");
}

D3DClass::~D3DClass()
{
	// ���� ü���� ������ ���� �ݵ��! ������ ���� �ٲٱ�
	// �׷��� ������ ���� �߻�
	if (m_swapChain) m_swapChain->SetFullscreenState(false, NULL);
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4] = { red, green, blue, alpha };

	// ����� ���� ����
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);

	// ���� ���� ���� ����
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void D3DClass::EndScene()
{
	// ���� �������� �Ϸ�� ����, ������� ������ ȭ�鿡 ǥ��
	if (m_vsync_enabled)
	{
		// ���� ��ħ ���� ����
		m_swapChain->Present(1, 0);
	}
	else
	{
		// ������ ������!
		m_swapChain->Present(0, 0);
	}
}

ID3D11Device* D3DClass::GetDevice()
{
	return m_device.Get();
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext.Get();
}

void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}

void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
}

void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}

void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
}

void D3DClass::TurnZBufferOn()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
}


void D3DClass::TurnZBufferOff()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState.Get(), 1);
}

void D3DClass::EnableAlphaBlending()
{
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState.Get(), blendFactor, 0xffffffff);
}

void D3DClass::DisableAlphaBlending()
{
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState.Get(), blendFactor, 0xffffffff);
}
