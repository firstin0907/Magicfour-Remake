#pragma once
#include <directxmath.h>
using namespace DirectX;

class LightClass
{
public:
    LightClass();
    LightClass(const LightClass&);
    ~LightClass();

    void SetAmbientColor(float r, float g, float b, float a);
    void SetDiffuseColor(float r, float g, float b, float a);
    void SetDirection(float x, float y, float z);
    void SetSpecularColor(float r, float g, float b, float a);
    void SetSpecularPower(float power);
    void SetPosition(float x, float y, float z);
    
    XMFLOAT4 GetAmbientColor();
    XMFLOAT4 GetDiffuseColor();
    XMFLOAT3 GetDirection();
    XMFLOAT4 GetSpecularColor();
    float GetSpecularPower();
    XMFLOAT4 GetPosition();

private:
    XMFLOAT4 m_ambientColor;
    XMFLOAT4 m_diffuseColor;
    XMFLOAT3 m_direction;
    XMFLOAT4 m_specularColor;
    float m_specularPower;
    XMFLOAT4 m_position;
};