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
		XMVECTOR pointPos = XMLoadFloat3(&(nodes[i]->m_object->m_transform.GetPosition()));
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
