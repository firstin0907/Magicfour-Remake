#include "graphics/particle-system/LinearMoveParticleSystem.hh"
#include "util/RandomClass.hh"
#include "graphics/ParticleSystemBaseClass.hh"

LinearMoveParticleSystem::LinearMoveParticleSystem(
	ID3D11Device* device, const char* texture_filename,
	float particle_size, float particles_per_second, size_t max_particle_num,
	XMFLOAT3 start_position_coord_min, XMFLOAT3 start_position_coord_max,
	float particle_start_velocity_min, float particle_start_velocity_max)
	: ParticleSystem<LinearMoveParticleSystem>(device, texture_filename,
		particle_size, particles_per_second, max_particle_num, false)
{
	start_position_coord_min_ = start_position_coord_min;
	start_position_coord_max_ = start_position_coord_max;
	particle_start_velocity_min_ = particle_start_velocity_min;
	particle_start_velocity_max_ = particle_start_velocity_max;
}

ParticleType LinearMoveParticleSystem::CreateParticle(time_t curr_time)
{
	ParticleType new_particle;

	new_particle.position.x = RandomClass::rand(start_position_coord_min_.x, start_position_coord_max_.x);
	new_particle.position.y = RandomClass::rand(start_position_coord_min_.y, start_position_coord_max_.y);
	new_particle.position.z = RandomClass::rand(start_position_coord_min_.z, start_position_coord_max_.z);

	new_particle.velocity.x = RandomClass::rand(particle_start_velocity_min_, particle_start_velocity_max_);
	new_particle.created_time = curr_time;

	new_particle.red = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	new_particle.green = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	new_particle.blue = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;

	return new_particle;
}

bool LinearMoveParticleSystem::UpdateParticle(ParticleType& particle,
	time_t curr_time, time_t time_delta)
{
	particle.position.x += particle.velocity.x * time_delta;

	return particle.position.x < 13.0f;
}
