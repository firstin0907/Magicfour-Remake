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
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const;

	inline const std::vector<GroundClass>& GetGrounds() const
	{
		return grounds_;
	}

private:
	std::vector<GroundClass> grounds_;
};