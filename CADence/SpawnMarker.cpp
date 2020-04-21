#include "SpawnMarker.h"
#include "vertexStructures.h"
#include "imgui.h"
using namespace DirectX;

void SpawnMarker::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	//Set constant buffer
	XMMATRIX mvp = m_transform.GetModelMatrix() * renderState->m_camera->GetViewProjectionMatrix();
	auto buffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbMVP.get(), mvp);
	ID3D11Buffer* cbs[] = {buffer};

	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	std::vector<VertexPositionColor> vertices{
		{{-3.0f,0.0f,0.0f},{m_color}},
		{{3.0f,0.0f,0.0f} ,{m_color}},
		{{0.0f,-3.0f,0.0f},{m_color}},
		{{0.0f,3.0f,0.0f}, {m_color}},
		{{0.0f,0.0f,-3.0f},{m_color}},
		{{0.0f,0.0f,3.0f}, {m_color}}
	};

	std::vector<unsigned short> indices{
	0,1,
	2,3,
	4,5
	};

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

bool SpawnMarker::CreateParamsGui()
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
	std::string text = "Cursor color";
	ImGui::Text(text.c_str());
	objectChanged = ImGui::ColorEdit3(GetIdentifier().c_str(), (float*)&color);
	m_color.x = color[0];
	m_color.y = color[1];
	m_color.z = color[2];

	ImGui::Spacing();
	
	// Edit position
	ImGui::Text("World position");
	DirectX::XMFLOAT3 pos = m_transform.GetPosition();
	float posf[3] = { pos.x,pos.y,pos.z };
	objectChanged |= ImGui::DragFloat3(GetIdentifier().c_str(), (posf), dragSpeed, -maxVal, maxVal);	
	m_transform.SetPosition(posf[0],posf[1],posf[2]);

	return objectChanged;
}