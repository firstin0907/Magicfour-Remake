#include "graphics/ParticleSystemBaseClass.hh"

#include <algorithm>

#include "graphics/TextureClass.hh"
#include "core/GameException.hh"

#include "core/global.hh"

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

ParticleSystemBaseClass::ParticleSystemBaseClass(ID3D11Device* device, const char* texture_filename,
	float particle_size, float particles_per_second, size_t max_particle_num,
	bool sort_particle_with_z)
	: particle_size_(particle_size),
	particles_per_second_(particles_per_second),
	max_particle_num_(max_particle_num),
	sort_particle_with_z_(sort_particle_with_z)
{
	m_vertices = nullptr;
	
	LoadTexture(device, texture_filename);
	InitializeParticleSystem();
	InitializeBuffers(device);
}

ParticleSystemBaseClass::~ParticleSystemBaseClass()
{
	// Release the buffers.
	ShutdownBuffers();
}


void ParticleSystemBaseClass::Clear()
{
	ClearParticles();
}

bool ParticleSystemBaseClass::Frame(time_t frame_time, time_t time_delta, ID3D11DeviceContext* device_context)
{
	bool result;

	// Emit new particles.
	EmitParticles(frame_time, time_delta);

	// Update the position of the particles.
	UpdateParticles(frame_time, time_delta);

	// Update the dynamic vertex buffer with the new position of each particle.
	UpdateVertices();
	result = UpdateBuffers(device_context);
	if (!result) return false;

	return true;
}


void ParticleSystemBaseClass::Render(ID3D11DeviceContext* device_context)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(device_context);

	return;
}


ID3D11ShaderResourceView* ParticleSystemBaseClass::GetTexture()
{
	return texture_->GetTexture();
}


int ParticleSystemBaseClass::GetIndexCount()
{
	return index_count_;
}


void ParticleSystemBaseClass::LoadTexture(ID3D11Device* device, const char* filename)
{
	// Create and initialize the texture object.
	texture_ = std::make_unique<TextureClass>(device, filename);
}


void ParticleSystemBaseClass::InitializeParticleSystem()
{
	// Create the particle list.
	particle_list_.reserve(max_particle_num_);

	// Clear the initial accumulated time for the particle per second emission rate.
	accumulated_time_ = 0.0f;
}


void ParticleSystemBaseClass::InitializeBuffers(ID3D11Device* device)
{
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Set the maximum number of vertices in the vertex array.
	vertex_count_ = max_particle_num_ * 6;

	// Set the maximum number of indices in the index array.
	index_count_ = vertex_count_;

	// Create the vertex array for the particles that will be rendered.
	m_vertices = new VertexType[vertex_count_];

	// Create the index array.
	indices = new unsigned long[index_count_];
	
	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(VertexType) * vertex_count_));

	// Initialize the index array.
	for (int i = 0; i < index_count_; i++) indices[i] = i;

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertex_count_;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, vertex_buffer_.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create vertex buffer.");

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * index_count_;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, index_buffer_.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create index buffer.");

	// Release just the index array since it is no longer needed.
	delete[] indices;
	indices = 0;
}


void ParticleSystemBaseClass::ShutdownBuffers()
{
	// Release the vertices.
	if (m_vertices)
	{
		delete[] m_vertices;
		m_vertices = nullptr;
	}

	return;
}


void ParticleSystemBaseClass::RenderBuffers(ID3D11DeviceContext* device_context)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	device_context->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	device_context->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

void ParticleSystemBaseClass::UpdateVertices()
{
	// Now build the vertex array from the particle list array. 
	// Each particle is a quad made out of two triangles.
	int index = 0;
	for (auto& particle : particle_list_)
	{
		constexpr int dx[] = { -1, -1, 1, 1, -1, 1 };
		constexpr int dy[] = { -1, 1, -1, -1, 1, 1 };

		for (size_t j = 0; j < 6; j++)
		{
			m_vertices[index + j].position = XMFLOAT3(
				particle.position.x + dx[j] * particle_size_,
				particle.position.y + dy[j] * particle_size_,
				particle.position.z);
			m_vertices[index + j].texture = XMFLOAT2((dx[j] + 1) / 2.0f,(dy[j] + 1) / 2.0f);
			m_vertices[index + j].color = XMFLOAT4(particle.red, particle.green, particle.blue, 1.0f);
		}

		index += 6;
	}

	index_count_ = index;
}


bool ParticleSystemBaseClass::UpdateBuffers(ID3D11DeviceContext* device_context)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;

	// Lock the vertex buffer.
	result = device_context->Map(vertex_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices, (sizeof(VertexType) * vertex_count_));

	// Unlock the vertex buffer.
	device_context->Unmap(vertex_buffer_.Get(), 0);

	return true;
}