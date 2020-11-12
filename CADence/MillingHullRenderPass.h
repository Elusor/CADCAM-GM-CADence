#pragma once
#include "IRenderPass.h"
#include "orthographicCamera.h"
#include "TextureRenderTarget.h"
#include <memory>

using namespace mini;

class MillingHullRenderPass : public IRenderPass
{
public:
	MillingHullRenderPass(std::unique_ptr<RenderState>& renderState, SIZE wndSize);

	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;
	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;
	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

private:
	std::unique_ptr<TextureRenderTarget> m_texture;
	std::unique_ptr<OrthographicCamera> m_camera;
};