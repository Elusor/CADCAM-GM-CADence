#include "orthographicCamera.h"

using namespace DirectX::SimpleMath;

OrthographicCamera::OrthographicCamera(
    float width, float height, 
    float zNear, float zFar,
    DirectX::SimpleMath::Vector3 pos,
    DirectX::SimpleMath::Vector3 target
    )
{
    m_width = width;
    m_height = height;
    m_aspectRatio = (float)width / (float)height;

    m_zFar = zFar;
    m_zNear = zNear;

    m_pos = pos;
    m_targetPos = target;

    RecalculateViewMatrix();
    RecalculateProjectionMatrix();
}

DirectX::XMMATRIX OrthographicCamera::GetViewMatrix()
{    
    return m_viewMatrix;
}

DirectX::XMMATRIX OrthographicCamera::GetViewProjectionMatrix()
{
    return m_VPMatrix;
}

void OrthographicCamera::ResizeViewportSize(float width, float height)
{
    m_width = width;
    m_height = height;
    RecalculateProjectionMatrix();
}

void OrthographicCamera::ResetZBounds(float zNear, float zFar)
{
    m_zFar = zFar;
    m_zNear = zNear;
    RecalculateProjectionMatrix();
}

void OrthographicCamera::RecalculateProjectionMatrix()
{
    m_projMatrix = DirectX::XMMatrixOrthographicLH(m_width, m_height, m_zNear, m_zFar);
    UpdateVPMatix();
}

void OrthographicCamera::RecalculateViewMatrix()
{
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    m_viewMatrix = DirectX::XMMatrixLookAtLH(m_pos, m_targetPos, up);
    UpdateVPMatix();
}

void OrthographicCamera::UpdateVPMatix()
{
    m_VPMatrix = m_viewMatrix * m_projMatrix;
}
