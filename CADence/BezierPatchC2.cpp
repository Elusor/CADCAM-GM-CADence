#include "BezierPatchC2.h"
#include "bezierCalculator.h"
#include "mathUtils.h"
BezierPatchC2::BezierPatchC2():BezierPatch()
{
}

BezierPatchC2::~BezierPatchC2()
{
}

void BezierPatchC2::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	if(m_meshDesc.indices.size() > 0)
	{
		RenderPatch(renderState);
	}
	if (m_displayPolygon)
	{
		RenderPolygon(renderState);
	}
}

void BezierPatchC2::RenderPatch(std::unique_ptr<RenderState>& renderState)
{
	auto context = renderState->m_device.context().get();

	auto desc = m_meshDesc;
	context->IASetPrimitiveTopology(desc.m_primitiveTopology);
	auto inputLayout = renderState->GetLayout(desc.GetVertexDataTypeIdx());
	renderState->m_device.context()->IASetInputLayout(inputLayout);

	auto prevPreset = renderState->GetCurrentShaderPreset();
	ShaderPreset preset;
	preset.vertexShader = renderState->m_paramVS.get();
	preset.pixelShader = renderState->m_pixelShader.get();
	preset.geometryShader = renderState->m_patchC2ParamGeometryShader.get();
	preset.hullShader = nullptr;
	preset.domainShader = nullptr;
	renderState->SetShaderPreset(preset);

	//Set constant buffer
	XMMATRIX m = m_transform.GetModelMatrix();
	XMMATRIX VP = renderState->currentCamera->GetViewProjectionMatrix();
	auto positions = GetPatchPointPositions();
	XMFLOAT4X4 row1 = GetRowAsFloat4x4(positions.row0);
	XMFLOAT4X4 row2 = GetRowAsFloat4x4(positions.row1);
	XMFLOAT4X4 row3 = GetRowAsFloat4x4(positions.row2);
	XMFLOAT4X4 row4 = GetRowAsFloat4x4(positions.row3);
	XMFLOAT4X4 rows[4] = { row1, row2, row3, row4 };

	auto CPbuffer1 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData.get(), row1);
	auto CPbuffer2 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData1.get(), row2);
	auto CPbuffer3 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData2.get(), row3);
	auto CPbuffer4 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData3.get(), row4);
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), m);
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), VP);

	ID3D11Buffer* cbs1[] = { Mbuffer }; //, VPbuffer
	ID3D11Buffer* cbs2[] = { VPbuffer }; //, VPbuffer
	ID3D11Buffer* cbsControlPoint[] = { CPbuffer1,CPbuffer2,CPbuffer3,CPbuffer4 }; //, VPbuffer

	context->GSSetConstantBuffers(0, 1, cbs1);
	context->GSSetConstantBuffers(1, 1, cbs2);
	context->GSSetConstantBuffers(2, 4, cbsControlPoint);

	// Update Vertex and index buffers
	renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(desc.vertices));
	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(desc.indices));
	ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexParameterColor) };
	UINT offsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	context->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(desc.indices.size(), 0, 0);
	

	// Draww Fill mesh
#pragma region FillMesh	

	ID3D11RasterizerState* stateOld;
	ID3D11RasterizerState* stateNew;
	context->RSGetState(&stateOld);
	int x = 2;

	D3D11_RASTERIZER_DESC rdesc;
	ZeroMemory(&rdesc, sizeof(D3D11_RASTERIZER_DESC));
	rdesc.FillMode = D3D11_FILL_SOLID;
	rdesc.CullMode = D3D11_CULL_NONE;
	rdesc.DepthClipEnable = true;

	// GS shader
	auto fillGs = renderState->m_paramPatchC2FillGS.get();
	context->GSSetShader(fillGs, nullptr, 0);
	renderState->m_device->CreateRasterizerState(&rdesc, &stateNew);

	context->RSSetState(stateNew);
	context->IASetPrimitiveTopology(desc.m_fillTopology);
	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(desc.fillIndices));
	context->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(desc.fillIndices.size(), 0, 0);
	context->RSSetState(stateOld);
#pragma endregion

	renderState->SetShaderPreset(prevPreset);

}

DirectX::XMFLOAT3 BezierPatchC2::GetPoint(float u, float v)
{
	auto refs = GetReferences().GetAllRef();

	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;
	auto points = GetPatchPointPositions();

	aux0 = BezierCalculator::CalculateDeBoor4(points.row0, u);
	aux1 = BezierCalculator::CalculateDeBoor4(points.row1, u);
	aux2 = BezierCalculator::CalculateDeBoor4(points.row2, u);	
	aux3 = BezierCalculator::CalculateDeBoor4(points.row3, u);

	return BezierCalculator::CalculateDeBoor4(aux0, aux1, aux2, aux3, v);
}

