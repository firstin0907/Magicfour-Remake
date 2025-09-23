#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include <wrl.h>

#include <unordered_map>
#include <vector>

#include "ShaderClass.hh"

class D3DClass;
class ModelClass;

class NormalMapShaderClass : public ShaderClass
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
	NormalMapShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);
	NormalMapShaderClass(const NormalMapShaderClass&) = delete;
	~NormalMapShaderClass() = default;

	void PushRenderQueue(std::shared_ptr<ModelClass> model, XMMATRIX world_matrix);
	void PushRenderQueue(std::shared_ptr<ModelClass> model, XMMATRIX world_matrix,
		ID3D11ShaderResourceView* diffuse_texture,
		ID3D11ShaderResourceView* normal_texture,
		ID3D11ShaderResourceView* emissive_texture);

	void ProcessRenderQueue(ID3D11DeviceContext* device_context, const XMMATRIX& vp_matrix,
		XMFLOAT3 light_direction, XMFLOAT4 diffuse_color, XMFLOAT3 camera_pos);

private:
	void InitializeShader(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd,
		const WCHAR* vs_filename, const WCHAR* ps_filename);

	void SetShaderParameters(ID3D11DeviceContext* device_context,
		XMMATRIX, XMMATRIX,
		ID3D11ShaderResourceView* diffuse_texture,
		ID3D11ShaderResourceView* normal_texture,
		ID3D11ShaderResourceView* emissive_texture,
		XMFLOAT3, XMFLOAT4,
		XMFLOAT3, XMFLOAT3, XMFLOAT3, XMFLOAT3);

	void RenderShader(ID3D11DeviceContext* device_context, int index_count, int index_start = 0);

private:
	ComPtr<ID3D11SamplerState>	sample_state_;

	ComPtr<ID3D11Buffer>		matrix_buffer_;
	ComPtr<ID3D11Buffer>		light_buffer_;
	ComPtr<ID3D11Buffer>		camera_buffer_;

	struct RenderCommand
	{
		std::shared_ptr<ModelClass>	model;
		XMMATRIX					world_matrix;

		ID3D11ShaderResourceView*	diffuse_texture;
		ID3D11ShaderResourceView*	normal_texture;
		ID3D11ShaderResourceView*	emissive_texture;

		XMFLOAT3					ambient_weight;
		XMFLOAT3					diffuse_weight;
		XMFLOAT3					specular_weight;

		int index_count, index_start;
	};

	std::unordered_map<std::shared_ptr<ModelClass>, std::vector<RenderCommand> > render_queue_;
};