#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class D3DClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	enum class DepthStencilMode
	{
		Default3D,
		Transparent3D,
		Disabled2D
	};

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

	void SetDepthStencilState(DepthStencilMode mode);

	void EnableAlphaBlending();
	void DisableAlphaBlending();

private:
	bool	m_vsync_enabled;		// 수직 동기화 설정
	int		videoCardMemory_;
	char	videoCardDescription_[128];

	ComPtr<IDXGISwapChain> swapChain_;
	ComPtr<ID3D11Device> device_;
	ComPtr<ID3D11DeviceContext> deviceContext_;
	ComPtr<ID3D11RenderTargetView> renderTargetView_;
	ComPtr<ID3D11Texture2D> depthStencilBuffer_;

	ComPtr<ID3D11DepthStencilState> depthNonWriteStencilState_; // for 3d transparent
	ComPtr<ID3D11DepthStencilState> depthDisabledStencilState_; // for 2d
	ComPtr<ID3D11DepthStencilState> depthStencilState_;
	ComPtr<ID3D11DepthStencilView> depthStencilView_;

	ComPtr<ID3D11RasterizerState> rasterState_;

	ComPtr<ID3D11BlendState> alphaEnableBlendingState_;
	ComPtr<ID3D11BlendState> alphaDisableBlendingState_;

	XMMATRIX projectionMatrix_;
	XMMATRIX worldMatrix_;
	XMMATRIX orthoMatrix_;

};
