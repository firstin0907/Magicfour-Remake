#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class D3DClass
{
public:
	D3DClass(int, int, bool, HWND, bool, float, float);
	D3DClass(const D3DClass&) = delete;
	~D3DClass();

	void BeginScene(float, float, float, float);
	void EndScene();

	IDXGISwapChain* GetSwapChain();
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	void TurnZBufferOn();
	void TurnZBufferOff();

	void EnableAlphaBlending();
	void DisableAlphaBlending();

private:
	bool	m_vsync_enabled;		// 수직 동기화 설정
	int		m_videoCardMemory;
	char	m_videoCardDescription[128];

	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

	ComPtr<ID3D11DepthStencilState> m_depthDisabledStencilState; // for 2d
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;

	ComPtr<ID3D11RasterizerState> m_rasterState;

	ComPtr<ID3D11BlendState> m_alphaEnableBlendingState;
	ComPtr<ID3D11BlendState> m_alphaDisableBlendingState;

	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;

};
