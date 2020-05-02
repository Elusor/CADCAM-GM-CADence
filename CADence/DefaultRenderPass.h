#pragma once
#include "IRenderPass.h"
#include "BackBufferRenderTarget.h"

class DefaultRenderPass : public IRenderPass
{
public:
	BackBufferRenderTarget* m_renderTarget;

	// Inherited via IRenderPass
	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;
	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;
	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;
};