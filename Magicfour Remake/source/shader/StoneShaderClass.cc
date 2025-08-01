#include "shader/StoneShaderClass.hh"

#include "graphics/ModelClass.hh"
#include "core/GameException.hh"

StoneShaderClass::StoneShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
	: ShaderClass(device, device_context)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(hwnd, L"shader/stone.vs", L"shader/stone.ps");
}


void StoneShaderClass::PushRenderQueue(ModelClass* model, XMMATRIX world_matrix, XMFLOAT4 color)
{
	RenderCommand render_command;

	render_command.model = model;
	render_command.world_matrix = world_matrix;
	render_command.diffuse_color = color;
	
	auto& material_list = model->GetMaterial();
	for (size_t i = 0; i < material_list.size(); i++)
	{
		render_command.ambient_weight  = material_list[i].first.ambient;
		render_command.diffuse_weight  = material_list[i].first.diffuse;
		render_command.specular_weight = material_list[i].first.specular;

		if (i == material_list.size() - 1)
		{
			render_command.index_count = model->GetIndexCount() - material_list[i].second;
			render_command.index_start = material_list[i].second;
		}
		else
		{
			render_command.index_count = material_list[i + 1].second - material_list[i].second;
			render_command.index_start = material_list[i].second;
		}
	}

	

	render_queue_[model].push_back(render_command);
}

void StoneShaderClass::ProcessRenderQueue(const XMMATRIX& vp_matrix,
	XMFLOAT3 light_direction, XMFLOAT3 camera_pos)
{
	for (auto& [model, params] : render_queue_)
	{
		// Batch processing for draw calls with same model
		model->Render(device_context_);
		for (const auto& param : params)
		{
			SetShaderParameters(param.world_matrix, vp_matrix,
				light_direction, param.diffuse_color, camera_pos,
				param.ambient_weight, param.diffuse_weight, param.specular_weight);

			RenderShader(param.index_count, param.index_start);
		}
	}

	render_queue_.clear();
}

void StoneShaderClass::Render(
	ModelClass* model, XMMATRIX world_matrix, XMMATRIX vp_matrix,
	XMFLOAT3 light_direction, XMFLOAT4 diffuse_color, XMFLOAT3 camera_pos)
{
	model->Render(device_context_);
	auto& material_list = model->GetMaterial();
	
	int curr = 0;
	// Set the shader parameters that it will use for rendering.
	for (size_t i = 0; i < material_list.size(); i++)
	{
		SetShaderParameters(world_matrix, vp_matrix,
			light_direction, diffuse_color, camera_pos,
			material_list[i].first.ambient,
			material_list[i].first.diffuse,
			material_list[i].first.specular);

		// Set the vertex input layout.
		device_context_->IASetInputLayout(input_layout_.Get());

		// Set the vertex and pixel shaders that will be used to render this triangle.
		device_context_->VSSetShader(vertex_shader_.Get(), NULL, 0);
		device_context_->PSSetShader(pixel_shader_.Get(), NULL, 0);

		// Set the sampler state in the pixel shader.
		device_context_->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

		// Render the triangle.
		if(i == material_list.size() - 1)
			device_context_->DrawIndexed(model->GetIndexCount() - material_list[i].second, material_list[i].second, 0);
		else device_context_->DrawIndexed(material_list[i+1].second - material_list[i].second, material_list[i].second, 0);
	}
}

void StoneShaderClass::InitializeShader(
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
	polygon_layout[0].AlignedByteOffset = 0;

	polygon_layout[0].SemanticName = "POSITION";
	polygon_layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	polygon_layout[1].SemanticName = "TEXCOORD";
	polygon_layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;

	polygon_layout[2].SemanticName = "NORMAL";
	polygon_layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	CreateShaderObject(hwnd, vs_filename, ps_filename, polygon_layout, kNumOfElements);

	sample_state_ = CreateSamplerState();

	matrix_buffer_ = CreateBasicConstantBuffer<MatrixBufferType>();
	if (!matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create matrix buffer");

	camera_buffer_ = CreateBasicConstantBuffer<CameraBufferType>();
	if (!camera_buffer_) throw GAME_EXCEPTION(L"Failed to create camera buffer");

	light_buffer_ = CreateBasicConstantBuffer<LightBufferType>();
	if (!light_buffer_) throw GAME_EXCEPTION(L"Failed to create light buffer");
}


void StoneShaderClass::SetShaderParameters(XMMATRIX world_matrix, XMMATRIX vp_matrix, 
	XMFLOAT3 light_direction, XMFLOAT4 diffuse_color, XMFLOAT3 camera_pos,
	XMFLOAT3 ambient_weight, XMFLOAT3 diffuse_weight, XMFLOAT3 specular_weight)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;
	CameraBufferType* dataPtr3;

	// Lock the constant buffer so it can be written to.
	result = device_context_->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock matrix buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	// Transpose the matrices to prepare them for the shader.
	dataPtr->mvp = XMMatrixTranspose(world_matrix * vp_matrix);
	dataPtr->world = XMMatrixTranspose(world_matrix);
	dataPtr->world_tr_inv = XMMatrixInverse(nullptr, world_matrix);

	// Unlock the constant buffer.
	device_context_->Unmap(matrix_buffer_.Get(), 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	device_context_->VSSetConstantBuffers(bufferNumber, 1, matrix_buffer_.GetAddressOf());

	// Lock the light constant buffer so it can be written to.
	result = device_context_->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock light buffer to set shader parameter.");


	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->diffuse_color = diffuse_color;
	dataPtr2->light_direction = light_direction;
	dataPtr2->padding = 0.0f;

	dataPtr2->ambient_weight = XMFLOAT4(ambient_weight.x, ambient_weight.y, ambient_weight.z, 1.0f);
	dataPtr2->diffuse_weight = XMFLOAT4(diffuse_weight.x, diffuse_weight.y, diffuse_weight.z, 1.0f);
	dataPtr2->specular_weight = XMFLOAT4(specular_weight.x, specular_weight.y, specular_weight.z, 1.0f);

	// Unlock the constant buffer.
	device_context_->Unmap(light_buffer_.Get(), 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	device_context_->PSSetConstantBuffers(bufferNumber, 1, light_buffer_.GetAddressOf());

	result = device_context_->Map(camera_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to camera matrix buffer to set shader parameter.");


	// Get a pointer to the data in the constant buffer.
	dataPtr3 = (CameraBufferType*)mappedResource.pData;

	// Copy the camera position into the constant buffer.
	dataPtr3->camera_pos = camera_pos;
	dataPtr3->padding = 0.0f;

	// Unlock the camera constant buffer.
	device_context_->Unmap(camera_buffer_.Get(), 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 1;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	device_context_->PSSetConstantBuffers(bufferNumber, 1, camera_buffer_.GetAddressOf());
}


void StoneShaderClass::RenderShader(int index_count, int index_start)
{
	// Set the vertex input layout.
	device_context_->IASetInputLayout(input_layout_.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	device_context_->VSSetShader(vertex_shader_.Get(), NULL, 0);
	device_context_->PSSetShader(pixel_shader_.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	device_context_->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

	// Render the triangle.
	device_context_->DrawIndexed(index_count, index_start, 0);

	return;
}