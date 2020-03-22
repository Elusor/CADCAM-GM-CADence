#include "MeshRenderer.h"
#include "GlobalRenderState.h"
#include "CameraRegistry.h"
void MeshRenderer::RenderMesh(MeshObject* object)
{
	UpdateInputLayout();	
	auto m_transform = object->m_transform;

	GlobalRenderState::m_device.context()->IASetPrimitiveTopology(object->m_surVerDesc.m_primitiveTopology);
	//Update content to fill constant buffer
	D3D11_MAPPED_SUBRESOURCE res;
	DirectX::XMMATRIX mvp = m_transform.GetModelMatrix() * CameraRegistry::currentCamera->GetViewProjectionMatrix();
	//Set constant buffer
	auto hres = GlobalRenderState::m_device.context()->Map((m_renderState->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(DirectX::XMMATRIX));
	GlobalRenderState::m_device.context()->Unmap(m_renderState->m_cbMVP.get(), 0);
	ID3D11Buffer* cbs[] = { m_renderState->m_cbMVP.get() };
	GlobalRenderState::m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	// Update Vertex and index buffers
	m_renderState->m_vertexBuffer = (GlobalRenderState::m_device.CreateVertexBuffer(object->m_surVerDesc.vertices));
	m_renderState->m_indexBuffer = (GlobalRenderState::m_device.CreateIndexBuffer(object->m_surVerDesc.indices));
	ID3D11Buffer* vbs[] = { m_renderState->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };

	GlobalRenderState::m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	GlobalRenderState::m_device.context()->IASetIndexBuffer(m_renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	GlobalRenderState::m_device.context()->DrawIndexed(object->m_surVerDesc.indices.size(), 0, 0);
}
