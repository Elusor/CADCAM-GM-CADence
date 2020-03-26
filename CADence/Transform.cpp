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

Transform& Transform::operator=(Transform& other)
{
	if (this != &other)
	{
		this->m_pos = other.GetPosition();
		this->m_rotation = other.GetRotation();
		this->m_scale = other.GetScale();
	}
	return (*this);
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

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return m_pos;
}

DirectX::XMFLOAT3 Transform::GetRotation()
{
	return m_rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return m_scale;
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	SetPosition(
		position.x,
		position.y,
		position.z);
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	SetRotation(
		rotation.x,
		rotation.y,
		rotation.z);
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	SetScale(
		scale.x,
		scale.y,
		scale.z);
}

void Transform::SetPosition(float x, float y, float z)
{
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;
}

void Transform::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}

void Transform::SetScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;
}

void Transform::Translate(float x, float y, float z)
{
	m_pos.x += x;
	m_pos.y += y;
	m_pos.z += z;
}

void Transform::Rotate(float x, float y, float z)
{
	m_rotation.x += x;
	m_rotation.y += y;
	m_rotation.z += z;
}

void Transform::Scale(float x, float y, float z)
{
	m_scale.x *= x;
	m_scale.y *= y;
	m_scale.z *= z;
}

void Transform::Translate(DirectX::XMFLOAT3 position)
{
	Translate(
		position.x,
		position.y,
		position.z);
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	Rotate(
		rotation.x,
		rotation.y,
		rotation.z);
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	Scale(
		scale.x,
		scale.y,
		scale.z);
}
