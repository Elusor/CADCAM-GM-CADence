#include "PointSelector.h"
#include <direct.h>

using namespace DirectX;

PointSelector::PointSelector(std::shared_ptr<Camera> camera)
{
	m_camera = camera;
}

std::weak_ptr<Node> PointSelector::GetNearestPoint(int mouseX, int mouseY, std::vector<std::shared_ptr<Node>> nodes, int width , int height, float radius)
{	
	XMMATRIX VP = m_camera->GetViewProjectionMatrix();
	
	float minDist = FLT_MAX;
	int minIdx = -1;

	float radSqr = radius * radius;

	for (int i = 0; i < nodes.size(); i++)
	{
		// tylko jesli mamy punkt 
		std::string name = nodes[i]->m_object->m_name;
		XMFLOAT4 screenPosFl;
		// Get point pos
		XMVECTOR pointPos = XMLoadFloat3(&(nodes[i]->m_object->GetPosition()));
		// Calc screen pos
		XMVECTOR screenPos = XMVector3Transform(pointPos, VP);

		XMStoreFloat4(&screenPosFl, screenPos);
		float x = screenPosFl.x;
		float y = screenPosFl.y;
		float z = screenPosFl.z;
		float w = screenPosFl.w;
	
		float xFrust = x / w;
		float yFrust = y / w;
		
		float wf = (float)width;
		float hf = (float)height;

		float wfHalf = wf / 2.0f;
		float hfHalf = hf / 2.0f;

		float screenW = wfHalf * (xFrust + 1.0f);
		float screenH = hfHalf * (1.0f - yFrust);
		
		float deltaX = screenW - mouseX;
		float deltaY = screenH - mouseY;

		float distSqr = deltaX * deltaX + deltaY * deltaY;

		if (minDist > distSqr && distSqr < radSqr)
		{
			minDist = distSqr;
			minIdx = i;
		}
	}		

	std::weak_ptr<Node> selectedNode;

	if (minIdx != -1)
	{
		selectedNode = nodes[minIdx];
	}
	else
	{
		selectedNode.reset();
	}

	return std::weak_ptr<Node>(selectedNode);
}

std::vector<std::weak_ptr<Node>> PointSelector::GetAllPointsInArea(std::vector<std::shared_ptr<Node>> nodes, int width, int height)
{
	XMMATRIX VP = m_camera->GetViewProjectionMatrix();
	std::vector<std::weak_ptr<Node>> points;

	float minY = min(m_p0.y, m_p1.y);
	float maxY = max(m_p0.y, m_p1.y);

	float minX = min(m_p0.x, m_p1.x);
	float maxX = max(m_p0.x, m_p1.x);

	for (int i = 0; i < nodes.size(); i++)
	{
		// tylko jesli mamy punkt 
		std::string name = nodes[i]->m_object->m_name;
		XMFLOAT4 screenPosFl;
		// Get point pos
		XMVECTOR pointPos = XMLoadFloat3(&(nodes[i]->m_object->GetPosition()));
		// Calc screen pos
		XMVECTOR screenPos = XMVector3Transform(pointPos, VP);

		XMStoreFloat4(&screenPosFl, screenPos);
		float x = screenPosFl.x;
		float y = screenPosFl.y;
		float z = screenPosFl.z;
		float w = screenPosFl.w;

		float xFrust = x / w;
		float yFrust = y / w;

		float wf = (float)width;
		float hf = (float)height;

		float wfHalf = wf / 2.0f;
		float hfHalf = hf / 2.0f;

		float screenW = wfHalf * (xFrust + 1.0f);
		float screenH = hfHalf * (1.0f - yFrust);

		if (screenW > minX&& screenW < maxX && screenH > minY&& screenH < maxY)
		{
			points.push_back(std::weak_ptr<Node>(nodes[i]));
		}
	}

	return points;
}

void PointSelector::ProcessInput(Scene* scene, SIZE windowSize)
{

	bool lDown = ImGui::GetIO().MouseDown[0];
	bool lUp = ImGui::GetIO().MouseReleased[0];

	if (lDown && !ImGui::GetIO().WantCaptureMouse)
	{
		auto pos = ImGui::GetIO().MousePos;
		StartCaptureMultiselect(pos.x, pos.y);


		/*auto selectedNode = GetNearestPoint(pos.x, pos.y, m_scene->m_nodes, m_window.getClientSize().cx, m_window.getClientSize().cy, 50);


		if (auto node = selectedNode.lock())
		{
			for (int i = 0; i < m_scene->m_selectedNodes.size(); i++)
			{
				if (auto nod = m_scene->m_selectedNodes[i].lock())
				{
					nod->m_isSelected = false;
				}
			}
			m_scene->m_selectedNodes.clear();

			node->m_isSelected = true;
			m_scene->m_selectedNodes.push_back(selectedNode);
		}*/
	}

	if (lUp && !ImGui::GetIO().WantCaptureMouse)
	{
		auto pos = ImGui::GetIO().MousePos;
		if (IsCapturing())
		{
			EndCaptureMultiselect(pos.x, pos.y);
			auto pts = GetAllPointsInArea(scene->m_nodes, windowSize.cx, windowSize.cy);

			for (int i = 0; i < scene->m_selectedNodes.size(); i++)
			{
				if (auto nod = scene->m_selectedNodes[i].lock())
				{
					nod->m_isSelected = false;
				}
			}
			scene->m_selectedNodes.clear();

			for (int i = 0; i < pts.size(); i++)
			{
				if (auto nod = pts[i].lock())
				{
					nod->m_isSelected = true;
					scene->m_selectedNodes.push_back(nod);
				}
			}

		}
	}

}

void PointSelector::StartCaptureMultiselect(int mouseX, int mouseY)
{
	if (m_isCapturing == false)
	{
		m_isCapturing = true;
		m_p0.x = mouseX;
		m_p0.y = mouseY;
	}
}

void PointSelector::EndCaptureMultiselect(int mouseX, int mouseY)
{
	m_isCapturing = false;
	m_p1.x = mouseX;
	m_p1.y = mouseY;
}

bool PointSelector::IsCapturing()
{
	return m_isCapturing;
}
