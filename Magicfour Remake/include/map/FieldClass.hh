#pragma once

#include <vector>

#include "../core/interface/IDrawable.hh"

#include "GroundClass.hh"

class FieldClass : public IDrawable
{
public:
	FieldClass(const char* filename);
	~FieldClass() = default;

	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		class GraphicResources* graphic_resources) const;

	// Check collision between the rigid body and grounds, and update the rigid body's position and velocity.
	/// @return true if the rigid body is collided with any ground, false otherwise.
	bool IsCollided(int x1, int x2, int from_bottom_coord, int to_bottom_coord, int* result = nullptr) const;

	inline const std::vector<GroundClass>& GetGrounds() const
	{
		return grounds_;
	}

private:
	std::vector<GroundClass> grounds_;
};