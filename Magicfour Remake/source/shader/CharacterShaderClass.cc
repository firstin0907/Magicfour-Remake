#include "shader/CharacterShaderClass.hh"

#include <algorithm>
#include <fstream>

#include "core/GameException.hh"
#include "core/D3DClass.hh"
#include "graphics/FbxModel.hh"
#include "graphics/FrustumCuller.hh"

CharacterShaderClass::CharacterShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(device, device_context, hwnd, L"shader/character.vs", L"shader/character.ps");
}

CharacterShaderClass::~CharacterShaderClass() = default;

void CharacterShaderClass::PushRenderQueue(std::shared_ptr<FbxModel> model, XMMATRIX world_matrix)
{
	RenderCommand render_command;
	render_command.model = model;
	render_command.world_matrix = world_matrix;

	render_queue_[model].push_back(render_command);
}


void CharacterShaderClass::ProcessRenderQueue(ID3D11DeviceContext* device_context, const XMMATRIX& vp_matrix)
{
    FrustumCuller fruster_culler(vp_matrix);

    SetShader(device_context);
    for (auto& [model, params] : render_queue_)
    {
        for (int i = 0; i < model->GetMeshCount(); i++)
        {
            model->Render(device_context, i);

			// Set the textures for this buffer index.
			SetTextures(device_context,
				model->GetDiffuseTexture(i),
				model->GetNormalTexture(i),
				model->GetEmissiveTexture(i));

            // Batch processing for draw calls with same model
            for (const auto& param : params)
			{
				// TODO: Check if the model is in the view frustum
				// if (!fruster_culler.IsInFrustum(model->GetBoundingVolume())) continue;

				// Set the shader parameters that it will use for rendering.
				SetShaderParameters(device_context, param, vp_matrix);


				// Now render the prepared buffers with the shader.
				RenderShader(device_context, model->GetMeshVertexCount(i), 0);
			}
		}
	}

	render_queue_.clear();
}

void CharacterShaderClass::InitializeShader(ID3D11Device* device,
	ID3D11DeviceContext* device_context, HWND hwnd,
	const WCHAR* vs_filename, const WCHAR* ps_filename)
{
	constexpr int kNumOfElements = 5;
	D3D11_INPUT_ELEMENT_DESC polygon_layout[kNumOfElements];

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	for (int i = 0; i < kNumOfElements; i++)
	{
		polygon_layout[i].SemanticIndex = 0;
		polygon_layout[i].InputSlot = 0;
		polygon_layout[i].InstanceDataStepRate = 0;
		polygon_layout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygon_layout[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	}
	polygon_layout[0].AlignedByteOffset = 0;

	polygon_layout[0].SemanticName = "POSITION";
	polygon_layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	polygon_layout[1].SemanticName = "TEXCOORD";
	polygon_layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;

	polygon_layout[2].SemanticName = "NORMAL";
	polygon_layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;

    polygon_layout[3].SemanticName = "BONEIDS";
    polygon_layout[3].Format = DXGI_FORMAT_R32G32B32A32_SINT;

    polygon_layout[4].SemanticName = "WEIGHTS";
    polygon_layout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	ShaderClass::CreateShaderObject(device, device_context, hwnd, vs_filename, ps_filename, polygon_layout, kNumOfElements);

	// Create the texture sampler state.
	sample_state_ = ShaderClass::CreateSamplerState(device);

	matrix_buffer_ = CreateBasicConstantBuffer<MatrixBufferType>(device);
	if (!matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create matrix buffer");

	light_buffer_ = CreateBasicConstantBuffer<LightBufferType>(device);
	if (!light_buffer_) throw GAME_EXCEPTION(L"Failed to create light buffer");

	bone_matrix_buffer_ = CreateBasicConstantBuffer<BoneMatrixBufferType>(device);
	if (!bone_matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create bone matrix buffer");
}

void CharacterShaderClass::SetTextures(ID3D11DeviceContext* device_context,
	ID3D11ShaderResourceView* diffuse_texture,
	ID3D11ShaderResourceView* normal_texture,
	ID3D11ShaderResourceView* emissive_texture)
{
	// Set the texture array in the pixel shader.	// Set shader texture resource in the pixel shader.
	if (diffuse_texture) device_context->PSSetShaderResources(0, 1, &diffuse_texture);
	if (normal_texture) device_context->PSSetShaderResources(1, 1, &normal_texture);
	if (emissive_texture) device_context->PSSetShaderResources(2, 1, &emissive_texture);
}

void CharacterShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context,
    const RenderCommand& render_command, XMMATRIX vp_matrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;

	// Lock the constant buffer so it can be written to.
	result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock matrix buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Transpose the matrices to prepare them for the shader.
	// And Copy them.
	dataPtr->mvp = XMMatrixTranspose(render_command.world_matrix * vp_matrix);
	dataPtr->world_tr_inv = XMMatrixInverse(nullptr, render_command.world_matrix);

	// Unlock the constant buffer.
	device_context->Unmap(matrix_buffer_.Get(), 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	device_context->VSSetConstantBuffers(bufferNumber, 1, matrix_buffer_.GetAddressOf());


	// Lock the light constant buffer so it can be written to.
	result = device_context->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock light buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer. TODO: Set actual light direction and color
	dataPtr2->diffuse_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dataPtr2->light_direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	dataPtr2->padding = 0.0f;

	// Unlock the constant buffer.
	device_context->Unmap(light_buffer_.Get(), 0);
    
	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 1;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	device_context->PSSetConstantBuffers(bufferNumber, 1, light_buffer_.GetAddressOf());

    BoneMatrixBufferType* boneDataPtr;
    result = device_context->Map(bone_matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock bone matrix buffer to set shader parameter.");
    boneDataPtr = (BoneMatrixBufferType*)mappedResource.pData;
    for (int i = 0; i < render_command.model->bone_info_.size(); i++)
    {
        boneDataPtr->bone_transforms[i] = DirectX::XMMatrixTranspose(render_command.model->bone_info_[i].final_transform);
    }
    device_context->Unmap(bone_matrix_buffer_.Get(), 0);

	// Set the position of the bone matrix constant buffer in the vertex shader.
	bufferNumber = 2;

	// Finally set the bone matrix constant buffer in the vertex shader with the updated values.
	device_context->VSSetConstantBuffers(bufferNumber, 1, bone_matrix_buffer_.GetAddressOf());

}


void CharacterShaderClass::RenderShader(ID3D11DeviceContext* device_context, int indexCount, int index_start)
{
	// Set the sampler state in the pixel shader.
	device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

	// Render the indexed geometry; use start offset if supplied.
	device_context->DrawIndexed(indexCount, index_start, 0);

	return;
}