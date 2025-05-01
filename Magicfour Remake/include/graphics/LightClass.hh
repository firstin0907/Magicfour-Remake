#pragma once
#include <directxmath.h>

class LightClass
{
private:
    using XMFLOAT3 = DirectX::XMFLOAT3;
    using XMFLOAT4 = DirectX::XMFLOAT4;

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
    XMFLOAT4 ambientColor_;
    XMFLOAT4 diffuseColor_;
    XMFLOAT3 direction_;
    XMFLOAT4 specularColor_;
    float specularPower_;
    XMFLOAT4 position_;
};