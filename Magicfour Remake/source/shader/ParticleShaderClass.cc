#include "shader/ParticleShaderClass.hh"

#include "graphics/ParticleSystemBaseClass.hh"
#include "core/GameException.hh"

ParticleShaderClass::ParticleShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
{
	//Initialize the vertex and pixel shaders.
	InitializeShader(device, device_context, hwnd, L"shader/particle.vs", L"shader/particle.ps");
}

void ParticleShaderClass::PushRenderQueue(std::shared_ptr<ParticleSystemBaseClass> particle_system,
	XMMATRIX world_matrix)
{
	RenderCommand render_command;
	render_command.particle_system = particle_system;
	render_command.world_matrix    = world_matrix;

	render_queue_[particle_system].push_back(render_command);
}

void ParticleShaderClass::ProcessRenderQueue(ID3D11DeviceContext* device_context, XMMATRIX vp_matrix)
{
	for (auto& [particle_system, commands] : render_queue_)
	{
		// Batch processing for draw calls with same particle system
		particle_system->Render(device_context);
		for (const auto& command : commands)
		{
			// Set the shader parameters that it will use for rendering.
			if (!SetShaderParameters(device_context,
				command.world_matrix, vp_matrix,
				particle_system->GetTexture()))
			{
				continue;
			}

			// Now render the prepared buffers with the shader.
			RenderShader(device_context, particle_system->GetIndexCount());
		}
	}

	render_queue_.clear();
}

void ParticleShaderClass::InitializeShader(ID3D11Device* device, ID3D11DeviceContext* device_context,
	HWND hwnd, const WCHAR* vs_filename, const WCHAR* ps_filename)
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

	polygon_layout[0].SemanticName = "POSITION";
	polygon_layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	polygon_layout[1].SemanticName = "TEXCOORD";
	polygon_layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;

	polygon_layout[2].SemanticName = "COLOR";
	polygon_layout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	CreateShaderObject(device, device_context, hwnd, vs_filename, ps_filename, polygon_layout, kNumOfElements);

	// Create the texture sampler state.
	sample_state_ = CreateSamplerState(device, D3D11_TEXTURE_ADDRESS_WRAP);

	matrix_buffer_ = CreateBasicConstantBuffer<MatrixBufferType>(device);
	if (!matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create matrix buffer");
}

bool ParticleShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context,
	XMMATRIX world_matrix, XMMATRIX vp_matrix,
	ID3D11ShaderResourceView* texture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;

    // Transpose the matrices to prepare them for the shader.
    XMMATRIX mvp_matrix = XMMatrixTranspose(world_matrix * vp_matrix);

    // Lock the constant buffer so it can be written to.
    result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result)) return false;

    // Get a pointer to the data in the constant buffer.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // Copy the matrices into the constant buffer.
    dataPtr->mvp = mvp_matrix;

    // Unlock the constant buffer.
    device_context->Unmap(matrix_buffer_.Get(), 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Finally set the constant buffer in the vertex shader with the updated values.
    device_context->VSSetConstantBuffers(bufferNumber, 1, matrix_buffer_.GetAddressOf());

    // Set shader texture resource in the pixel shader.
    device_context->PSSetShaderResources(0, 1, &texture);

    return true;
}

void ParticleShaderClass::RenderShader(ID3D11DeviceContext* device_context, int index_count, int index_start)
{
	// Set the vertex input layout.
	device_context->IASetInputLayout(input_layout_.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
	device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

	// Render the triangle.
	device_context->DrawIndexed(index_count, index_start, 0);

	return;
}
