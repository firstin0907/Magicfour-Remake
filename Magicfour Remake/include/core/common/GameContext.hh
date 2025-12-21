#include "util/ResourceMap.hh"

struct RenderContext
{
	ResourceMap<class ModelClass>&				models_;
	ResourceMap<class TextureClass>&			textures_;
	ResourceMap<class ParticleSystemBaseClass>& particle_system_;
};