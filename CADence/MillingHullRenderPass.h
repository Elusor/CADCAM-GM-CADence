#pragma once
#include "IRenderPass.h"
#include "orthographicCamera.h"
#include "TextureRenderTarget.h"
#include <memory>

using namespace mini;

class MillingHullRenderPass : public IRenderPass
{
public:
	MillingHullRenderPass(
		std::unique_ptr<RenderState>& renderState,
		float cameraSide, 
		UINT resolution);

	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;
	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;
	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

	void SetOffset(float val);
	float GetOffset();

private:
	float m_offset = 0.0f;
	D3D11_VIEWPORT m_viewPort;
	std::unique_ptr<TextureRenderTarget> m_texture;
	std::unique_ptr<OrthographicCamera> m_camera;
	
	void CreateDepthStencil(
		std::unique_ptr<RenderState>& renderState, 
		UINT resolution, float minZ, float maxZ);
	void CreateViewport(UINT resolution);

	mini::dx_ptr<ID3D11Texture2D> m_depthTex;
	mini::dx_ptr<ID3D11DepthStencilView> m_dsv;
};