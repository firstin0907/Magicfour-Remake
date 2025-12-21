#pragma once
#include "graphics/ParticleSystemBaseClass.hh"

#include <vector>

#include "graphics/particle-system/ParticleType.hh"

class RadialSpreadParticleSystem : public ParticleSystem<RadialSpreadParticleSystem>
{
public:
    RadialSpreadParticleSystem(ID3D11Device* device,
		const char* texture_filename,
		float particle_size, float particles_per_second,
		float particle_start_velocity,
		float particle_max_degree,
		time_t particle_lifetime,
		bool sort_particle_with_z = false);

	ParticleType CreateParticle(time_t curr_time);
	bool UpdateParticle(ParticleType& particle, time_t curr_time, time_t time_delta);

private:
	float particle_start_velocity_;
	float particle_max_degree_;

	time_t particle_lifetime_;
};