DirectX::XMFLOAT3 BezierPatchC2::GetTangent(float u, float v, TangentDir tangentDir)
{
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	auto points = GetPatchPointPositions();
	
	DirectX::XMFLOAT3 result;

	if (tangentDir == TangentDir::AlongU)
	{
		auto bern0 = BezierCalculator::ConvertDeBoorToBezier(points.row0);
		auto bern1 = BezierCalculator::ConvertDeBoorToBezier(points.row1);
		auto bern2 = BezierCalculator::ConvertDeBoorToBezier(points.row2);
		auto bern3 = BezierCalculator::ConvertDeBoorToBezier(points.row3);
		
		// Along U
		auto der0 = BezierCalculator::GetDerivativeCoefficients(bern0);
		auto der1 = BezierCalculator::GetDerivativeCoefficients(bern1);
		auto der2 = BezierCalculator::GetDerivativeCoefficients(bern2);
		auto der3 = BezierCalculator::GetDerivativeCoefficients(bern3);

		auto derVal0 = BezierCalculator::CalculateBezier3(der0, u);
		auto derVal1 = BezierCalculator::CalculateBezier3(der1, u);
		auto derVal2 = BezierCalculator::CalculateBezier3(der2, u);
		auto derVal3 = BezierCalculator::CalculateBezier3(der3, u);
		
		result = BezierCalculator::CalculateDeBoor4(derVal0, derVal1, derVal2, derVal3, v);
	}
	else {
		// Along V
		aux0 = BezierCalculator::CalculateDeBoor4(points.row0, u);
		aux1 = BezierCalculator::CalculateDeBoor4(points.row1, u);
		aux2 = BezierCalculator::CalculateDeBoor4(points.row2, u);
		aux3 = BezierCalculator::CalculateDeBoor4(points.row3, u);

		// Convert Auxs to Beziers
		auto auxBerns = BezierCalculator::ConvertDeBoorToBezier(aux0, aux1, aux2, aux3);

		// Calculate diff along V
		auto derivatives = BezierCalculator::GetDerivativeCoefficients(auxBerns);

		result = BezierCalculator::CalculateBezier3(derivatives,v);
	}

	return result;
}

DirectX::XMFLOAT3 BezierPatchC2::GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir)
{
	auto points = GetPatchPointPositions();

	DirectX::XMFLOAT3 result;

	if (tangentDir == TangentDir::AlongU)
	{
		auto bern0 = BezierCalculator::ConvertDeBoorToBezier(points.row0);
		auto bern1 = BezierCalculator::ConvertDeBoorToBezier(points.row1);
		auto bern2 = BezierCalculator::ConvertDeBoorToBezier(points.row2);
		auto bern3 = BezierCalculator::ConvertDeBoorToBezier(points.row3);

		auto dder0 = BezierCalculator::GetSecondDerivativeCoefficients(bern0);
		auto dder1 = BezierCalculator::GetSecondDerivativeCoefficients(bern1);
		auto dder2 = BezierCalculator::GetSecondDerivativeCoefficients(bern2);
		auto dder3 = BezierCalculator::GetSecondDerivativeCoefficients(bern3);

		auto curvePoint0 = BezierCalculator::CalculateBezier2(dder0.b210, dder0.b321, u);
		auto curvePoint1 = BezierCalculator::CalculateBezier2(dder1.b210, dder1.b321, u);
		auto curvePoint2 = BezierCalculator::CalculateBezier2(dder2.b210, dder2.b321, u);
		auto curvePoint3 = BezierCalculator::CalculateBezier2(dder3.b210, dder3.b321, u);

		result = BezierCalculator::CalculateDeBoor4(curvePoint0, curvePoint1, curvePoint2, curvePoint3, v);
	}
	else {
		auto deBoor0 = BezierCalculator::CalculateDeBoor4(points.row0, u);
		auto deBoor1 = BezierCalculator::CalculateDeBoor4(points.row1, u);
		auto deBoor2 = BezierCalculator::CalculateDeBoor4(points.row2, u);
		auto deBoor3 = BezierCalculator::CalculateDeBoor4(points.row3, u);

		auto vCurve = BezierCalculator::ConvertDeBoorToBezier(deBoor0, deBoor1, deBoor2, deBoor3);			
		auto vDer = BezierCalculator::GetSecondDerivativeCoefficients(vCurve);

		result = BezierCalculator::CalculateBezier2(vDer.b210, vDer.b321, v);
	}

	return result;
}

DirectX::XMFLOAT3 BezierPatchC2::GetSecondDarivativeMixed(float u, float v)
{
	auto points = GetPatchPointPositions();

	DirectX::XMFLOAT3 result;

	auto bern0 = BezierCalculator::ConvertDeBoorToBezier(points.row0);
	auto bern1 = BezierCalculator::ConvertDeBoorToBezier(points.row1);
	auto bern2 = BezierCalculator::ConvertDeBoorToBezier(points.row2);
	auto bern3 = BezierCalculator::ConvertDeBoorToBezier(points.row3);

	auto bernDu0 = BezierCalculator::GetDerivativeCoefficients(bern0);
	auto bernDu1 = BezierCalculator::GetDerivativeCoefficients(bern1);
	auto bernDu2 = BezierCalculator::GetDerivativeCoefficients(bern2);
	auto bernDu3 = BezierCalculator::GetDerivativeCoefficients(bern3);

	auto der0 = BezierCalculator::CalculateBezier3(bernDu0, u);
	auto der1 = BezierCalculator::CalculateBezier3(bernDu1, u);
	auto der2 = BezierCalculator::CalculateBezier3(bernDu2, u);
	auto der3 = BezierCalculator::CalculateBezier3(bernDu3, u);

	auto bernsteinVCurve = BezierCalculator::ConvertDeBoorToBezier(der0, der1, der2, der3);
	auto finalPoints = BezierCalculator::GetDerivativeCoefficients(bernsteinVCurve);
	result = BezierCalculator::CalculateBezier3(finalPoints, v);
	return result;
}
