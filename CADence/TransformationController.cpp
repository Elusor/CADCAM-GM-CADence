#include "TransformationController.h"
#include "Node.h"
#include <direct.h>

TransformationController::TransformationController(std::shared_ptr<Scene> scene)
{
	prevPos = ImVec2(0.0f, 0.0f);
	m_isCapturingMouse = false;
	m_scene = scene;
	m_transformer = std::unique_ptr<ObjectTransformer>(new ObjectTransformer());
	m_type = TransformationType::NoTT;
	m_coordinate = AffectedCoordinate::NoAC;
}

void TransformationController::ProcessInput(ImGuiIO& imguiIO)
{
	bool isUsedByImgui = imguiIO.WantCaptureKeyboard || imguiIO.WantCaptureMouse;

	if (!isUsedByImgui)
	{
		SetOperationType(imguiIO);
		ProcessMouse(imguiIO);
	}
}

void TransformationController::CreateGuiStatus()
{
	if (m_type != TransformationType::NoTT)
	{
		ImGui::Text("Press Q to exit transformation mode");
		ImGui::Spacing();
	}

	ImGui::Text("Transformation:");
	ImGui::SameLine();
	switch (m_type)
	{
	case TransformationType::Translation:
		ImGui::Text("Translation");
		break;
	case TransformationType::Rotation:
		ImGui::Text("Rotation");
		break;
	case TransformationType::Scaling:
		ImGui::Text("Scaling");
		break;
	case TransformationType::NoTT:
		ImGui::Text("None");
		break;
	}

	ImGui::Text("Coordinate:");
	ImGui::SameLine();
	switch (m_coordinate)
	{
	case AffectedCoordinate::X:
		ImGui::Text("X");
		break;
	case AffectedCoordinate::Y:
		ImGui::Text("Y");
		break;
	case AffectedCoordinate::Z:
		ImGui::Text("Z");
		break;
	case AffectedCoordinate::NoAC:
		ImGui::Text("None");
		break;
	}
}

void TransformationController::Reset()
{
	m_type = TransformationType::NoTT;
	m_coordinate = AffectedCoordinate::NoAC;
}

void TransformationController::SetOperationType(ImGuiIO& imguiIO)
{
	if (m_type != TransformationType::NoTT)
	{
		// Set coordinate
		if (ImGui::IsKeyDown('X'))
		{
			m_coordinate = AffectedCoordinate::X;
		}
		if (ImGui::IsKeyDown('Y'))
		{
			m_coordinate = AffectedCoordinate::Y;
		}
		if (ImGui::IsKeyDown('Z'))
		{
			m_coordinate = AffectedCoordinate::Z;
		}
	}	

	// Set operation
	if (ImGui::IsKeyDown('R'))
	{
		m_type = TransformationType::Rotation;
	}
	if (ImGui::IsKeyDown('T'))
	{
		m_type = TransformationType::Translation;
	}
	if (ImGui::IsKeyDown('S'))
	{
		m_type = TransformationType::Scaling;
	}

	if (ImGui::IsKeyDown('Q'))
	{
		if (m_coordinate == AffectedCoordinate::NoAC)
		{
			m_type = TransformationType::NoTT;
		}
		else
		{
			m_coordinate = AffectedCoordinate::NoAC;
		}
	}
}

void TransformationController::ProcessMouse(ImGuiIO& imguiIO)
{
	DirectX::XMFLOAT3 vect;
	DirectX::XMFLOAT3 scaleVect;
	float deltaX = 0;
	float deltaY = 0;

	bool lDown = imguiIO.MouseDown[0];
	bool lUp = imguiIO.MouseReleased[0];

	if (lDown)
	{
		m_isCapturingMouse = true;
	}

	if (lUp)
	{
		m_isCapturingMouse = false;
	}

	switch (m_coordinate)
	{
		case AffectedCoordinate::X:
			vect = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
			break;
		case AffectedCoordinate::Y:
			vect = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
			break;
		case AffectedCoordinate::Z:
			vect = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
			break;
		default:
			vect = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	if (m_isCapturingMouse)
	{
		auto pos = imguiIO.MousePos;
		deltaY = -(pos.y - prevPos.y);

		float sensitivity = 0.5f;
		float scaleSensitivity = 0.01f;
		float rotSensitivity = 0.01f;
		

		DirectX::XMFLOAT3 pivotPos;

		if (m_transAroundCursor)
		{
			pivotPos = m_scene->m_spawnMarker->GetPosition();
		}
		else
		{
			pivotPos = m_scene->m_middleMarker->GetPosition();
		}

		

		for (int i = 0; i < m_scene->m_selectedNodes.size(); i++)
		{			
			DirectX::XMFLOAT3 vec2 = DirectX::XMFLOAT3(vect.x, vect.y, vect.z);
			if (auto node = m_scene->m_selectedNodes[i].lock())
			{
				switch (m_type)
				{
				case TransformationType::Translation:
					vec2.x *= deltaY * sensitivity;
					vec2.y *= deltaY * sensitivity;
					vec2.z *= deltaY * sensitivity;
					m_transformer->TranslateObject(node->m_object, pivotPos, vec2);
					break;
				case TransformationType::Rotation:
					vec2.x *= deltaY * rotSensitivity;
					vec2.y *= deltaY * rotSensitivity;
					vec2.z *= deltaY * rotSensitivity;
					m_transformer->RotateObject(node->m_object, pivotPos, vec2);
					break;
				case TransformationType::Scaling:	
					if (deltaY != 0)
					{
						DirectX::XMFLOAT3 scaleVector = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);					

						if (deltaY > 0)
						{
							switch (m_coordinate)
							{
							case AffectedCoordinate::X:
								scaleVector.x += deltaY * scaleSensitivity;
								break;
							case AffectedCoordinate::Y:
								scaleVector.y += deltaY * scaleSensitivity;
								break;
							case AffectedCoordinate::Z:
								scaleVector.z += deltaY * scaleSensitivity;
								break;
							}				
						}
						else
						{
							switch (m_coordinate)
							{
							case AffectedCoordinate::X:
								scaleVector.x -= -deltaY * scaleSensitivity;
								break;
							case AffectedCoordinate::Y:
								scaleVector.y -= -deltaY * scaleSensitivity;
								break;
							case AffectedCoordinate::Z:
								scaleVector.z -= -deltaY * scaleSensitivity;
								break;
							}				
						}
											
						m_transformer->ScaleObject(node->m_object, pivotPos, scaleVector);
					}
					
					break;
				}
			}
		}
	}
	
	prevPos = imguiIO.MousePos;

}
