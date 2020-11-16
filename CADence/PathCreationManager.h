#pragma once
#include <memory>
#include "SimpleMath.h"
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

	float m_millRadius;

	float m_blockBaseHeight;
	float m_blockSide;
	float m_modelDepth;
	float m_radius;

	float m_offset;
	float m_zNear;
	float m_zFar;
	UINT m_resolution;
	UINT m_instructionCounter = 1;

	Scene* m_scene;
	std::unique_ptr<MillingHullRenderPass> m_millingHullPass;
	std::unique_ptr<PathModel> m_model;
	
	void ParseDepthTexture(std::unique_ptr<RenderState>& renderState);
	float LinearizeDepth(float uNormDepth);
	
	
	std::vector<DirectX::SimpleMath::Vector3> GeneratePath(std::vector<float>& heights);
	bool SavePathToFile(std::vector<float>& heights);
	void PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr = false);
	std::string PrepareMoveInstruction(DirectX::SimpleMath::Vector3 pos);
};