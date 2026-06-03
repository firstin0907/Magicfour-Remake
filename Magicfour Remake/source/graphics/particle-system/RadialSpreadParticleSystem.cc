#include "graphics/particle-system/RadialSpreadParticleSystem.hh"

#include "util/RandomClass.hh"
#include "graphics/ParticleSystemBaseClass.hh"

RadialSpreadParticleSystem::RadialSpreadParticleSystem(
    ID3D11Device* device, const char* texture_filename,
	float particle_size, float particles_per_second,
	float particle_start_velocity,
	float particle_max_degree,
	time_t particle_lifetime,
	bool sort_particle_with_z)
    : ParticleSystem<RadialSpreadParticleSystem>(device, texture_filename,
		particle_size, particles_per_second,
		static_cast<size_t>(particles_per_second * (particle_lifetime * 0.001)),
		sort_particle_with_z)
{
	particle_max_degree_ = XMConvertToRadians(particle_max_degree);
	particle_start_velocity_ = particle_start_velocity;
	particle_lifetime_ = particle_lifetime;
}

ParticleType RadialSpreadParticleSystem::CreateParticle(time_t curr_time)
{
	ParticleType new_particle;

	new_particle.position = XMFLOAT3(0, 0, 0);

	float angle1 = RandomClass::rand(particle_max_degree_);
	float angle2 = RandomClass::rand(XM_PI * 2);

	new_particle.velocity.x = particle_start_velocity_ * cos(angle1);
	new_particle.velocity.y = particle_start_velocity_ * sin(angle1) * cos(angle2);
	new_particle.velocity.z = particle_start_velocity_ * sin(angle1) * sin(angle2);

	const float alpha = RandomClass::rand(0.0f, 0.5f);
	new_particle.red = new_particle.green = new_particle.blue = alpha;

	new_particle.created_time = curr_time;

	return new_particle;
}

bool RadialSpreadParticleSystem::UpdateParticle(ParticleType& particle, time_t curr_time, time_t time_delta)
{
	particle.position.x += particle.velocity.x * time_delta;
	particle.position.y += particle.velocity.y * time_delta;
	particle.position.z += particle.velocity.z * time_delta;
	
	return particle.created_time + particle_lifetime_ > curr_time;
}
