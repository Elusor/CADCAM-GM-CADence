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

	explicit TransformationController(std::shared_ptr<Scene> scene);

	bool IsTransforming()
	{
		return
			m_type != TransformationType::NoTT
			|| m_coordinate != AffectedCoordinate::NoAC;
	};

	void DrawGUI();
	void ProcessInput(ImGuiIO& imguiIO);

	bool m_transAroundCursor = false;

private:

	
	ImVec2 prevPos;
	bool m_isCapturingMouse;
	std::shared_ptr<Scene> m_scene;
	std::unique_ptr<ObjectTransformer> m_transformer;	
	TransformationType m_type;
	AffectedCoordinate m_coordinate;

	void SetOperationType(ImGuiIO& imguiIO);
	void ProcessMouse(ImGuiIO& imguiIO);



	

};

