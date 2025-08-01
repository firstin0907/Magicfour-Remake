#include "shader/FireShaderClass.hh"

#include "core/GameException.hh"
#include "core/D3DClass.hh"
#include "graphics/ModelClass.hh"

FireShaderClass::FireShaderClass(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
	: ShaderClass(device, device_context)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(hwnd, L"shader/fire.vs", L"shader/fire.ps");
}

FireShaderClass::~FireShaderClass()
{

}

void FireShaderClass::PushRenderQueue(class ModelClass* model,
	XMMATRIX mvp_matrix,
	ID3D11ShaderResourceView* fire_texture,
	ID3D11ShaderResourceView* noise_texture,
	ID3D11ShaderResourceView* alpha_texture,
	XMFLOAT3 scroll_speeds, XMFLOAT3 scales,
	XMFLOAT2 distortion1, XMFLOAT2 distortion2, XMFLOAT2 distortion3,
	float distortion_scale, float distortion_bias)
{
	RenderCommand render_command;
	render_command.model = model;
	render_command.mvp_matrix = mvp_matrix;
	render_command.fire_texture = fire_texture;
	render_command.noise_texture = noise_texture;
	render_command.alpha_texture = alpha_texture;
	render_command.scroll_speeds = scroll_speeds;
	render_command.scales = scales;
	render_command.distortion1 = distortion1;
	render_command.distortion2 = distortion2;
	render_command.distortion3 = distortion3;
	render_command.distortion_scale = distortion_scale;
	render_command.distortion_bias = distortion_bias;

	render_queue_[model].push_back(render_command);
}

void FireShaderClass::ProcessRenderQueue(float frame_time)
{
	for (auto& [model, params] : render_queue_)
	{
		// Batch processing for draw calls with same model
		model->Render(device_context_);
		for (const auto& param : params)
		{
			MatrixBufferType matrix_data;
			matrix_data.mvp = param.mvp_matrix;

			NoiseBufferType noise_data;
			noise_data.frame_time = frame_time;
			noise_data.scroll_speeds = param.scroll_speeds;
			noise_data.scales = param.scales;
			noise_data.padding = 0.0f;

			DistortionBuffer distortion_data;
			distortion_data.distortion1 = param.distortion1;
			distortion_data.distortion2 = param.distortion2;
			distortion_data.distortion3 = param.distortion3;
			distortion_data.distortion_scale = param.distortion_scale;
			distortion_data.distortion_bias = param.distortion_bias;

			// Set the shader parameters that it will use for rendering.
			SetShaderParameters(matrix_data, noise_data, distortion_data,
				param.fire_texture, param.noise_texture, param.alpha_texture);

			// Now render the prepared buffers with the shader.
			RenderShader(model->GetIndexCount());
		}
	}

	render_queue_.clear();
}

void FireShaderClass::Render(ModelClass* model,  XMMATRIX mvp_matrix, ID3D11ShaderResourceView* fire_texture,
	ID3D11ShaderResourceView* noise_texture, ID3D11ShaderResourceView* alpha_texture,
	float frame_time, XMFLOAT3 scroll_speeds, XMFLOAT3 scales,
	XMFLOAT2 distortion1, XMFLOAT2 distortion2, XMFLOAT2 distortion3,
	float distortion_scale, float distortion_bias)
{
	MatrixBufferType matrix_data;
	matrix_data.mvp = mvp_matrix;

	NoiseBufferType noise_data;
	noise_data.frame_time = frame_time;
	noise_data.scroll_speeds = scroll_speeds;
	noise_data.scales = scales;
	noise_data.padding = 0.0f;

	DistortionBuffer distortion_data;
	distortion_data.distortion1 = distortion1;
	distortion_data.distortion2 = distortion2;
	distortion_data.distortion3 = distortion3;
	distortion_data.distortion_scale = distortion_scale;
	distortion_data.distortion_bias = distortion_bias;

	SetShaderParameters(matrix_data, noise_data, distortion_data,
		fire_texture, noise_texture, alpha_texture);

	// Now render the prepared buffers with the shader.
	RenderShader(model->GetIndexCount());
}

