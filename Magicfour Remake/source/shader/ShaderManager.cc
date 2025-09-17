#include "shader/ShaderManager.hh"

#include "shader/LightShaderClass.hh"
#include "shader/StoneShaderClass.hh"
#include "shader/NormalMapShaderClass.hh"
#include "shader/FireShaderClass.hh"

ShaderManager::ShaderManager(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
{
	// Create and initialize the light shader object.
	light_shader_		= std::make_unique<LightShaderClass>(device, device_context, hwnd);
	stone_shader_		= std::make_unique<StoneShaderClass>(device, device_context, hwnd);
	normalMap_shader_	= std::make_unique<NormalMapShaderClass>(device, device_context, hwnd);
	fire_shader_		= std::make_unique<FireShaderClass>(device, device_context, hwnd);
}