#include "SceneGrid.h"
#include "imgui.h"

using namespace DirectX;

SceneGrid::SceneGrid() : SceneGrid(300)
{
}

SceneGrid::SceneGrid(int gridSize)
{
	m_color = { .4f, .4f, .5f };
	m_gridSize = gridSize;
	m_defaultName = "SceneGrid";
	GenerateVertices(gridSize);
}

void SceneGrid::RenderObject(std::unique_ptr<RenderState>& renderState)
{		
	XMMATRIX m = m_transform.GetModelMatrix();
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), m);
	ID3D11Buffer* cbs[] = { Mbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(1, 1, cbs);

	renderState->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Update Vertex and index buffers
	renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(m_vertices));
	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(m_indices));
	ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };

	renderState->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	renderState->m_device.context()->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	renderState->m_device.context()->DrawIndexed(m_indices.size(), 0, 0);
}

bool SceneGrid::CreateParamsGui()
{
	bool objectChanged = false;
	float dragSpeed = 0.01f;
	float maxVal = 1000.0f;

	// Edit Color
	float color[3] = {
		m_color.x,
		m_color.y,
		m_color.z,
	};
	std::string text = "Grid color";
	ImGui::Text(text.c_str());
	objectChanged |= ImGui::ColorEdit3(GetIdentifier().c_str(), (float*)&color);
	m_color.x = color[0];
	m_color.y = color[1];
	m_color.z = color[2];

	objectChanged |= ImGui::DragInt(GetIdentifier().c_str(), &m_gridSize);

	if (objectChanged)
	{
		GenerateVertices(m_gridSize);
	}
	return objectChanged;
}

void SceneGrid::UpdateObject()
{
	GenerateVertices(m_gridSize);
}

void SceneGrid::GenerateVertices(int gridSize)
{
	if (gridSize < 1)
		gridSize = 1;
	m_vertices.clear();
	m_indices.clear();

	int counter = 0;
	for (int i = -gridSize ; i <= gridSize; i++)
	{
		VertexPositionColor v1{ {(float) gridSize * 5.f, 0.f, (float)i * 5.f},{m_color} };
		VertexPositionColor v2{ {(float)-gridSize * 5.f, 0.f, (float)i * 5.f},{m_color} };
		VertexPositionColor v3{ {(float)i * 5.f, 0.f, (float) gridSize * 5.f},{m_color} };
		VertexPositionColor v4{ {(float)i * 5.f, 0.f, (float)-gridSize * 5.f},{m_color} };
		// calculate horizontal line
		m_vertices.push_back(v1);
		m_vertices.push_back(v2);
		m_vertices.push_back(v3);
		m_vertices.push_back(v4);		
		
		// add corresponding indices
		m_indices.push_back(counter++);
		m_indices.push_back(counter++);
		m_indices.push_back(counter++);
		m_indices.push_back(counter++);
	}
}
