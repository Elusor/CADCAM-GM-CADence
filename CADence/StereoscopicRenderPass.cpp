#include "StereoscopicRenderPass.h"

void StereoscopicRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	//// initialize all targets
	//tex1->Initialize();
	//tex2->Initialize();
	//target->Initialize();

	//// Render to the first texture
	//Clear();
	//Render();
	//tex1->SetRenderTarget();

	//// Render to the second texture
	//Clear();
	//Render();
	//tex2->SetRenderTarget();

	//// Blend both textures and display them
	//Clear();
	//Render();
	//target->SetRenderTarget();
}

void StereoscopicRenderPass::Clear(std::unique_ptr<RenderState>& renderState)
{
}

void StereoscopicRenderPass::Render(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
}
