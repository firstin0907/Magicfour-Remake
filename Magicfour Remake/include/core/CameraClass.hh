#pragma once

#include <directxmath.h>

class CameraClass
{
	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;

public:
	/// @brief Constructs a CameraClass object with the specified field of view, screen aspect ratio, near clipping plane, and far clipping plane.
	/// @param screen_width 	width of the screen in pixels
	/// @param screen_height 	height of the screen in pixels
	/// @param field_of_view 	field of view in radians
	/// @param screen_near 		distance to the near clipping plane
	/// @param screen_far 		distance to the far clipping plane
	CameraClass(float screen_width, float screen_height, float field_of_view, float screen_near, float screen_far);
	CameraClass(const CameraClass&) = delete;
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition() { return XMFLOAT3(positionX_, positionY_, positionZ_); }
	XMFLOAT3 GetRotation() { return XMFLOAT3(rotationX_, rotationY_, rotationZ_); }

	void Render();
	XMMATRIX GetViewMatrix() { return view_matrix_; }
	XMMATRIX GetProjectionMatrix() { return projection_matrix_; }
	XMMATRIX GetOrthoMatrix() { return ortho_matrix_; }

private:
	float positionX_, positionY_, positionZ_;
	float rotationX_, rotationY_, rotationZ_;

	XMMATRIX view_matrix_;
	XMMATRIX projection_matrix_;
	XMMATRIX ortho_matrix_;
};