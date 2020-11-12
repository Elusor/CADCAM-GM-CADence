#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

__interface ICamera
{
public:
	virtual DirectX::XMMATRIX GetViewMatrix() = 0;
	virtual DirectX::XMMATRIX GetViewProjectionMatrix() = 0;
};