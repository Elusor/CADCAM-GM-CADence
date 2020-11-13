#include "PathCreationManager.h"
#include "MillingHullRenderPass.h"

PathCreationManager::PathCreationManager(std::unique_ptr<RenderState>& renderState)
{
	m_offset = 0.0f;
	float cameraSizeLength = 15.f;
	m_millingHullPass = new MillingHullRenderPass(renderState, cameraSizeLength, 1000);
}

void PathCreationManager::RenderGui()
{
	if (ImGui::DragFloat("Hull offsetm##PathCreationManager", &m_offset, 0.05))
	{
		m_millingHullPass->SetOffset(m_offset);
	}
}

void PathCreationManager::ExecuteRenderPass(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	// Todo render scene without grid, points etc. only Model and the block

	m_millingHullPass->Execute(renderState, scene);
}
