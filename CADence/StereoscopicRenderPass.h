#pragma once
#include "IRenderPass.h"
#include "TextureRenderTarget.h"
#include "BackBufferRenderTarget.h"

class StereoscopicRenderPass : public IRenderPass
{
	TextureRenderTarget* tex1;
	TextureRenderTarget* tex2;
	BackBufferRenderTarget* target;
	// Inherited via IRenderPass

	// Inherited via IRenderPass
	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;

	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

};