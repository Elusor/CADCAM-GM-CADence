#pragma once
#include <DirectXMath.h>
// Struct which represents object's position, rotation and scale. All angles values are reprented in radians.
struct Transform
{
public:		
	Transform();
	Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);

	Transform& operator+(Transform&& other);
	Transform& operator=(Transform& other);

	DirectX::XMMATRIX GetModelMatrix();

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();
	
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetRotation(DirectX::XMFLOAT3 rotation);
	void SetScale(DirectX::XMFLOAT3 scale);

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	void Translate(float x, float y, float z);
	void Rotate(float x, float y, float z);
	void Scale(float x, float y, float z);

	void Translate(DirectX::XMFLOAT3 position);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(DirectX::XMFLOAT3 scale);

private:
	
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;
	

};