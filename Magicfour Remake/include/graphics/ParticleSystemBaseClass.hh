#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <memory>
#include <vector>

#include <wrl.h>

#include "particle-system/ParticleType.hh"

using namespace DirectX;

class TextureClass;
class ParticleSystemBaseClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT4 color;
	};

public:
	ParticleSystemBaseClass(ID3D11Device* device, const char* texture_filename,
		float particle_size, float particles_per_second, size_t max_particle_num,
		bool sort_particle_with_z = false);

	ParticleSystemBaseClass(const ParticleSystemBaseClass&) = delete;
	~ParticleSystemBaseClass();

	bool Frame(time_t curr_time, time_t time_delta, ID3D11DeviceContext*);
	void Render(ID3D11DeviceContext*);

	ID3D11ShaderResourceView* GetTexture();
	int GetIndexCount();

private:
	void LoadTexture(ID3D11Device*, const char*);

	void InitializeParticleSystem();

	void InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	virtual void EmitParticles(time_t curr_time, time_t time_delta) = 0;
	virtual void UpdateParticles(time_t curr_time, time_t time_delta) = 0;
	
	void UpdateVertices();
	
	bool UpdateBuffers(ID3D11DeviceContext*);

protected:
	std::shared_ptr<TextureClass> texture_;
	std::vector<ParticleType> particle_list_;
	VertexType* m_vertices;
	ComPtr<ID3D11Buffer> vertex_buffer_, index_buffer_;

	int m_vertexCount, m_indexCount;
	float particle_size_, particles_per_second_;
	size_t max_particle_num_;
	float m_accumulatedTime;

	const bool sort_particle_with_z_;
};

// T Should implement functions below.
	// ParticleType CreateParticle(time_t curr_time, time_t time_delta);
	// void UpdateParticle(ParticleType& particle, time_t curr_time, time_t time_delta);

template <typename T>
class ParticleSystem : public ParticleSystemBaseClass
{
public:
	ParticleSystem(ID3D11Device* device, const char* texture_filename,
		float particle_size, float particles_per_second, size_t max_particle_num,
		bool sort_particle_with_z = false)
		: ParticleSystemBaseClass(device, texture_filename,
			particle_size, particles_per_second, max_particle_num,
			sort_particle_with_z) {
	}

	virtual void EmitParticles(time_t curr_time, time_t time_delta)
	{
		bool emitParticle = false;

		// Increment the frame time.
		m_accumulatedTime += curr_time;

		// Check if it is time to emit a new particle or not.
		if (m_accumulatedTime > (1.0f / particles_per_second_))
		{
			m_accumulatedTime = 0.0f;
			emitParticle = true;
		}

		// If there are particles to emit then emit one per frame.
		if ((emitParticle == true) && (particle_list_.size() < (max_particle_num_ - 1)))
		{
			ParticleType particle_to_insert = static_cast<T*>(this)->CreateParticle(curr_time);

			if (sort_particle_with_z_)
			{
				// Now since the particles need to be rendered from back to front for blending we have to sort the particle array.
				// We will sort using Z depth so we need to find where in the list the particle should be inserted.
				auto it_to_insert = std::lower_bound(particle_list_.begin(), particle_list_.end(), particle_to_insert,
					[](const ParticleType& a, const ParticleType& b) { return a.position.z < b.position.z; });
				particle_list_.insert(it_to_insert, particle_to_insert);
			}
			else particle_list_.push_back(particle_to_insert);
		}
	}
	
	virtual void UpdateParticles(time_t curr_time, time_t time_delta)
	{
		// Update particles' status and kill if the condition meets.
		for (size_t i = 0; i < particle_list_.size(); i++)
		{
			if (static_cast<T*>(this)->UpdateParticle(particle_list_[i], curr_time, time_delta) == false)
			{
				if (sort_particle_with_z_) particle_list_.erase(particle_list_.begin() + i);
				else std::swap(particle_list_[i], particle_list_.back()), particle_list_.pop_back();
			}
		}
	}
};