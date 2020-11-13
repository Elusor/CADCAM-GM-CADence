#pragma once
#include <memory>
#include "imgui.h"
#include "renderState.h"
#include "Scene.h"
#include "MillingHullRenderPass.h"
#include "PathModel.h"

class PathCreationManager
{
public:	
	PathCreationManager(std::unique_ptr<RenderState>& renderState, Scene* scene);

	void RenderGui();
	void ExecuteRenderPass(std::unique_ptr<RenderState>& renderState);
	void CreateMillingModel();
private:
	float m_offset;
	UINT resolution;

	Scene* m_scene;
	MillingHullRenderPass* m_millingHullPass;
	std::unique_ptr<PathModel> m_model;
};