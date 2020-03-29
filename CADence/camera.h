#pragma once
#include <DirectXMath.h>
#include <windows.h>
class Camera
{
public:
	DirectX::XMMATRIX m_projMat;

	Camera();
	Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT2 rotation, 
		uint32_t width, uint32_t height, float fovAngle, float zNear, float zFar);

	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetViewProjectionMatrix();

	void ResetCamera();
	void UpdateUpVector();
	void CameraZoom(float delta);
	void RecalculateProjectionMatrix();
	void RotateCamera(float dx, float dy);
	void TranslateCamera(float dx, float dy);
	void ResizeViewport(float width, float height);
	SIZE GetViewportSize();	

private:	
	// Calculate view matrix based on the cmaera position and target position

	DirectX::XMFLOAT3 m_pos;
	float m_yaw;
	float m_pitch;
	float m_fov;
	float m_aspectRatio;
	float m_zNear;
	float m_zFar;

	uint32_t m_width;
	uint32_t m_height;

	DirectX::XMFLOAT3 m_target;
	DirectX::XMFLOAT3 m_right;
	DirectX::XMFLOAT3 m_up;

};