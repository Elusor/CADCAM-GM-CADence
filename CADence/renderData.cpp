#include "renderData.h"

RenderData::RenderData(DxDevice* device, Camera* camera, ID3D11Buffer* vBuffer, ID3D11Buffer* iBuffer, ID3D11Buffer* mvpBuffer)
{
	m_device = device;
	m_camera = camera;
	m_vertexBuffer = vBuffer;
	m_indexBuffer = iBuffer;
	m_cbMVP = mvpBuffer;
}
