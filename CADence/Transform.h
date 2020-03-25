#pragma once
#include <DirectXMath.h>
// Struct which represents object's position, rotation and scale. All angles values are reprented in radians.
struct Transform
{
public:		
	Transform();
	Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);

	Transform& operator+(Transform&& other);

	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;

	DirectX::XMMATRIX GetModelMatrix();

};