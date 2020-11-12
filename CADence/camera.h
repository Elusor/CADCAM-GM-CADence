#pragma once
#include <DirectXMath.h>
#include <windows.h>
#include "ICamera.h"

class Camera : public ICamera
{
public:
	DirectX::XMMATRIX m_projMat;

	Camera();
	Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT2 rotation, 
		uint32_t width, uint32_t height, float fovAngle, float zNear, float zFar);

	virtual DirectX::XMMATRIX GetViewMatrix() override;
	virtual DirectX::XMMATRIX GetViewProjectionMatrix() override;

	DirectX::XMFLOAT4 GetCameraPosition();
	float GetZNear();
	float GetZFar();
	void ResetCamera();
	void UpdateUpVector();
	void CameraZoom(float delta);
	void RecalculateProjectionMatrix();
	void RotateCamera(float dx, float dy);
	void TranslateCamera(float dx, float dy);
	void ResizeViewport(float width, float height);
	DirectX::XMFLOAT3 ScreenToWorldPoint(int x, int y);
	SIZE GetViewportSize();	
	DirectX::XMMATRIX GetStereoscopicMatrix(bool isLeft, float d, float focusPlaneDist);

	DirectX::XMFLOAT4 ConvertScreenPosToNDC(int x, int y);
	DirectX::XMFLOAT3 GetTargetPos();

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