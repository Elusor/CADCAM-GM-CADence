#pragma once
#include <d3d11.h>
#include "renderState.h"
#include "Scene.h"
#include <direct.h>

using namespace DirectX;

__interface IRenderPass
{
	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene);
	virtual void Clear(std::unique_ptr<RenderState>& renderState);
	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene);
};