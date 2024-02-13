#pragma once

#include <directxmath.h>

class CameraClass
{
	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;

public:
	CameraClass();
	CameraClass(const CameraClass&) = delete;
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:
	float positionX_, positionY_, positionZ_;
	float rotationX_, rotationY_, rotationZ_;
	XMMATRIX viewMatrix_;
};