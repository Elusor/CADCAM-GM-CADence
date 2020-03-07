#include "camera.h"
using namespace DirectX;

XMFLOAT3 SumFloat3(XMFLOAT3 v1, XMFLOAT3 v2);
XMFLOAT3 Float3TimesFloat(XMFLOAT3 v1, float a);

Camera::Camera() : Camera(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), 0, 0, 0, 0, 0)
{}

Camera::Camera(XMFLOAT3 position, XMFLOAT3 target, XMFLOAT2 rotation, uint32_t width, uint32_t height, float fovAngle, float zNear, float zFar)
{
	m_aspectRatio = (float)width / (float)height;
	m_fov = fovAngle;

	m_pos = position;
	m_target = target;

	m_zNear = zNear;
	m_zFar = zFar;

	m_yaw = rotation.x;
	m_pitch = rotation.y;

	UpdateUpVector();
	RecalculateProjectionMatrix();
	RotateCamera(0, 0);
}

void Camera::UpdateUpVector()
{
	//Get the LookAtDirection
	XMFLOAT3 LookAt = SumFloat3(m_target, Float3TimesFloat(m_pos, -1.0f));	
	/*LookAt.x = m_target.x - m_pos.x;
	LookAt.y = m_target.y - m_pos.y;
	LookAt.z = m_target.z - m_pos.z;*/

	XMVECTOR LookAtDir = XMLoadFloat3(&LookAt);

	//Get the Y direction	
	XMFLOAT3 globalUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR GlobalUpDir = XMLoadFloat3(&globalUp);

	//Calculate Vector parallel to XZ plane
	XMVECTOR cameraRightDir = XMVector3Cross(GlobalUpDir, LookAtDir);
	XMStoreFloat3(&m_right, cameraRightDir);

	//Calculate Up Vector
	XMVECTOR upDir = XMVector3Cross(LookAtDir, cameraRightDir);
	XMStoreFloat3(&m_up, XMVector3Normalize(upDir));
}

XMMATRIX Camera::GetViewMatrix()
{	
	XMVECTOR pos = XMLoadFloat3(&m_pos);
	XMVECTOR target = XMLoadFloat3(&m_target);
	XMVECTOR up = XMLoadFloat3(&m_up);
	return XMMatrixLookAtLH(pos, target, up);
}

void Camera::RecalculateProjectionMatrix()
{	
	m_projMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), m_aspectRatio, m_zNear, m_zFar);
}

DirectX::XMMATRIX Camera::GetViewProjectionMatrix()
{
	return GetViewMatrix() * m_projMat;
}

void Camera::ResetCamera()
{
	m_pos = XMFLOAT3(0.0f, 0.0f, 10.0f);
	m_target = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void Camera::CameraZoom(float delta)
{
	float sensitivity = 0.2f;
	
	XMFLOAT3 cameraDirection;

	//XMFLOAT3 negTargetDir = Float3TimesFloat(m_target, -1.0f);
	cameraDirection.x = ((m_pos.x - m_target.x) * sensitivity * delta) + m_target.x;
	cameraDirection.y = ((m_pos.y - m_target.y) * sensitivity * delta) + m_target.y;
	cameraDirection.z = ((m_pos.z - m_target.z) * sensitivity * delta) + m_target.z;
	
	//Scale camera direction	
	m_pos = cameraDirection;
}

void Camera::RotateCamera(float dx, float dy)
{
	float sensitivity = 0.005f;
	float yawIncrement = dx * sensitivity;
	float pitchIncrement = dy * sensitivity;

	m_yaw += yawIncrement;
	m_pitch += pitchIncrement;

	XMFLOAT3 relativeDir;

	relativeDir = SumFloat3(m_pos, Float3TimesFloat(m_target, -1.0f));
	/*relativeDir.x = m_pos.x - m_target.x;
	relativeDir.y = m_pos.y - m_target.y;
	relativeDir.z = m_pos.z - m_target.z;*/

	XMVECTOR relativeDirVect = XMLoadFloat3(&relativeDir);
	relativeDirVect = XMVector3Rotate(relativeDirVect, 
		XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(m_pitch), 
			XMConvertToRadians(m_yaw), 
			0.0f));
	
	XMFLOAT3 relativeDirRotated;
	XMStoreFloat3(&relativeDirRotated, relativeDirVect);

	m_pos = SumFloat3(m_target, Float3TimesFloat(relativeDirRotated, -1.0f));
	/*m_pos.x = m_target.x - relativeDirRotated.x;
	m_pos.y = m_target.y - relativeDirRotated.y;
	m_pos.z = m_target.z - relativeDirRotated.z;*/
	UpdateUpVector();
}

void Camera::TranslateCamera(float dx, float dy)
{
	//Translate Camera in m_up m_right plane
	float sensitivity = 0.005f;
	XMFLOAT3 cameraDelta = XMFLOAT3(
		dx * m_right.x + dy * m_up.x,
		dx * m_right.y + dy * m_up.y,
		dx * m_right.z + dy * m_up.z);
		
	m_pos = SumFloat3(m_pos,cameraDelta);
	m_target = SumFloat3(m_target, cameraDelta);
}

void Camera::ResizeViewport(float width, float height)
{
	m_aspectRatio = width / height;
	RecalculateProjectionMatrix();
}

XMFLOAT3 SumFloat3(XMFLOAT3 v1, XMFLOAT3 v2)
{
	XMVECTOR resV = XMVectorAdd(XMLoadFloat3(&v1), XMLoadFloat3(&v2));	
	XMFLOAT3 res;
	XMStoreFloat3(&res, resV);
	return res;
}

XMFLOAT3 Float3TimesFloat(XMFLOAT3 v1, float a)
{	
	XMFLOAT3 res = XMFLOAT3(v1.x * a, v1.y * a, v1.z * a);	
	return res;
}