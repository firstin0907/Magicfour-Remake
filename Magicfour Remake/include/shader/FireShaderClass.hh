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

class FireShaderClass : public ShaderClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT2 = DirectX::XMFLOAT2;

	struct MatrixBufferType
	{
		XMMATRIX mvp;
	};

	struct NoiseBufferType
	{
		float frame_time;
		XMFLOAT3 scroll_speeds;
		XMFLOAT3 scales;
		float padding;
	};

	struct DistortionBuffer
	{
		XMFLOAT2 distortion1;
		XMFLOAT2 distortion2;
		XMFLOAT2 distortion3;
		float distortion_scale;
		float distortion_bias;
	};

public:
	FireShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);
	FireShaderClass(const FireShaderClass&) = delete;
	~FireShaderClass();

	void PushRenderQueue(class ModelClass* model,
		XMMATRIX mvp_matrix,
		ID3D11ShaderResourceView* fire_texture,
		ID3D11ShaderResourceView* noise_texture,
		ID3D11ShaderResourceView* alpha_texture,
		XMFLOAT3 scroll_speeds,
		XMFLOAT3 scales,
		XMFLOAT2 distortion1,
		XMFLOAT2 distortion2,
		XMFLOAT2 distortion3,
		float distortion_scale,
		float distortion_bias);

	void ProcessRenderQueue(float frame_time);

	void Render(class ModelClass* model,
		XMMATRIX mvp_matrix,
		ID3D11ShaderResourceView* fire_texture,
		ID3D11ShaderResourceView* noise_texture,
		ID3D11ShaderResourceView* alpha_texture,
		float frame_time,
		XMFLOAT3 scroll_speeds,
		XMFLOAT3 scales,
		XMFLOAT2 distortion1,
		XMFLOAT2 distortion2,
		XMFLOAT2 distortion3,
		float distortion_scale,
		float distortion_bias);

private:
	void InitializeShader(HWND hwnd, const WCHAR* vs_filename, const WCHAR* ps_filename);

	void SetShaderParameters(
		const MatrixBufferType& matrix_buffer,
		const NoiseBufferType& noise_buffer,
		const DistortionBuffer& distortion_buffer,
		ID3D11ShaderResourceView* fire_texture,
		ID3D11ShaderResourceView* noise_texture,
		ID3D11ShaderResourceView* alpha_texture
	);
	
	void RenderShader(int);


private:
	ComPtr<ID3D11SamplerState>	sample_state_wrap_;
	ComPtr<ID3D11SamplerState>	sample_state_clamp_;

	ComPtr<ID3D11Buffer>		matrix_buffer_;
	ComPtr<ID3D11Buffer>		noise_buffer_;
	ComPtr<ID3D11Buffer>		distortion_buffer_;

	struct RenderCommand
	{
		class ModelClass*	model;
		
		XMMATRIX mvp_matrix;
		ID3D11ShaderResourceView* fire_texture;
		ID3D11ShaderResourceView* noise_texture;
		ID3D11ShaderResourceView* alpha_texture;

		XMFLOAT3	scroll_speeds;
		XMFLOAT3	scales;

		XMFLOAT2	distortion1;
		XMFLOAT2	distortion2;
		XMFLOAT2	distortion3;

		float		distortion_scale;
		float		distortion_bias;
	};

	std::unordered_map<ModelClass*, std::vector<RenderCommand> > render_queue_;
};