#include "core/D3DClass.hh"

#include "../third-party/DirectXTex.h"
#include "core/GameException.hh"
#include "core/D2DClass.hh"

#pragma comment(lib, "third-party/DirectXTex.lib")

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

using namespace DirectX;

D3DClass::D3DClass(int screenWidth, int screenHeight,
	bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	//DXGI : DirectX Graphic Infrastructure

	HRESULT result;
	ComPtr<IDXGIFactory> factory;			// DXGI 객체들을 찍어내는 공장
	ComPtr<IDXGIAdapter> adapter;			// HW/SW 기능을 형상화한 것
	// (하나 이상의 GPU, DAC, 비디오 메모리를 포함하는 디스플레이 서브시스템)
	ComPtr<IDXGIOutput> adapterOutput;		// 어답터 출력(모니터 등)

	unsigned int numModes = 0, numerator = 0, denominator = 1;
	DXGI_ADAPTER_DESC adapterDesc;

	// 수직 동기화 설정 저장
	m_vsync_enabled = vsync;

	// 그래픽 인터페이스 팩토리를 만든다?
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factory.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create DirectX Graphic Interface Factory.", WFILE, __LINE__);

	// 팩토리 객체를 사용하여 첫번째 그래픽 카드 인터페이스에 대한 아답터를 만듭니다.
	result = factory->EnumAdapters(0, adapter.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create adapter.", WFILE, __LINE__);

	// 모니터 출력에 대한 첫 번째 아답터 나열
	result = adapter->EnumOutputs(0, adapterOutput.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// DXGI_FORMAT_R8G8B8A8_UNORM 모니터 출력 디스플레이 포맷에 맞는 모드의 개수를 구합니다.
	result = adapterOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// 모니터 - 그래픽카드 조합 저장할 리스트
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	result = adapterOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// 새로 고침 비율 가져오기
	for (unsigned int i = 0; i < numModes; i++)
	{
		auto& curr = displayModeList[i];
		if (curr.Width == screenWidth && curr.Height == screenHeight)
		{
			numerator = curr.RefreshRate.Numerator;
			denominator = curr.RefreshRate.Denominator;
		}
	}

	// 어댑터(그래픽카드) Description 가져오기
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// 그래픽카드의 메모리 용량을 가져오기(MB 단위)
	videoCardMemory_ = static_cast<int>(adapterDesc.DedicatedVideoMemory) / 1'024 / 1'024;

	// 그래픽 카드의 이름 가져오기
	size_t stringLenght;
	int error = wcstombs_s(&stringLenght, videoCardDescription_, 128, adapterDesc.Description, 128);
	if (error != 0) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);


	// 정보를 얻기 위해 사용했던 구조체 정보 해지
	delete[] displayModeList;
	displayModeList = nullptr;

	// 스왑 체인 Description 초기화
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// 백 버퍼 개수, 너비, 높이, 서페이스 포맷(일반적인 32bit 서페이스로) 설정
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_vsync_enabled)
	{
		// 백버퍼의 새로고침 비율을 모니터의 주사율만큼 설정!
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		//백버퍼의 새로고침 비율을 무한대로 설정!
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// 백버퍼의 용도, 윈도우 핸들 설정
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;

	// 멀티샘플링 끄기(그게 뭔데)
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// 풀스크린모드
	swapChainDesc.Windowed = (fullscreen) ? false : true;

	// 스캔라인의 정렬과 스캔라이닝을 지정되지 않음으로(unspecified) 설정합니다.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	// 출력된 이후의 백버퍼의 내용을 버림
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0; // 추가 옵션 없션


	// 스왑체인, Direct3D 장치, Direct3D 장치 컨텍스트 만들기
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, swapChain_.GetAddressOf(),
		device_.GetAddressOf(), NULL, deviceContext_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3D swap chain.", WFILE, __LINE__);

	// 백버퍼의 포인터 받아오기
	ID3D11Texture2D* backBufferPtr;
	result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)) throw GameException(L"Failed to get buffer of swap chain.", WFILE, __LINE__);
	

	// 그 포인터로 렌더 타겟 뷰 생성
	result = device_->CreateRenderTargetView(backBufferPtr, NULL, renderTargetView_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create render target view.", WFILE, __LINE__);


	backBufferPtr->Release();
	backBufferPtr = nullptr;

	// 깊이 버퍼의 description 작성
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

	// 그 description으로 깊이 버퍼 텍스쳐 생성(화면에 그려지는 건 2D니까 2D로 생성)
	result = device_->CreateTexture2D(&depthBufferDesc, NULL, 
		depthStencilBuffer_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// 깊이-스텐실 description 작성(어떤 깊이 테스트를 할지 정할 수 있게)
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
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR; // 왜 다르지?
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 깊이-스텐실 상태 생성
	result = device_->CreateDepthStencilState(&depthStencilDesc, depthStencilState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);
	// 깊이-스텐실 상태 설정(디바이스가 아니라 컨텍스트)
	deviceContext_->OMSetDepthStencilState(depthStencilState_.Get(), 1);

	// 깊이-스텐실 "뷰" description 초기화
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device_->CreateDepthStencilView(depthStencilBuffer_.Get(),
		&depthStencilViewDesc, depthStencilView_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// 지금껏 만든 깊이-스텐실 뷰를 출력 렌더링 파이프라인에 바인딩
	deviceContext_->OMSetRenderTargets(1,
		renderTargetView_.GetAddressOf(), depthStencilView_.Get());

	// 래스터화기 상태(도형이 어떻게 픽셀로 그려지는지) e.g. 와이어프레임 or 앞뒷면
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
	// 레스터 상태 설정
	deviceContext_->RSSetState(rasterState_.Get());

	D3D11_VIEWPORT viewport;
	// 뷰포트 설정(윈도우 전체 크기와 동일하게)
	viewport.Width = static_cast<float>(screenWidth);
	viewport.Height = static_cast<float>(screenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = viewport.TopLeftY = 0.0f;
	// 뷰포트 생성
	deviceContext_->RSSetViewports(1, &viewport);

	// 투영 행렬(3D -> 2D) 설정 및 생성
	float fieldOfView = 3.141592654f / 4.0f;
	float screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	projectionMatrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	worldMatrix_ = XMMatrixIdentity(); // 단위 행렬로 초기화

	// 직교 투영 행렬(UI 등 2D 요소 그리기)
	orthoMatrix_ = XMMatrixOrthographicLH(
		static_cast<float>(screenWidth), static_cast<float>(screenHeight), -1, 1);

	// 반투명 랜더링용 --> depth에 맞게 그리되, depth 값 갱신은 하지 않기
	D3D11_DEPTH_STENCIL_DESC depthNonWriteStencilDesc = depthStencilDesc;
	depthNonWriteStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	result = device_->CreateDepthStencilState(&depthNonWriteStencilDesc, depthNonWriteStencilState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// 기존 desc에서 DepthEnable만 false로 바꾸기!
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc = depthStencilDesc;
	depthDisabledStencilDesc.DepthEnable = false;

	result = device_->CreateDepthStencilState(&depthDisabledStencilDesc, depthDisabledStencilState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE; // 알파값 적용
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 알파값 적용
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	result = device_->CreateBlendState(&blendStateDescription,
		alphaEnableBlendingState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	// 알파값 미적용
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	result = device_->CreateBlendState(&blendStateDescription,
		alphaDisableBlendingState_.GetAddressOf());
	if (FAILED(result)) throw GameException(L"Failed to create D3DClass.", WFILE, __LINE__);

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) throw GAME_EXCEPTION(L"Failed to initialize DirectXTex library.");
}

D3DClass::~D3DClass()
{
	// 스왑 체인은 끝나기 전에 반드시! 윈도우 모드로 바꾸기
	// 그렇지 않으면 예외 발생
	if (swapChain_) swapChain_->SetFullscreenState(false, NULL);
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4] = { red, green, blue, alpha };

	// 백버퍼 내용 지움
	deviceContext_->ClearRenderTargetView(renderTargetView_.Get(), color);

	// 깊이 버퍼 내용 지움
	deviceContext_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void D3DClass::EndScene()
{
	// 현재 렌더링이 완료된 상태, 백버퍼의 내용을 화면에 표시
	if (m_vsync_enabled)
	{
		// 새로 고침 비율 고정
		swapChain_->Present(1, 0);
	}
	else
	{
		// 가능한 빠르게!
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

void D3DClass::EnableAlphaBlending()
{
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext_->OMSetBlendState(alphaEnableBlendingState_.Get(), blendFactor, 0xffffffff);
}

void D3DClass::DisableAlphaBlending()
{
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext_->OMSetBlendState(alphaDisableBlendingState_.Get(), blendFactor, 0xffffffff);
}
