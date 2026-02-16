#include "core/D3DClass.hh"

#include <DirectXTex.h>
#include "core/GameException.hh"
#include "core/D2DClass.hh"

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

using namespace DirectX;

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
	vsync_enabled_ = vsync;

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
	videoCardMemory_ = static_cast<int>(adapterDesc.DedicatedVideoMemory) / 1'024 / 1'024;

	// �׷��� ī���� �̸� ��������
	size_t stringLenght;
	int error = wcstombs_s(&stringLenght, videoCardDescription_, 128, adapterDesc.Description, 128);
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

	if (vsync_enabled_)
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
		NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, swapChain_.GetAddressOf(),
		device_.GetAddressOf(), NULL, deviceContext_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3D swap chain.", WFILE, __LINE__);

	// ������� ������ �޾ƿ���
	ID3D11Texture2D* backBufferPtr;
	result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)) throw GameException(L"Failed to get buffer of swap chain.", WFILE, __LINE__);
	

	// �� �����ͷ� ���� Ÿ�� �� ����
	result = device_->CreateRenderTargetView(backBufferPtr, NULL, renderTargetView_.GetAddressOf());
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
	result = device_->CreateTexture2D(&depthBufferDesc, NULL, 
		depthStencilBuffer_.GetAddressOf());
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
	result = device_->CreateDepthStencilState(&depthStencilDesc, depthStencilState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);
	// ����-���ٽ� ���� ����(����̽��� �ƴ϶� ���ؽ�Ʈ)
	deviceContext_->OMSetDepthStencilState(depthStencilState_.Get(), 1);

	// ����-���ٽ� "��" description �ʱ�ȭ
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device_->CreateDepthStencilView(depthStencilBuffer_.Get(),
		&depthStencilViewDesc, depthStencilView_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ���ݲ� ���� ����-���ٽ� �並 ��� ������ ���������ο� ���ε�
	deviceContext_->OMSetRenderTargets(1,
		renderTargetView_.GetAddressOf(), depthStencilView_.Get());

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

	result = device_->CreateRasterizerState(&rasterDesc, rasterState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);
	// ������ ���� ����
	deviceContext_->RSSetState(rasterState_.Get());

	D3D11_VIEWPORT viewport;
	// ����Ʈ ����(������ ��ü ũ��� �����ϰ�)
	viewport.Width = static_cast<float>(screenWidth);
	viewport.Height = static_cast<float>(screenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = viewport.TopLeftY = 0.0f;
	// ����Ʈ ����
	deviceContext_->RSSetViewports(1, &viewport);

	// ���� ���(3D -> 2D) ���� �� ����
	float fieldOfView = 3.141592654f / 4.0f;
	float screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	projectionMatrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	worldMatrix_ = XMMatrixIdentity(); // ���� ��ķ� �ʱ�ȭ

	// ���� ���� ���(UI �� 2D ��� �׸���)
	orthoMatrix_ = XMMatrixOrthographicLH(
		static_cast<float>(screenWidth), static_cast<float>(screenHeight), -1, 1);

	// ������ �������� --> depth�� �°� �׸���, depth �� ������ ���� �ʱ�
	D3D11_DEPTH_STENCIL_DESC depthNonWriteStencilDesc = depthStencilDesc;
	depthNonWriteStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	result = device_->CreateDepthStencilState(&depthNonWriteStencilDesc, depthNonWriteStencilState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ���� desc���� DepthEnable�� false�� �ٲٱ�!
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc = depthStencilDesc;
	depthDisabledStencilDesc.DepthEnable = false;

	result = device_->CreateDepthStencilState(&depthDisabledStencilDesc, depthDisabledStencilState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE; // ���İ� ����
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // ���İ� ����
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	result = device_->CreateBlendState(&blendStateDescription,
		alphaEnableBlendingState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// ����Ƽ�� ������
	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	result = device_->CreateBlendState(&blendStateDescription,
		alphaAdditiveBlendingState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);


	// ���İ� ������
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	result = device_->CreateBlendState(&blendStateDescription,
		alphaDisableBlendingState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);



	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to initialize DirectXTex library.");
}

D3DClass::~D3DClass()
{
	// ���� ü���� ������ ���� �ݵ��! ������ ���� �ٲٱ�
	// �׷��� ������ ���� �߻�
	if (swapChain_) swapChain_->SetFullscreenState(false, NULL);
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4] = { red, green, blue, alpha };

	// ����� ���� ����
	deviceContext_->ClearRenderTargetView(renderTargetView_.Get(), color);

	// ���� ���� ���� ����
	deviceContext_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void D3DClass::EndScene()
{
	// ���� �������� �Ϸ�� ����, ������� ������ ȭ�鿡 ǥ��
	if (vsync_enabled_)
	{
		// ���� ��ħ ���� ����
		swapChain_->Present(1, 0);
	}
	else
	{
		// ������ ������!
		swapChain_->Present(0, 0);
	}
}

IDXGISwapChain* D3DClass::GetSwapChain()
{
	return swapChain_.Get();
}

ID3D11Device* D3DClass::GetDevice()
{
	return device_.Get();
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return deviceContext_.Get();
}

void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = projectionMatrix_;
}

void D3DClass::GetWorldMatrix(XMMATRIX& world)
{
	world = worldMatrix_;
}

void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = orthoMatrix_;
}

void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, videoCardDescription_);
	memory = videoCardMemory_;
}

void D3DClass::SetDepthStencilState(DepthStencilMode mode)
{
	switch (mode)
	{
	case DepthStencilMode::Default3D:
		deviceContext_->OMSetDepthStencilState(depthStencilState_.Get(), 1);
		break;
	case DepthStencilMode::Transparent3D:
		deviceContext_->OMSetDepthStencilState(depthNonWriteStencilState_.Get(), 1);
		break;
	case DepthStencilMode::Disabled2D:
		deviceContext_->OMSetDepthStencilState(depthDisabledStencilState_.Get(), 1);
		break;
	}
}

void D3DClass::SetAlphaBlending(BlendStateMode blend_state_mode)
{
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	switch (blend_state_mode)
	{
	case BlendStateMode::AlphaEnable:
		deviceContext_->OMSetBlendState(alphaEnableBlendingState_.Get(), blendFactor, 0xffffffff);
		break;
	case BlendStateMode::AlphaDisable:
		deviceContext_->OMSetBlendState(alphaDisableBlendingState_.Get(), blendFactor, 0xffffffff);
		break;
	case BlendStateMode::AlphaAdditive:
		deviceContext_->OMSetBlendState(alphaAdditiveBlendingState_.Get(), blendFactor, 0xffffffff);
		break;
	}
}