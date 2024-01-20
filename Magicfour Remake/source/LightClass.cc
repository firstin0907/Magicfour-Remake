#include "../include/LightClass.hh"

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
	m_ambientColor = { r, g, b, a };
}

void LightClass::SetDiffuseColor(float r, float g, float b, float a)
{
	m_diffuseColor = { r, g, b, a };
}

void LightClass::SetDirection(float x, float y, float z)
{
	m_direction = { x, y, z };
}

void LightClass::SetSpecularColor(float r, float g, float b, float a)
{
	m_specularColor = { r, g, b, a };
}

void LightClass::SetSpecularPower(float power)
{
	m_specularPower = power;
}

void LightClass::SetPosition(float x, float y, float z)
{
	m_position = { x, y, z, 1.0f };
}

XMFLOAT4 LightClass::GetAmbientColor()
{
	return m_ambientColor;
}

XMFLOAT4 LightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}

XMFLOAT3 LightClass::GetDirection()
{
	return m_direction;
}

XMFLOAT4 LightClass::GetSpecularColor()
{
	return m_specularColor;
}

float LightClass::GetSpecularPower()
{
	return m_specularPower;
}

XMFLOAT4 LightClass::GetPosition()
{
	return m_position;
}
