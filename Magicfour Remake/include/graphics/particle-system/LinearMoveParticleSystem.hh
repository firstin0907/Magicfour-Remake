#pragma once
#include "graphics/ParticleSystemBaseClass.hh"

#include <vector>

#include "graphics/particle-system/ParticleType.hh"

class LinearMoveParticleSystem : public ParticleSystem<LinearMoveParticleSystem>
{
public:
	LinearMoveParticleSystem(ID3D11Device* device,
		const char* texture_filename,
		float particle_size, float particles_per_second, size_t max_particle_num,
		XMFLOAT3 start_position_coord_min,
		XMFLOAT3 start_position_coord_max,
		float particle_start_velocity_min,
		float particle_start_velocity_max);

	ParticleType CreateParticle(time_t curr_time);
	bool UpdateParticle(ParticleType& particle, time_t curr_time, time_t time_delta);

private:
	XMFLOAT3 start_position_coord_min_;
	XMFLOAT3 start_position_coord_max_;
	float particle_start_velocity_min_;
	float particle_start_velocity_max_;
};