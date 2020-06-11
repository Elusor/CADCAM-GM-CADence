#pragma once
#include "Scene.h"
#include "ObjectTransformer.h"
#include "imgui.h"
enum TransformationType
{
	Rotation,
	Scaling,
	Translation,
	NoTT
};

enum AffectedCoordinate
{
	X,
	Y,
	Z,
	NoAC
};

class TransformationController
{
public:
	bool m_transAroundCursor = false;

	bool IsTransforming()
	{
		return
			m_type != TransformationType::NoTT
			|| m_coordinate != AffectedCoordinate::NoAC;
	};

	explicit TransformationController(std::shared_ptr<Scene> scene);
	void ProcessInput(ImGuiIO& imguiIO);
	void CreateGuiStatus();
	void Reset();
private:	

	float m_sensitivity = 0.5f;
	float m_scaleSensitivity = 0.01f;
	float m_rotSensitivity = 0.01f;
	ImVec2 prevPos;
	bool m_isCapturingMouse;
	std::shared_ptr<Scene> m_scene;
	std::unique_ptr<ObjectTransformer> m_transformer;	
	TransformationType m_type;
	AffectedCoordinate m_coordinate;

	void SetOperationType(ImGuiIO& imguiIO);
	void ProcessMouse(ImGuiIO& imguiIO);
	void Transform(DirectX::XMFLOAT3 transformationVector, DirectX::XMFLOAT3 pivotPosition, std::shared_ptr<Node> object, float dx, float dy);
};