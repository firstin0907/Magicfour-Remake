#pragma once

#include "shader/ShaderClass.hh"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>

class StoneShaderClass : public ShaderClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

	struct MatrixBufferType
	{
		XMMATRIX mvp;
		XMMATRIX world;
		XMMATRIX world_tr_inv;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuse_color;
		XMFLOAT3 light_direction;
		float padding;  // Added extra padding so structure is a multiple of 16 for CreateBuffer function requirements.

		XMFLOAT4 ambient_weight;
		XMFLOAT4 diffuse_weight;
		XMFLOAT4 specular_weight;
	};

	struct CameraBufferType
	{
		XMFLOAT3 camera_pos;
		float padding;
	};

public:
	StoneShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);
	StoneShaderClass(const StoneShaderClass&) = delete;
	~StoneShaderClass() = default;

	void Render(
		class ModelClass* model, XMMATRIX world_matrix, XMMATRIX vp_matrix,
		XMFLOAT3 light_direction, XMFLOAT4 diffuse_color, XMFLOAT3 camera_pos);

	void Render(int indexCount,
		XMMATRIX world_matrix, XMMATRIX vp_matrix,
		XMFLOAT3 light_direction, XMFLOAT4 diffuse_color, XMFLOAT3 camera_pos);

private:
	void InitializeShader(HWND hwnd,
		const WCHAR* vs_filename, const WCHAR* ps_filename);

	void SetShaderParameters(XMMATRIX, XMMATRIX,
		XMFLOAT3, XMFLOAT4,
		XMFLOAT3 camera_pos, XMFLOAT3, XMFLOAT3, XMFLOAT3);
	void RenderShader(int);

private:
	ComPtr<ID3D11SamplerState>	sample_state_;

	ComPtr<ID3D11Buffer>		matrix_buffer_;
	ComPtr<ID3D11Buffer>		light_buffer_;
	ComPtr<ID3D11Buffer>		camera_buffer_;
};