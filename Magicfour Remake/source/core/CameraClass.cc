#include "core/CameraClass.hh"

using namespace DirectX;

CameraClass::CameraClass(float screen_width, float screen_height, float field_of_view, float screen_near, float screen_far)
{
	positionX_ = positionY_ = positionZ_ = 0.0f;
	rotationX_ = rotationY_ = rotationZ_ = 0.0f;

	// Create the projection matrix for 3D rendering.
	const float screen_aspect = screen_width / screen_height;
	projection_matrix_ = XMMatrixPerspectiveFovLH(field_of_view, screen_aspect, screen_near, screen_far);
	ortho_matrix_ = XMMatrixOrthographicLH(
		static_cast<float>(screen_width), static_cast<float>(screen_height), -1, 1);
}

CameraClass::~CameraClass()
{
}

void CameraClass::SetPosition(float x, float y, float z)
{
	positionX_ = x;
	positionY_ = y;
	positionZ_ = z;
}

void CameraClass::SetRotation(float x, float y, float z)
{
	rotationX_ = x;
	rotationY_ = y;
	rotationZ_ = z;
}

void CameraClass::Render()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVector, positionVector, lookAtVector;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Load it into a XMVECTOR structure.
	upVector = XMLoadFloat3(&up);

	// Setup the position of the camera in the world.
	position.x = positionX_;
	position.y = positionY_;
	position.z = positionZ_;

	// Load it into a XMVECTOR structure.
	positionVector = XMLoadFloat3(&position);

	// Setup where the camera is looking by default.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// Load it into a XMVECTOR structure.
	lookAtVector = XMLoadFloat3(&lookAt);

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = rotationX_ * 0.0174532925f;
	yaw = rotationY_ * 0.0174532925f;
	roll = rotationZ_ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// Finally create the view matrix from the three updated vectors.
	view_matrix_ = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}
