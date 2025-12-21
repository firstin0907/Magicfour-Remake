#pragma once

#include <DirectXMath.h>

struct ParticleType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	float red, green, blue;

	time_t created_time;
};
