#pragma once
#include "IRenderPass.h"
#include "TextureRenderTarget.h"
#include "BackBufferRenderTarget.h"

class StereoscopicRenderPass : public IRenderPass
{
public:
	TextureRenderTarget* m_tex1;
	TextureRenderTarget* m_tex2;
	BackBufferRenderTarget* m_backTarget;

	StereoscopicRenderPass(const std::unique_ptr<RenderState>& renderState, SIZE wndSize);

	// Inherited via IRenderPass
	virtual void Execute(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

	virtual void Clear(std::unique_ptr<RenderState>& renderState) override;

	virtual void Render(std::unique_ptr<RenderState>& renderState, Scene* scene) override;

private: 
	mini::dx_ptr<ID3D11VertexShader> m_texVS;
	mini::dx_ptr<ID3D11PixelShader> m_texPS;
	mini::dx_ptr<ID3D11SamplerState> m_sampler;
	
	std::vector<VertexPositionColor> m_quadVerts;
	std::vector<unsigned short> m_quadIndices;

	void ClearDepth(std::unique_ptr<RenderState>& renderState);
	void DrawTexturedQuad(std::unique_ptr<RenderState>& renderState);
};