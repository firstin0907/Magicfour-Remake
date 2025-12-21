#include "graphics/particle-system/ParticleType.hh"

#include "util/RandomClass.hh"

struct SimpleParticle : public ParticleType
{
	bool ShouldDie() const override
	{
		return positionY < -3.0f;
	}

	void Update(float elapsed_time) override
	{
		positionY -= (velocity * elapsed_time * 0.0000001f);
	}
};


/// A particle that spreads out radially within a certain angle range and speed.
/// @tparam ANGLE angle in degrees.
/// @tparam SPEED Base speed of the particle. (pixels per millisecond)
template<int ANGLE, int SPEED>
struct RadialSpreadParticle : public ParticleType
{
    float velocity_x, velocity_y, velocity_z;

	RadialSpreadParticle()
	{
        float xy_rad = (RandomClass::rand(ANGLE) * XM_PI) / 180.0f; // Convert to radians
        float z_rad = (RandomClass::rand(ANGLE) - ANGLE / 2) * XM_PI / 180.0f; // Convert to radians

        velocity_x = SPEED * cos(z_rad) * cos(xy_rad);
        velocity_y = SPEED * sin(z_rad);
        velocity_z = SPEED * cos(z_rad) * sin(xy_rad);
	}

	bool ShouldDie() const override
	{
		return positionY < -3.0f;
	}

	void Update(float elapsed_time) override
	{
        positionX += velocity_x * elapsed_time * 0.0000001f;
        positionY += velocity_y * elapsed_time * 0.0000001f;
        positionZ += velocity_z * elapsed_time * 0.0000001f;
	}
};