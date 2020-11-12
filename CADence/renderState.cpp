#include "renderState.h"

RenderState::RenderState(mini::Window& window) : m_device(window)
{
}

RenderState::RenderState(mini::Window& window, Viewport viewport, std::shared_ptr<Camera> camera) : m_device(window)
{
	m_device.context()->RSSetViewports(1, &viewport);
	m_camera = camera;
	currentCamera = camera.get();

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
	const auto paramVsBytes = DxDevice::LoadByteCode(L"paramVS.cso");
	const auto ssVsBytes = DxDevice::LoadByteCode(L"ssVs.cso");
	const auto ssPsBytes = DxDevice::LoadByteCode(L"ssPs.cso");
	const auto texVsBytes = DxDevice::LoadByteCode(L"vsTex.cso");
	const auto texPsBytes = DxDevice::LoadByteCode(L"psTex.cso");
	const auto paramSpaceVsBytes = DxDevice::LoadByteCode(L"paramSpaceVS.cso");
	const auto paramSpacePsBytes = DxDevice::LoadByteCode(L"paramSpacePS.cso");
	const auto torusGSBytes = DxDevice::LoadByteCode(L"torusGS.cso");
	const auto bezierGsBytes = DxDevice::LoadByteCode(L"bezierGs.cso");
	const auto patchParamGSBytes = DxDevice::LoadByteCode(L"patchParamGS.cso");
	const auto patchC2ParamGSBytes = DxDevice::LoadByteCode(L"patchC2ParamGS.cso");
	const auto patchGsBytes = DxDevice::LoadByteCode(L"patchGS.cso");
	const auto patchHsBytes = DxDevice::LoadByteCode(L"patchHS.cso");
	const auto patchC2DsBytes = DxDevice::LoadByteCode(L"patchC2DS.cso");
	const auto patchDsBytes = DxDevice::LoadByteCode(L"patchDS.cso");
	const auto patchGregDsBytes = DxDevice::LoadByteCode(L"patchGregDS.cso");
	const auto patchGregHsBytes = DxDevice::LoadByteCode(L"patchGregHS.cso");

	const auto paramPatchC2FillGS = DxDevice::LoadByteCode(L"paramPatchC2FillGS.cso");
	const auto paramPatchFillGS = DxDevice::LoadByteCode(L"paramPatchFillGS.cso");

	m_paramVS = m_device.CreateVertexShader(paramVsBytes);
	m_texVS = m_device.CreateVertexShader(texVsBytes);
	m_texPS = m_device.CreatePixelShader(texPsBytes);
	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);
	m_screenSpaceVS = m_device.CreateVertexShader(ssVsBytes);
	m_screenSpacePS = m_device.CreatePixelShader(ssPsBytes);
	m_paramSpaceVS = m_device.CreateVertexShader(paramSpaceVsBytes);
	m_paramSpacePS = m_device.CreatePixelShader(paramSpacePsBytes);
	m_bezierGeometryShader = m_device.CreateGeometryShader(bezierGsBytes);
	m_patchParamGeometryShader = m_device.CreateGeometryShader(patchParamGSBytes);
	m_patchC2ParamGeometryShader = m_device.CreateGeometryShader(patchC2ParamGSBytes);
	m_patchGeometryShader = m_device.CreateGeometryShader(patchGsBytes);
	m_torusGeometryShader = m_device.CreateGeometryShader(torusGSBytes);
	m_paramPatchC2FillGS = m_device.CreateGeometryShader(paramPatchC2FillGS);
	m_paramPatchFillGS = m_device.CreateGeometryShader(paramPatchFillGS);

	m_patchC2DomainShader = m_device.CreateDomainShader(patchC2DsBytes);
	m_patchDomainShader = m_device.CreateDomainShader(patchDsBytes);
	m_patchGregDomainShader = m_device.CreateDomainShader(patchGregDsBytes);
	

	m_patchHullShader = m_device.CreateHullShader(patchHsBytes);
	m_patchGregHullShader = m_device.CreateHullShader(patchGregHsBytes);
	
	RegisterLayout<VertexPositionColor>(VertexPositionColor::GetInputLayoutElements(), vsBytes);
	RegisterLayout<VertexParameterColor>(VertexParameterColor::GetInputLayoutElements(), paramVsBytes);

	m_cbCamPos = m_device.CreateConstantBuffer<XMFLOAT4>();
	m_cbFogBuffer = m_device.CreateConstantBuffer<XMFLOAT4>();
	m_cbM = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_cbVP = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_cbGSData = m_device.CreateConstantBuffer<XMFLOAT4>();
	m_cbTorusData = m_device.CreateConstantBuffer<XMFLOAT4>();

	m_cbPatchData = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_cbPatchData1 = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_cbPatchData2 = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_cbPatchData3 = m_device.CreateConstantBuffer<XMFLOAT4X4>();

	m_cbPatchDivisions = m_device.CreateConstantBuffer<XMFLOAT4>();
}

ID3D11InputLayout* RenderState::GetLayout(std::type_index vertexDataTypeIndex)
{
	return m_layoutManager.GetLayout(vertexDataTypeIndex);
}

ShaderPreset RenderState::GetCurrentShaderPreset()
{
	auto context = m_device.context().get();

	ShaderPreset preset;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11DomainShader* domainShader;
	ID3D11HullShader* hullShader;
	ID3D11GeometryShader* geometryShader;

	UINT count = 0;
	context->VSGetShader(&vertexShader, nullptr, &count);
	context->PSGetShader(&pixelShader, nullptr, &count);
	context->HSGetShader(&hullShader, nullptr, &count);
	context->DSGetShader(&domainShader, nullptr, &count);
	context->GSGetShader(&geometryShader, nullptr, &count);

	preset.vertexShader	= vertexShader;
	preset.pixelShader = pixelShader;
	preset.domainShader	= domainShader;
	preset.hullShader = hullShader;
	preset.geometryShader = geometryShader;

	return preset;
}

void RenderState::SetShaderPreset(ShaderPreset preset)
{
	auto context = m_device.context().get();
	ID3D11VertexShader* vertexShader = preset.vertexShader;
	ID3D11PixelShader* pixelShader = preset.pixelShader;
	ID3D11DomainShader* domainShader = preset.domainShader;
	ID3D11HullShader* hullShader = preset.hullShader;
	ID3D11GeometryShader* geometryShader = preset.geometryShader;

	context->VSSetShader(vertexShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);
	context->HSSetShader(hullShader, nullptr, 0);
	context->DSSetShader(domainShader, nullptr, 0);
	context->GSSetShader(geometryShader, nullptr, 0);
}
