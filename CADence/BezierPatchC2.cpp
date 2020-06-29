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
	MeshObject::RenderMesh(renderState, m_VDesc);
	context->HSSetShader(nullptr, 0, 0);
	context->DSSetShader(nullptr, 0, 0);
	context->RSSetState(nullptr);
}

DirectX::XMFLOAT3 BezierPatchC2::GetPoint(float u, float v)
{
	auto refs = GetReferences().GetAllRef();

	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	DirectX::XMFLOAT3 p00, p01, p02, p03;
	p00 = refs[0].m_refered.lock()->m_object->GetPosition();
	p01 = refs[1].m_refered.lock()->m_object->GetPosition();
	p02 = refs[2].m_refered.lock()->m_object->GetPosition();
	p03 = refs[3].m_refered.lock()->m_object->GetPosition();
	aux0 = BezierCalculator::CalculateBezier4(p00, p01, p02, p03, u);

	DirectX::XMFLOAT3 p10, p11, p12, p13;
	p10 = refs[4].m_refered.lock()->m_object->GetPosition();
	p11 = refs[5].m_refered.lock()->m_object->GetPosition();
	p12 = refs[6].m_refered.lock()->m_object->GetPosition();
	p13 = refs[7].m_refered.lock()->m_object->GetPosition();
	aux1 = BezierCalculator::CalculateBezier4(p10, p11, p12, p13, u);

	DirectX::XMFLOAT3 p20, p21, p22, p23;
	p20 = refs[8].m_refered.lock()->m_object->GetPosition();
	p21 = refs[9].m_refered.lock()->m_object->GetPosition();
	p22 = refs[10].m_refered.lock()->m_object->GetPosition();
	p23 = refs[11].m_refered.lock()->m_object->GetPosition();
	aux2 = BezierCalculator::CalculateBezier4(p20, p21, p22, p23, u);

	DirectX::XMFLOAT3 p30, p31, p32, p33;
	p30 = refs[12].m_refered.lock()->m_object->GetPosition();
	p31 = refs[13].m_refered.lock()->m_object->GetPosition();
	p32 = refs[14].m_refered.lock()->m_object->GetPosition();
	p33 = refs[15].m_refered.lock()->m_object->GetPosition();
	aux3 = BezierCalculator::CalculateBezier4(p30, p31, p32, p33, u);

	return BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
}

DirectX::XMFLOAT3 BezierPatchC2::GetTangent(float u, float v, TangentDir tangentDir)
{
	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	DirectX::XMFLOAT3 p00, p01, p02, p03;
	p00 = refs[0].m_refered.lock()->m_object->GetPosition();
	p01 = refs[1].m_refered.lock()->m_object->GetPosition();
	p02 = refs[2].m_refered.lock()->m_object->GetPosition();
	p03 = refs[3].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 p10, p11, p12, p13;
	p10 = refs[4].m_refered.lock()->m_object->GetPosition();
	p11 = refs[5].m_refered.lock()->m_object->GetPosition();
	p12 = refs[6].m_refered.lock()->m_object->GetPosition();
	p13 = refs[7].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 p20, p21, p22, p23;
	p20 = refs[8].m_refered.lock()->m_object->GetPosition();
	p21 = refs[9].m_refered.lock()->m_object->GetPosition();
	p22 = refs[10].m_refered.lock()->m_object->GetPosition();
	p23 = refs[11].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 p30, p31, p32, p33;
	p30 = refs[12].m_refered.lock()->m_object->GetPosition();
	p31 = refs[13].m_refered.lock()->m_object->GetPosition();
	p32 = refs[14].m_refered.lock()->m_object->GetPosition();
	p33 = refs[15].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 result;

	if (tangentDir == TangentDir::AlongU)
	{
		// Along U
		auto aux00 = BezierCalculator::CalculateBezier3(p00, p01, p02, u);
		auto aux01 = BezierCalculator::CalculateBezier3(p01, p02, p03, u);
		aux0 = aux01 - aux00;

		auto aux10 = BezierCalculator::CalculateBezier3(p10, p11, p12, u);
		auto aux11 = BezierCalculator::CalculateBezier3(p11, p12, p13, u);
		aux1 = aux11 - aux10;

		auto aux20 = BezierCalculator::CalculateBezier3(p20, p21, p22, u);
		auto aux21 = BezierCalculator::CalculateBezier3(p21, p22, p23, u);
		aux2 = aux21 - aux20;

		auto aux30 = BezierCalculator::CalculateBezier3(p30, p31, p32, u);
		auto aux31 = BezierCalculator::CalculateBezier3(p31, p32, p33, u);
		aux3 = aux31 - aux30;

		result = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
	}
	else {
		// Along V
		aux0 = BezierCalculator::CalculateBezier4(p00, p01, p02, p03, u);
		aux1 = BezierCalculator::CalculateBezier4(p10, p11, p12, p13, u);
		aux2 = BezierCalculator::CalculateBezier4(p20, p21, p22, p23, u);
		aux3 = BezierCalculator::CalculateBezier4(p30, p31, p32, p33, u);

		// Calculate diff along V
		auto p0 = BezierCalculator::CalculateBezier3(aux0, aux1, aux2, v);
		auto p1 = BezierCalculator::CalculateBezier3(aux1, aux2, aux3, v);
		result = p1 - p0;
	}

	return result;
}
