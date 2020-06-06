#include "renderState.h"

RenderState::RenderState(mini::Window& window) : m_device(window)
{
}

RenderState::RenderState(mini::Window& window, Viewport viewport, std::shared_ptr<Camera> camera) : m_device(window)
{
	m_device.context()->RSSetViewports(1, &viewport);
	m_camera = camera;

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
	const auto ssVsBytes = DxDevice::LoadByteCode(L"ssVs.cso");
	const auto ssPsBytes = DxDevice::LoadByteCode(L"ssPs.cso");
	const auto bezierGsBytes = DxDevice::LoadByteCode(L"bezierGs.cso");
	const auto patchGsBytes = DxDevice::LoadByteCode(L"patchGS.cso");
	const auto patchHsBytes = DxDevice::LoadByteCode(L"patchHS.cso");
	const auto patchDsBytes = DxDevice::LoadByteCode(L"patchDS.cso");

	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);
	m_screenSpaceVS = m_device.CreateVertexShader(ssVsBytes);
	m_screenSpacePS = m_device.CreatePixelShader(ssPsBytes);
	m_bezierGeometryShader = m_device.CreateGeometryShader(bezierGsBytes);
	m_patchGeometryShader = m_device.CreateGeometryShader(patchGsBytes);

	m_patchDomainShader = m_device.CreateDomainShader(patchDsBytes);
	m_patchHullShader = m_device.CreateHullShader(patchHsBytes);

	auto elements = VertexPositionColor::GetInputLayoutElements();
	m_layout = m_device.CreateInputLayout(elements, vsBytes);

	m_cbCamPos = m_device.CreateConstantBuffer<XMFLOAT4>();
	m_cbM = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_cbVP = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_cbGSData = m_device.CreateConstantBuffer<XMFLOAT4>();
	m_cbPatchData = m_device.CreateConstantBuffer<XMMATRIX>();
	m_cbPatchData1 = m_device.CreateConstantBuffer<XMMATRIX>();
	m_cbPatchData2 = m_device.CreateConstantBuffer<XMMATRIX>();
	m_cbPatchDivisions = m_device.CreateConstantBuffer<XMFLOAT4>();
}