void FireShaderClass::InitializeShader(HWND hwnd, const WCHAR* vs_filename, const WCHAR* ps_filename)
{
	constexpr int kNumOfElements = 2;
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

	CreateShaderObject(hwnd, vs_filename, ps_filename, polygon_layout, kNumOfElements);

	// Create the texture sampler state.
	sample_state_clamp_ = CreateSamplerState(D3D11_TEXTURE_ADDRESS_CLAMP);
	sample_state_wrap_ = CreateSamplerState(D3D11_TEXTURE_ADDRESS_WRAP);

	matrix_buffer_ = CreateBasicConstantBuffer<MatrixBufferType>();
	if (!matrix_buffer_) throw GAME_EXCEPTION(L"Failed to create matrix buffer");

	noise_buffer_ = CreateBasicConstantBuffer<NoiseBufferType>();
	if (!noise_buffer_) throw GAME_EXCEPTION(L"Failed to create noise buffer");

	distortion_buffer_ = CreateBasicConstantBuffer<DistortionBuffer>();
	if (!distortion_buffer_) throw GAME_EXCEPTION(L"Failed to create noise buffer");
}

void FireShaderClass::SetShaderParameters(const MatrixBufferType& matrix_buffer_data,
	const NoiseBufferType& noise_buffer_data, const DistortionBuffer& distortion_buffer_data,
	ID3D11ShaderResourceView* fire_texture, ID3D11ShaderResourceView* noise_texture,
	ID3D11ShaderResourceView* alpha_texture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber = 0;
	

	// Lock the constant buffer so it can be written to.
	result = device_context_->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock matrix buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy cbuffer data with transposing matrix.
	*dataPtr = matrix_buffer_data;
	dataPtr->mvp = XMMatrixTranspose(matrix_buffer_data.mvp);

	device_context_->Unmap(matrix_buffer_.Get(), 0);

	// Now set the constant buffer in the vertex shader with the updated bufferNumber values.
	device_context_->VSSetConstantBuffers(bufferNumber++, 1, matrix_buffer_.GetAddressOf());


	// Lock the constant buffer so it can be written to.
	result = device_context_->Map(noise_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock noise buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	NoiseBufferType* dataPtr2 = (NoiseBufferType*)mappedResource.pData;
	*dataPtr2 = noise_buffer_data;

	device_context_->Unmap(noise_buffer_.Get(), 0);

	// Now set the constant buffer in the vertex shader with the updated bufferNumber values.
	device_context_->VSSetConstantBuffers(bufferNumber++, 1, noise_buffer_.GetAddressOf());
	

	bufferNumber = 0; // initialize for pixel shader buffer

	// Set shader texture resource in the pixel shader.
	device_context_->PSSetShaderResources(0, 1, &fire_texture);
	device_context_->PSSetShaderResources(1, 1, &noise_texture);
	device_context_->PSSetShaderResources(2, 1, &alpha_texture);

	result = device_context_->Map(distortion_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to lock noise buffer to set shader parameter.");

	// Get a pointer to the data in the constant buffer.
	DistortionBuffer* dataPtr3 = (DistortionBuffer*)mappedResource.pData;
	*dataPtr3 = distortion_buffer_data;

	device_context_->Unmap(distortion_buffer_.Get(), 0);

	// Now set the constant buffer in the vertex shader with the updated bufferNumber values.
	device_context_->PSSetConstantBuffers(bufferNumber++, 1, distortion_buffer_.GetAddressOf());

}

void FireShaderClass::RenderShader(int indexCount)
{
	// Set the vertex input layout.
	device_context_->IASetInputLayout(input_layout_.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	device_context_->VSSetShader(vertex_shader_.Get(), NULL, 0);
	device_context_->PSSetShader(pixel_shader_.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	device_context_->PSSetSamplers(0, 1, sample_state_wrap_.GetAddressOf());
	device_context_->PSSetSamplers(1, 1, sample_state_clamp_.GetAddressOf());

	// Render the triangle.
	device_context_->DrawIndexed(indexCount, 0, 0);

	return;
}