#pragma once
#include <d3d11.h>
#include "renderState.h"

__interface IRenderTarget
{
	void Dispose();
	void SetRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView);
	void ClearRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView, float r, float g, float b, float alpha, float depth = 1.0f);
};