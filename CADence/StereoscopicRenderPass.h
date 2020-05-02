#pragma once
#include "IRenderPass.h"
#include "TextureRenderTarget.h"
#include "BackBufferRenderTarget.h"

class StereoscopicRenderPass : public IRenderPass
{
	TextureRenderTarget* m_tex1;
	TextureRenderTarget* m_tex2;
	BackBufferRenderTarget* m_backTarget;

	StereoscopicRenderPass(const std::unique_ptr<RenderState>& renderState, SIZE wndSize);

	// Inherited via IRenderPass
	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;

	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

};