#pragma once
#include "IRenderPass.h"
#include "orthographicCamera.h"
#include "TextureRenderTarget.h"
#include "PathModel.h"
#include <memory>

using namespace mini;

class MillingHullRenderPass : public IRenderPass
{
public:
	MillingHullRenderPass(
		std::unique_ptr<RenderState>& renderState,
		float cameraSide,
		float zNear,
		float zFar,
		UINT resolution);

	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;
	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;
	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

	void Execute(std::unique_ptr<RenderState>& renderState, PathModel* model);
	void Render(std::unique_ptr<RenderState>& renderState, PathModel* model, bool baseOnly = false);

	void SetOffset(float val);
	float GetOffset();

	ID3D11Texture2D* GetDepthTex();
private:
	float m_offset = 0.0f;
	D3D11_VIEWPORT m_viewPort;
	std::unique_ptr<TextureRenderTarget> m_texture;
	std::unique_ptr<OrthographicCamera> m_camera;
	std::unique_ptr<OrthographicCamera> m_cameraCloser;
	void CreateDepthStencil(
		std::unique_ptr<RenderState>& renderState, 
		UINT resolution, float minZ, float maxZ);
	void CreateViewport(UINT resolution);

	mini::dx_ptr<ID3D11Texture2D> m_depthTex;
	mini::dx_ptr<ID3D11DepthStencilView> m_dsv;
};