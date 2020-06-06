#pragma once
#include "dxApplication.h"
#include "IRenderPass.h"
#include "BackBufferRenderTarget.h"

using namespace mini;

class DefaultRenderPass : public IRenderPass
{
public:
	DefaultRenderPass(const std::unique_ptr<RenderState>& renderState, SIZE wndSize);
	BackBufferRenderTarget* m_renderTarget;

	// Inherited via IRenderPass
	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;
	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;
	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;
private:
	dx_ptr<ID3D11BlendState> m_blendState;
};