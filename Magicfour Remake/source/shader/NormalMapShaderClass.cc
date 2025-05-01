#include "shader/NormalMapShaderClass.hh"

#include <algorithm>
#include <fstream>

#include "core/GameException.hh"
#include "graphics/ModelClass.hh"


NormalMapShaderClass::NormalMapShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
	: ShaderClass(device, device_context)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(hwnd, L"shader/normalmap.vs", L"shader/normalmap.ps");

}

void NormalMapShaderClass::Render(
	ModelClass* model, XMMATRIX world_matrix, XMMATRIX vp_matrix,
	XMFLOAT3 light_direction, XMFLOAT4 diffuse_color, XMFLOAT3 camera_pos)
{
	auto& material_list = model->GetMaterial();

	int curr = 0;
	// Set the shader parameters that it will use for rendering.
	for (size_t i = 0; i < material_list.size(); i++)
	{
		SetShaderParameters(world_matrix, vp_matrix,
			model->GetDiffuseTexture(), model->GetNormalTexture(),
			model->GetEmissiveTexture(),
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
		if (i == material_list.size() - 1)
			device_context_->DrawIndexed(model->GetIndexCount() - material_list[i].second, material_list[i].second, 0);
		else device_context_->DrawIndexed(material_list[i + 1].second - material_list[i].second, material_list[i].second, 0);
	}
}

void NormalMapShaderClass::Render(int indexCount, XMMATRIX world_matrix, XMMATRIX vp_matrix,
	ID3D11ShaderResourceView* diffuse_texture,
	ID3D11ShaderResourceView* normal_texture,
	ID3D11ShaderResourceView* emissive_texture,
	XMFLOAT3 light_direction, XMFLOAT4 diffuse_color, XMFLOAT3 camera_pos)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(world_matrix, vp_matrix,
		diffuse_texture, normal_texture, emissive_texture, light_direction, diffuse_color,
		camera_pos,	{ 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } );

	// Now render the prepared buffers with the shader.
	RenderShader(indexCount);
}


void NormalMapShaderClass::InitializeShader(
	HWND hwnd, const WCHAR* vs_filename, const WCHAR* ps_filename)
{
	constexpr int num_of_elements = 5;
	D3D11_INPUT_ELEMENT_DESC polygon_layout[num_of_elements];

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	for (int i = 0; i < num_of_elements; i++)
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

	polygon_layout[3].SemanticName = "TANGENT";
	polygon_layout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	polygon_layout[4].SemanticName = "BINORMAL";
	polygon_layout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;

	CreateShaderObject(hwnd, vs_filename, ps_filename, polygon_layout, num_of_elements);

	// Create the texture sampler state.
	sample_state_ = CreateSamplerState();

	matrix_buffer_ = CreateBasicConstantBuffer<MatrixBufferType>();
	if (!matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create matrix buffer");

	camera_buffer_ = CreateBasicConstantBuffer<CameraBufferType>();
	if (!camera_buffer_) throw GAME_EXCEPTION(L"Failed to create camera buffer");

	light_buffer_ = CreateBasicConstantBuffer<LightBufferType>();
	if (!light_buffer_) throw GAME_EXCEPTION(L"Failed to create light buffer");
}


void NormalMapShaderClass::SetShaderParameters(
	XMMATRIX world_matrix, XMMATRIX vp_matrix,
	ID3D11ShaderResourceView* diffuse_texture,
	ID3D11ShaderResourceView* normal_texture,
	ID3D11ShaderResourceView* emissive_texture,
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
	// Transpose the matrices to prepare them for the shader.
	// And Copy them.
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
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock constant buffer to set shader parameter.");
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
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to camera buffer to set shader parameter.");
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


	// Set shader texture resource in the pixel shader.
	device_context_->PSSetShaderResources(0, 1, &diffuse_texture);
	device_context_->PSSetShaderResources(1, 1, &normal_texture);
	device_context_->PSSetShaderResources(2, 1, &emissive_texture);

}


void NormalMapShaderClass::RenderShader(int indexCount)
{
	// Set the vertex input layout.
	device_context_->IASetInputLayout(input_layout_.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	device_context_->VSSetShader(vertex_shader_.Get(), NULL, 0);
	device_context_->PSSetShader(pixel_shader_.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	device_context_->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

	// Render the triangle.
	device_context_->DrawIndexed(indexCount, 0, 0);

	return;
}