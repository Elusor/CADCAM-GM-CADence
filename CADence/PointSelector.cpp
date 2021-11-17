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
		auto pointPosTmp = nodes[i]->m_object->GetPosition();
		XMVECTOR pointPos = XMLoadFloat3(&pointPosTmp);
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

		if (minDist > distSqr && distSqr < radSqr && nodes[i]->GetIsInactive() == false)
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
		auto pointPosTmp = nodes[i]->m_object->GetPosition();
		XMVECTOR pointPos = XMLoadFloat3(&(pointPosTmp));
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

		if (screenW > minX&& screenW < maxX && screenH > minY&& screenH < maxY && nodes[i]->GetIsInactive() == false)
		{
			points.push_back(std::weak_ptr<Node>(nodes[i]));
		}
	}

	return points;
}

void PointSelector::ProcessInput(std::shared_ptr<Scene>& scene, SIZE windowSize)
{
	bool lDown = ImGui::GetIO().MouseDown[0];
	bool lUp = ImGui::GetIO().MouseReleased[0];

	if (lDown && !ImGui::GetIO().WantCaptureMouse)
	{
		auto pos = ImGui::GetIO().MousePos;
		StartCaptureMultiselect(pos.x, pos.y);		
	}

	if (lUp && !ImGui::GetIO().WantCaptureMouse)
	{
		auto pos = ImGui::GetIO().MousePos;
		if (IsCapturing())
		{
			EndCaptureMultiselect(pos.x, pos.y);
			// Find all points in the selected area
			auto pts = GetAllPointsInArea(scene->m_nodes, windowSize.cx, windowSize.cy);
			if (pts.size() != 0)
			{
				// Mark the points on the scene as selected
				scene->ClearSelection();
				scene->Select(pts);				
			}
			else {
				// If no points are in the selected area - get nearest point
				// TODO : execute only when deltaX and deltaY are smaller than certain eps
				auto selectedNode = GetNearestPoint(pos.x, pos.y, scene->m_nodes, windowSize.cx, windowSize.cy, 50);
				scene->Select(selectedNode);
			}
		}
	}
}

void PointSelector::DrawSelectionWindow(std::unique_ptr<RenderState>& renderState, SIZE windowSize)
{
	if (m_isCapturing)
	{
		auto pos = ImGui::GetIO().MousePos;
		StartCaptureMultiselect(pos.x, pos.y);

		renderState->m_device.context()->VSSetShader(renderState->m_screenSpaceVS.get(), 0, 0);
		renderState->m_device.context()->PSSetShader(renderState->m_screenSpacePS.get(), 0, 0);

		XMFLOAT3 m_color = XMFLOAT3(0.8f, 0.3f, 0.1f);

		float wf = (float)windowSize.cx;
		float hf = (float)windowSize.cy;

		// calculcate pos 

		float x1 = m_p0.x / wf - 0.5f;
		float y1 = m_p0.y / hf - 0.5f;

		float x2 = pos.x / wf - 0.5f;
		float y2 = pos.y / hf - 0.5f;

		x1 *= 2.f;
		y1 *= -2.f;
		x2 *= 2.f;
		y2 *= -2.f;

		std::vector<VertexPositionColor> vertices{
			{{x1,y2, 0.0f}, {m_color}},
			{{x1,y1, 0.0f}, {m_color}},
			{{x2,y1, 0.0f}, {m_color}},
			{{x2,y2, 0.0f}, {m_color}}
		};

		std::vector<unsigned short> indices{ 0,1,1,2,2,3,3,0 };

		renderState->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		// Update Vertex and index buffers
		renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(vertices));
		renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(indices));
		ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };

		//Update strides and offets based on the vertex class
		UINT strides[] = { sizeof(VertexPositionColor) };
		UINT offsets[] = { 0 };

		renderState->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

		// Watch out for meshes that cannot be covered by ushort
		renderState->m_device.context()->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
		renderState->m_device.context()->DrawIndexed(indices.size(), 0, 0);
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
