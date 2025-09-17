#pragma once

#include "ShaderClass.hh"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>
#include <unordered_map>
#include <vector>

class D3DClass;

class LightShaderClass : public ShaderClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

	struct MatrixBufferType
	{
		XMMATRIX mvp; // world * view * projection matrix
		XMMATRIX world_tr_inv;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuse_color;
		XMFLOAT3 light_direction;
		float padding;  // Added extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
	};

public:
	LightShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);
	LightShaderClass(const LightShaderClass&) = delete;
	~LightShaderClass();

	void PushRenderQueue(class ModelClass* model, XMMATRIX world_matrix,
		ID3D11ShaderResourceView* texture);

	void ProcessRenderQueue(ID3D11DeviceContext* device_context, const XMMATRIX& vp_matrix,
		XMFLOAT3 light_direction, XMFLOAT4 diffuse_color);

private:
	void InitializeShader(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd,
		const WCHAR* vs_filename, const WCHAR* ps_filename);

	void SetShaderParameters(ID3D11DeviceContext* device_context,
		XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);

	void RenderShader(ID3D11DeviceContext* device_context, int);

private:
	ComPtr<ID3D11SamplerState>	sample_state_;
	ComPtr<ID3D11Buffer>		matrix_buffer_;
	ComPtr<ID3D11Buffer>		light_buffer_;

	struct RenderCommand
	{
		class ModelClass*			model;
		XMMATRIX					world_matrix;
		ID3D11ShaderResourceView*	texture;
	};

	std::unordered_map<ModelClass*, std::vector<RenderCommand> > render_queue_;
};