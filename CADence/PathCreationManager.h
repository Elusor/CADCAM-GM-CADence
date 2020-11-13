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

	void RenderGui(std::unique_ptr<RenderState>& renderState);
	void ExecuteRenderPass(std::unique_ptr<RenderState>& renderState);
	void CreateMillingModel();

private:
	float m_offset;
	float m_zNear;
	float m_zFar;
	UINT m_resolution;
	Scene* m_scene;
	std::unique_ptr<MillingHullRenderPass> m_millingHullPass;
	std::unique_ptr<PathModel> m_model;
	
	void ParseDepthTexture(std::unique_ptr<RenderState>& renderState);
	float LinearizeDepth(float uNormDepth);
};