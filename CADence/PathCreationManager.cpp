#include "PathCreationManager.h"
#include "MillingHullRenderPass.h"

PathCreationManager::PathCreationManager(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	m_offset = 0.0f;
	m_scene = scene;
	float cameraSizeLength = 15.f;
	m_millingHullPass = new MillingHullRenderPass(renderState, cameraSizeLength, 1000);

	m_model = std::make_unique<PathModel>();
}

void PathCreationManager::RenderGui()
{
	if (ImGui::DragFloat("Hull offsetm##PathCreationManager", &m_offset, 0.05))
	{
		m_millingHullPass->SetOffset(m_offset);
	}

	if (ImGui::Button("Create milling model"))
	{
		CreateMillingModel();
	}

	ImGui::Text("Current milling model elements:");
	for (auto name : m_model->GetObjectNames())
	{
		ImGui::Text(name.c_str());
	}
}

void PathCreationManager::ExecuteRenderPass(std::unique_ptr<RenderState>& renderState)
{
	// Todo render scene without grid, points etc. only Model and the block
	m_millingHullPass->Execute(renderState, m_model.get());
}

void PathCreationManager::CreateMillingModel()
{	
	m_model->SetModelObjects(m_scene->m_selectedNodes);
}
