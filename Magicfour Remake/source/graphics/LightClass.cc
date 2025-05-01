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
	ambientColor_ = { r, g, b, a };
}

void LightClass::SetDiffuseColor(float r, float g, float b, float a)
{
	diffuseColor_ = { r, g, b, a };
}

void LightClass::SetDirection(float x, float y, float z)
{
	direction_ = { x, y, z };
}

void LightClass::SetSpecularColor(float r, float g, float b, float a)
{
	specularColor_ = { r, g, b, a };
}

void LightClass::SetSpecularPower(float power)
{
	specularPower_ = power;
}

void LightClass::SetPosition(float x, float y, float z)
{
	position_ = { x, y, z, 1.0f };
}

XMFLOAT4 LightClass::GetAmbientColor()
{
	return ambientColor_;
}

XMFLOAT4 LightClass::GetDiffuseColor()
{
	return diffuseColor_;
}

XMFLOAT3 LightClass::GetDirection()
{
	return direction_;
}

XMFLOAT4 LightClass::GetSpecularColor()
{
	return specularColor_;
}

float LightClass::GetSpecularPower()
{
	return specularPower_;
}

XMFLOAT4 LightClass::GetPosition()
{
	return position_;
}
