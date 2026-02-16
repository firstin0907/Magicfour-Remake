#include "graphics/LightClass.hh"

using namespace DirectX;

LightClass::LightClass()
{
}

LightClass::LightClass(const LightClass& other)
{
}

LightClass::~LightClass()
{
}

void LightClass::SetAmbientColor(float r, float g, float b, float a)
{
	ambient_color_ = { r, g, b, a };
}

void LightClass::SetDiffuseColor(float r, float g, float b, float a)
{
	diffuse_color_ = { r, g, b, a };
}

void LightClass::SetDirection(float x, float y, float z)
{
	direction_ = { x, y, z };
}

void LightClass::SetSpecularColor(float r, float g, float b, float a)
{
	specular_color_ = { r, g, b, a };
}

void LightClass::SetSpecularPower(float power)
{
	specular_power_ = power;
}

void LightClass::SetPosition(float x, float y, float z)
{
	position_ = { x, y, z, 1.0f };
}

XMFLOAT4 LightClass::GetAmbientColor()
{
	return ambient_color_;
}

XMFLOAT4 LightClass::GetDiffuseColor()
{
	return diffuse_color_;
}

XMFLOAT3 LightClass::GetDirection()
{
	return direction_;
}

XMFLOAT4 LightClass::GetSpecularColor()
{
	return specular_color_;
}

float LightClass::GetSpecularPower()
{
	return specular_power_;
}

XMFLOAT4 LightClass::GetPosition()
{
	return position_;
}
