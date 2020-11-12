#pragma once
#include <d3d11.h>
#include "SimpleMath.h"
#include "ICamera.h"

class OrthographicCamera: public ICamera
{
public:
	OrthographicCamera(
		float width, float height, 
		float zNear, float zFar,
		DirectX::SimpleMath::Vector3 pos, 
		DirectX::SimpleMath::Vector3 target
		);

	virtual DirectX::XMMATRIX GetViewMatrix() override;
	virtual DirectX::XMMATRIX GetViewProjectionMatrix() override;

	void ResizeViewportSize(float width, float height);
	void ResetZBounds(float zNear, float zFar);

	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();
private:
	DirectX::SimpleMath::Matrix m_viewMatrix;
	DirectX::SimpleMath::Matrix m_projMatrix;
	DirectX::SimpleMath::Matrix m_VPMatrix;

	DirectX::SimpleMath::Vector3 m_pos;
	DirectX::SimpleMath::Vector3 m_targetPos;

	float m_width;
	float m_height;
	float m_aspectRatio;

	float m_zNear;
	float m_zFar;

	void UpdateVPMatix();
};