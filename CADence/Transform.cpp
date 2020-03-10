#include "Transform.h"

Transform::Transform() : Transform(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
{
}

Transform::Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale)
{
	m_pos = position;
	m_rotation = rotation;
	m_scale = scale;
}

DirectX::XMMATRIX Transform::GetModelMatrix()
{
	auto matrix = DirectX::XMMatrixIdentity() *
		DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
		DirectX::XMMatrixRotationX(m_rotation.x) *
		DirectX::XMMatrixRotationY(m_rotation.y) *
		DirectX::XMMatrixRotationZ(m_rotation.z) *
		DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
	return matrix;		
}
