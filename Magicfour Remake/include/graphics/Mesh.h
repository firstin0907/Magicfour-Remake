#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>

#include <core/GameException.hh>

template<typename VertexType> 
class MeshBuffer
{
public:
	MeshBuffer() = default;

	void Initialize(ID3D11Device* device,
		const std::vector<VertexType>& vertices,
		const std::vector<unsigned long>& indices)
	{
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT result;

		// Set up the description of the static vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		result = device->CreateBuffer(&vertexBufferDesc, &vertexData, vertex_buffer_.GetAddressOf());
		if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create vertex buffer.");

		// Set up the description of the static index buffer.
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		result = device->CreateBuffer(&indexBufferDesc, &indexData, index_buffer_.GetAddressOf());
		if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create index buffer.");

		index_count_ = indices.size();
	}

	void Render(ID3D11DeviceContext* device_context)
	{
		// Set vertex buffer stride and offset.
		unsigned int stride = sizeof(VertexType);
		unsigned int offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		device_context->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(), &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		device_context->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	size_t GetIndexCount() const
	{
		return index_count_;
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;
	size_t index_count_;
};