#pragma once
#include "util/ResourceMap.hh"

struct GraphicResources
{
    ResourceMap<class ModelClass>		models_;
	ResourceMap<class FbxModel>			fbx_models_;
	ResourceMap<class TextureClass>		textures_;
	ResourceMap<class ParticleSystemBaseClass> particle_system_;
};