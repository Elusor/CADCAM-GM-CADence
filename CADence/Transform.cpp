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

Transform& Transform::operator+(Transform&& other)
{
	Transform t1 = *this;
	Transform t2 = other;

	Transform res = Transform(
		DirectX::XMFLOAT3(
			t1.m_pos.x + t2.m_pos.x,
			t1.m_pos.y + t2.m_pos.y,
			t1.m_pos.z + t2.m_pos.z),
		DirectX::XMFLOAT3(
			t1.m_rotation.x + t2.m_rotation.x,
			t1.m_rotation.y + t2.m_rotation.y, 
			t1.m_rotation.z + t2.m_rotation.z),
		DirectX::XMFLOAT3(
			t1.m_scale.x + t2.m_scale.x,
			t1.m_scale.y + t2.m_scale.y,
			t1.m_scale.z + t2.m_scale.z)
	);

	return res;
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
