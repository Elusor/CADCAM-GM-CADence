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

	RenderPatch(renderState);
	if (m_displayPolygon)
	{
		RenderPolygon(renderState);
	}

}

void BezierPatchC2::RenderPatch(std::unique_ptr<RenderState>& renderState)
{
	auto context = renderState->m_device.context().get();
	XMMATRIX x = GetCoordinates(Coord::Xpos);
	XMMATRIX y = GetCoordinates(Coord::Ypos);
	XMMATRIX z = GetCoordinates(Coord::Zpos);

	XMMATRIX mat = m_transform.GetModelMatrix();
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), mat);
	ID3D11Buffer* cbs1[] = { Mbuffer }; //, VPbuffer
	renderState->m_device.context()->VSSetConstantBuffers(1, 1, cbs1);

	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = D3D11_FILL_WIREFRAME;
	desc.CullMode = D3D11_CULL_NONE;
	desc.AntialiasedLineEnable = 0;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0;
	desc.DepthClipEnable = true;
	desc.FrontCounterClockwise = 0;
	desc.MultisampleEnable = 0;
	desc.ScissorEnable = 0;
	desc.SlopeScaledDepthBias = 0;

	ID3D11RasterizerState* rs;

	renderState->m_device.m_device->CreateRasterizerState(&desc, &rs);
	context->RSSetState(rs);

	context->HSSetShader(renderState->m_patchHullShader.get(), 0, 0);
	ID3D11Buffer* hsCb[] = { renderState->m_cbPatchDivisions.get() };
	context->HSSetConstantBuffers(0, 1, hsCb);

	context->DSSetShader(renderState->m_patchC2DomainShader.get(), 0, 0);
	context->DSSetConstantBuffers(1, 1, cbs1);
	ID3D11Buffer* cbs2[] = { renderState->m_cbVP.get() };
	context->DSSetConstantBuffers(0, 1, cbs2);

	XMFLOAT4 divs = XMFLOAT4(m_uSize, 0.0f, 0.f, 0.f);
	auto divBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbPatchDivisions.get(), divs);
	ID3D11Buffer* divCBuffer[] = { divBuff }; //, VPbuffer
	renderState->m_device.context()->HSSetConstantBuffers(0, 1, divCBuffer);
	MeshObject::RenderMesh(renderState, m_UDesc);

	divs = XMFLOAT4(m_vSize, 0.0f, 0.f, 0.f);
	divBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbPatchDivisions.get(), divs);
	ID3D11Buffer* divCBuffer2[] = { divBuff }; //, VPbuffer
	renderState->m_device.context()->HSSetConstantBuffers(0, 1, divCBuffer2);
	//MeshObject::RenderMesh(renderState, m_VDesc);
	context->HSSetShader(nullptr, 0, 0);
	context->DSSetShader(nullptr, 0, 0);
	context->RSSetState(nullptr);
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
