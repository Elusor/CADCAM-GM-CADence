#pragma once
#include <memory>
#include "imgui.h"
#include "renderState.h"
#include "Scene.h"
#include "MillingHullRenderPass.h"

class PathCreationManager
{
public:	
	PathCreationManager(std::unique_ptr<RenderState>& renderState);

	void RenderGui();
	void ExecuteRenderPass(std::unique_ptr<RenderState>& renderState, Scene* scene);

private:
	float m_offset;
	UINT resolution;
	MillingHullRenderPass* m_millingHullPass;
};