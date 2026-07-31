#pragma once

#include "shader/ShaderClass.hh"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <fstream>
#include <unordered_map>
#include <vector>

class D3DClass;
class FbxModel; 

class CharacterShaderClass : public ShaderClass
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

    struct BoneMatrixBufferType
    {
        XMMATRIX bone_transforms[100];
    };

    struct RenderCommand
    {
        std::shared_ptr<FbxModel> model;
        XMMATRIX world_matrix;
        int index_count;
        int index_start;
    };

public:
    CharacterShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);
    CharacterShaderClass(const CharacterShaderClass&) = delete;
    ~CharacterShaderClass();

    void PushRenderQueue(std::shared_ptr<FbxModel> model, XMMATRIX world_matrix);

	/// @brief Enable the shader for rendering a frame. This function sets the shader parameters.
    void EnableShaderForFrame(const XMMATRIX& vp_matrix);

    void ProcessRenderQueue(ID3D11DeviceContext* device_context);


private:
    void InitializeShader(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd,
        const WCHAR* vs_filename, const WCHAR* ps_filename);

    void SetShaderParameters(ID3D11DeviceContext* device_context,
        const RenderCommand& render_command, XMMATRIX vp_matrix);

    void SetTextures(ID3D11DeviceContext* device_context,
        ID3D11ShaderResourceView* diffuse_texture,
        ID3D11ShaderResourceView* normal_texture,
        ID3D11ShaderResourceView* emissive_texture);

    void RenderShader(ID3D11DeviceContext* device_context, int index_count, int index_start = 0);

private:
    ComPtr<ID3D11Buffer> matrix_buffer_;
    ComPtr<ID3D11Buffer> light_buffer_;
    ComPtr<ID3D11Buffer> bone_matrix_buffer_;
    ComPtr<ID3D11SamplerState> sample_state_;

    std::unordered_map<std::shared_ptr<FbxModel>, std::vector<RenderCommand> > render_queue_;

	struct RenderConstant
	{
		XMMATRIX	vp_matrix;
	};
	bool enabled_for_frame_ = false;
	RenderConstant render_constant_;
};