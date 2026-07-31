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
class ParticleSystemBaseClass;

class ParticleShaderClass : public ShaderClass
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
	};

public:
	ParticleShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);
	ParticleShaderClass(const ParticleShaderClass&) = delete;
	~ParticleShaderClass() = default;

	void PushRenderQueue(std::shared_ptr<ParticleSystemBaseClass> model,
		XMMATRIX world_matrix);

	/// @brief Enable the shader for rendering a frame. This function sets the shader parameters.
	void EnableShaderForFrame(const XMMATRIX& vp_matrix);

	void ProcessRenderQueue(ID3D11DeviceContext* device_context);

private:
	void InitializeShader(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd,
		const WCHAR* vs_filename, const WCHAR* ps_filename);

	bool SetShaderParameters(ID3D11DeviceContext* device_context,
		XMMATRIX world_matrix, XMMATRIX vp_matrix,
		ID3D11ShaderResourceView* texture);

	void RenderShader(ID3D11DeviceContext* device_context, int index_count, int index_start = 0);

private:
	ComPtr<ID3D11SamplerState>	sample_state_;
	ComPtr<ID3D11Buffer>		matrix_buffer_;

	struct RenderCommand
	{
		std::shared_ptr<ParticleSystemBaseClass> particle_system;
		XMMATRIX world_matrix;
	};

	std::unordered_map<std::shared_ptr<ParticleSystemBaseClass>, std::vector<RenderCommand> > render_queue_;

	struct RenderConstant
	{
		XMMATRIX	vp_matrix;
	};
	bool enabled_for_frame_ = false;
	RenderConstant render_constant_;
};