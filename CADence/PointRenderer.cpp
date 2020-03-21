#include "PointRenderer.h"

void PointRenderer::Render(Object object)
{
	//Update content to fill constant buffer
	D3D11_MAPPED_SUBRESOURCE res;
	XMMATRIX mvp = m_transform.GetModelMatrix() * renderData->m_camera->GetViewProjectionMatrix();
	//Set constant buffer
	auto hres = renderData->m_device.context()->Map((renderData->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	renderData->m_device.context()->Unmap(renderData->m_cbMVP.get(), 0);
	ID3D11Buffer* cbs[] = { renderData->m_cbMVP.get() };
	renderData->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	float size = 0.2f;

	std::vector<VertexPositionColor> vertices{
		{{-size,0.0f,0.0f},{1.0f,1.0f,1.0f}},
		{{ size,0.0f,0.0f},{1.0f,1.0f,1.0f}},
		{{0.0f,-size,0.0f},{1.0f,1.0f,1.0f}},
		{{0.0f, size,0.0f},{1.0f,1.0f,1.0f}},
		{{0.0f,0.0f,-size},{1.0f,1.0f,1.0f}},
		{{0.0f,0.0f, size},{1.0f,1.0f,1.0f}}
	};

	std::vector<unsigned short> indices{
	0,1,
	2,3,
	4,5
	};

	renderData->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Update Vertex and index buffers
	renderData->m_vertexBuffer = (renderData->m_device.CreateVertexBuffer(vertices));
	renderData->m_indexBuffer = (renderData->m_device.CreateIndexBuffer(indices));
	ID3D11Buffer* vbs[] = { renderData->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };

	renderData->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	renderData->m_device.context()->IASetIndexBuffer(renderData->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	renderData->m_device.context()->DrawIndexed(6, 0, 0);
}
