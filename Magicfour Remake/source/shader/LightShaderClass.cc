#include "shader/LightShaderClass.hh"

#include <algorithm>
#include <fstream>

#include "core/GameException.hh"
#include "core/D3DClass.hh"
#include "graphics/ModelClass.hh"
#include "graphics/FrustumCuller.hh"

LightShaderClass::LightShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(device, device_context, hwnd, L"shader/light.vs", L"shader/light.ps");
}

LightShaderClass::~LightShaderClass()
{

}

void LightShaderClass::PushRenderQueue(std::shared_ptr<ModelClass> model, XMMATRIX world_matrix)
{
	RenderCommand render_command;
	render_command.model = model;
	render_command.world_matrix = world_matrix;
	render_command.texture = model->GetDiffuseTexture();

	render_queue_[model].push_back(render_command);
}

void LightShaderClass::PushRenderQueue(std::shared_ptr<ModelClass> model, XMMATRIX world_matrix,
	ID3D11ShaderResourceView* texture)
{
	RenderCommand render_command;
	render_command.model = model;
	render_command.world_matrix = world_matrix;
	render_command.texture = texture;

	render_queue_[model].push_back(render_command);
}

void LightShaderClass::ProcessRenderQueue(ID3D11DeviceContext* device_context,
	const XMMATRIX& vp_matrix, XMFLOAT3 light_direction, XMFLOAT4 diffuse_color)
{
	FrustumCuller fruster_culler(vp_matrix);
	for (auto& [model, params] : render_queue_)
	{
		// Batch processing for draw calls with same model
		model->Render(device_context);
		for (const auto& param : params)
		{
			// Check if the model is in the view frustum
			if (!fruster_culler.IsInFrustum(model->GetBoundingVolume())) continue;

			// Set the shader parameters that it will use for rendering.
			SetShaderParameters(device_context, param.world_matrix, vp_matrix, param.texture, light_direction, diffuse_color);

			// Now render the prepared buffers with the shader.
			RenderShader(device_context, model->GetIndexCount());
		}
	}

	render_queue_.clear();
}

void LightShaderClass::InitializeShader(ID3D11Device* device,
	ID3D11DeviceContext* device_context, HWND hwnd,
	const WCHAR* vs_filename, const WCHAR* ps_filename)
{
	constexpr int kNumOfElements = 3;
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

	ShaderClass::CreateShaderObject(device, device_context, hwnd, vs_filename, ps_filename, polygon_layout, 3);

	// Create the texture sampler state.
	sample_state_ = ShaderClass::CreateSamplerState(device);

	matrix_buffer_ = CreateBasicConstantBuffer<MatrixBufferType>(device);
	if (!matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create matrix buffer");

	light_buffer_ = CreateBasicConstantBuffer<LightBufferType>(device);
	if (!light_buffer_) throw GAME_EXCEPTION(L"Failed to create light buffer");
}

void LightShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context, XMMATRIX world_matrix, XMMATRIX vp_matrix,
	ID3D11ShaderResourceView* texture, XMFLOAT3 light_direction, XMFLOAT4 diffuse_color)
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
	dataPtr->mvp = XMMatrixTranspose(world_matrix * vp_matrix);
	dataPtr->world_tr_inv = XMMatrixInverse(nullptr, world_matrix);

	// Unlock the constant buffer.
	device_context->Unmap(matrix_buffer_.Get(), 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	device_context->VSSetConstantBuffers(bufferNumber, 1, matrix_buffer_.GetAddressOf());

	// Set shader texture resource in the pixel shader.
	device_context->PSSetShaderResources(0, 1, &texture);

	// Lock the light constant buffer so it can be written to.
	result = device_context->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock light buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->diffuse_color = diffuse_color;
	dataPtr2->light_direction = light_direction;
	dataPtr2->padding = 0.0f;

	// Unlock the constant buffer.
	device_context->Unmap(light_buffer_.Get(), 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	device_context->PSSetConstantBuffers(bufferNumber, 1, light_buffer_.GetAddressOf());
}


void LightShaderClass::RenderShader(ID3D11DeviceContext* device_context, int indexCount)
{
	// Set the vertex input layout.
	device_context->IASetInputLayout(input_layout_.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
	device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

	// Render the triangle.
	device_context->DrawIndexed(indexCount, 0, 0);

	return;
}