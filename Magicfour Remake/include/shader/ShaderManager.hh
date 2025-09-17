#pragma once

#include <memory>
#include <Windows.h>

class ShaderManager
{
public:
	std::unique_ptr<class LightShaderClass>		light_shader_;
	std::unique_ptr<class StoneShaderClass>		stone_shader_;
	std::unique_ptr<class NormalMapShaderClass>	normalMap_shader_;
	std::unique_ptr<class FireShaderClass>		fire_shader_;

	ShaderManager(class ID3D11Device* device, class ID3D11DeviceContext* device_context, HWND hwnd);
};