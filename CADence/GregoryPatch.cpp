#include "GregoryPatch.h"
#include "imgui.h"
#include "Node.h"
#include "exceptions.h"
#include "ArgumentExceptions.h"
#include "GeometryUtils.h"
#include "mathUtils.h"
GregoryPatch::GregoryPatch()
{
}

GregoryPatch::~GregoryPatch()
{
}

void GregoryPatch::Initialize(
	std::vector<std::weak_ptr<Node>> edge1,
	std::vector<std::weak_ptr<Node>> edgePrev1,
	std::vector<std::weak_ptr<Node>> edge2,
	std::vector<std::weak_ptr<Node>> edgePrev2,
	std::vector<std::weak_ptr<Node>> edge3,
	std::vector<std::weak_ptr<Node>> edgePrev3)
{

	// This also needs the next to last points
	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(edge1[i]);		
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(edgePrev1[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(edge2[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(edgePrev2[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(edge3[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(edgePrev3[i]);
	}
	SetModified(true);
}

void GregoryPatch::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	//RenderPatch(renderState);
}

bool GregoryPatch::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	bool patchChanged = false;	
	ImGui::Spacing();
	
	std::string dimDrag = "Grid density" + GetIdentifier();
	patchChanged |= ImGui::DragInt(dimDrag.c_str(), &m_uSize, 1.0f, 2, 64);

	if (m_uSize < 2)
		m_uSize = 2;
	if (m_uSize > 64)
		m_uSize = 64;

	ImGui::End();
	return patchChanged;
}

void GregoryPatch::UpdateObject()
{
	CalculateGergoryPositions();

	m_UDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
	// Push lines in one direction
	for (int i = 0; i < m_patch1Positions.size(); i++)
	{
		m_UDesc.vertices.push_back(VertexPositionColor{
			{m_patch1Positions[i]},
			{m_UDesc.m_defaultColor} });
		m_UDesc.indices.push_back(i);
	}

	for (int i = 0; i < m_patch2Positions.size(); i++)
	{
		m_UDesc.vertices.push_back(VertexPositionColor{
			{m_patch2Positions[i]},
			{m_UDesc.m_defaultColor} });
		m_UDesc.indices.push_back(i + 20);
	}

	for (int i = 0; i < m_patch3Positions.size(); i++)
	{
		m_UDesc.vertices.push_back(VertexPositionColor{
			{m_patch3Positions[i]},
			{m_UDesc.m_defaultColor} });
		m_UDesc.indices.push_back(i + 40);
	}

	// TODO fix to represent "rotated patches"
	m_VDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
	// Push lines in one direction
	for (int i = 0; i < m_patch1Positions.size(); i++)
	{
		m_VDesc.vertices.push_back(VertexPositionColor{
			{m_patch1Positions[i]},
			{m_VDesc.m_defaultColor} });
		m_VDesc.indices.push_back(i);
	}

	for (int i = 0; i < m_patch2Positions.size(); i++)
	{
		m_VDesc.vertices.push_back(VertexPositionColor{
			{m_patch2Positions[i]},
			{m_VDesc.m_defaultColor} });
		m_VDesc.indices.push_back(i + 20);
	}

	for (int i = 0; i < m_patch3Positions.size(); i++)
	{
		m_VDesc.vertices.push_back(VertexPositionColor{
			{m_patch3Positions[i]},
			{m_VDesc.m_defaultColor} });
		m_VDesc.indices.push_back(i + 40);
	}

}

bool GregoryPatch::GetIsModified()
{
	bool modified = false;
	auto refs = GetReferences().GetAllRef();
	for (int i = 0; i < refs.size(); i++)
	{
		modified |= refs[i].m_refered.lock()->m_object->GetIsModified();
	}

	return modified;
}

void GregoryPatch::RenderPatch(std::unique_ptr<RenderState>& renderState)
{
	auto context = renderState->m_device.context().get();

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

	// TODO : change shaders
	context->HSSetShader(renderState->m_patchGregHullShader.get(), 0, 0);
	ID3D11Buffer* hsCb[] = { renderState->m_cbPatchDivisions.get() };
	context->HSSetConstantBuffers(0, 1, hsCb);

	// TODO : change shaders
	context->DSSetShader(renderState->m_patchGregDomainShader.get(), 0, 0);
	context->DSSetConstantBuffers(1, 1, cbs1);
	ID3D11Buffer* cbs2[] = { renderState->m_cbVP.get() };
	context->DSSetConstantBuffers(0, 1, cbs2);


	// Draw lines in one direction
	XMFLOAT4 divs = XMFLOAT4(m_uSize, 0.0f, 0.f, 0.f);
	auto divBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbPatchDivisions.get(), divs);
	ID3D11Buffer* divCBuffer[] = { divBuff }; //, VPbuffer
	renderState->m_device.context()->HSSetConstantBuffers(0, 1, divCBuffer);
	MeshObject::RenderMesh(renderState, m_UDesc);

	// Draw lines in the second direction
	divs = XMFLOAT4(m_vSize, 0.0f, 0.f, 0.f);
	divBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbPatchDivisions.get(), divs);
	ID3D11Buffer* divCBuffer2[] = { divBuff }; //, VPbuffer
	renderState->m_device.context()->HSSetConstantBuffers(0, 1, divCBuffer2);
	MeshObject::RenderMesh(renderState, m_VDesc);


	context->HSSetShader(nullptr, 0, 0);
	context->DSSetShader(nullptr, 0, 0);
	context->RSSetState(nullptr);
}

void GregoryPatch::CalculateGergoryPositions()
{
	std::vector<std::weak_ptr<Node>> edge1, edge2, edge3;
	std::vector<std::weak_ptr<Node>> edgePrev1, edgePrev2, edgePrev3;
	// fill these with object fron the ObjectReferences class
	FillReferences(edge1, edgePrev1, edge2, edgePrev2, edge3, edgePrev3);
	auto corner12 = GetPatchDefiningPointsAtCorner(edge1, edge2, edgePrev1, edgePrev2);
	auto corner23 = GetPatchDefiningPointsAtCorner(edge3, edge2, edgePrev3, edgePrev2);
	auto corner13 = GetPatchDefiningPointsAtCorner(edge1, edge3, edgePrev1, edgePrev3);

	std::vector<DirectX::XMFLOAT3> innerLeft12, innerRight12;
	std::vector<DirectX::XMFLOAT3> innerLeft23, innerRight23;
	std::vector<DirectX::XMFLOAT3> innerLeft13, innerRight13;

	CalculateInnerPoints(corner12, innerLeft12, innerRight12);
	CalculateInnerPoints(corner23, innerLeft23, innerRight23);
	CalculateInnerPoints(corner13, innerLeft13, innerRight13);

	XMFLOAT3 Q1, Q2, Q3;
	// calculate Qs
	// Determine how to check which corner is which (figure out how to acces the points which are made from the respective curve

	XMFLOAT3 top = (Q1 + Q2 + Q3) * (1.f / 3.f);
	XMFLOAT3 P1, P2, P3;
	P1 = (2.f * Q1 + top) * (1.f / 3.f);
	P2 = (2.f * Q2 + top) * (1.f / 3.f);
	P3 = (2.f * Q3 + top) * (1.f / 3.f);

	// calculate the last two inner points
	// this vectors are directed like so 
	// P0 -------------- ^ --------------- Top ( the arrow in the middle)
	DirectX::XMFLOAT3 curve1LeftInnerVec = CalculateVectorFromVectorField(XMFLOAT3(),XMFLOAT3(),XMFLOAT3(),XMFLOAT3(),2.f / 3.f);	
	DirectX::XMFLOAT3 curve2LeftInnerVec = CalculateVectorFromVectorField(XMFLOAT3(),XMFLOAT3(),XMFLOAT3(),XMFLOAT3(),2.f / 3.f);
	DirectX::XMFLOAT3 curve3LeftInnerVec = CalculateVectorFromVectorField(XMFLOAT3(),XMFLOAT3(),XMFLOAT3(),XMFLOAT3(),2.f / 3.f);

	//Get point in the 2/3rds of the curve to the top and this point +/- vector gives the two points taht should be calculated

	DirectX::XMFLOAT3 twoThirds1 = DirectX::XMFLOAT3(); //Calculate
	DirectX::XMFLOAT3 twoThirds2 = DirectX::XMFLOAT3(); //Calculate
	DirectX::XMFLOAT3 twoThirds3 = DirectX::XMFLOAT3(); //Calculate

	// check the orientation of the curves
	DirectX::XMFLOAT3 patch12LastLeftInner, patch12LastRightInner;
	DirectX::XMFLOAT3 patch23LastLeftInner, patch23LastRightInner;
	DirectX::XMFLOAT3 patch13LastLeftInner, patch13LastRightInner;

	// For example if patch 12 has 1 as the left the last inner left will be twoThirds1 + curve1LeftInnerVec 
	// if the 1 is on the right it will be twoThirds1 - curve1LeftInnerVec

	// Fill draw data for each patch
	FillPatchDrawData(); // call for patch1
	FillPatchDrawData(); // call for patch2
	FillPatchDrawData(); // call for patch3
}

void GregoryPatch::FillPatchDrawData(
	std::vector<DirectX::XMFLOAT3> left, 
	std::vector<DirectX::XMFLOAT3> right, 
	std::vector<DirectX::XMFLOAT3> innerLeft, 
	std::vector<DirectX::XMFLOAT3> innerRight, 
	DirectX::XMFLOAT3 outerLastLeft, DirectX::XMFLOAT3 outerLastRight, 
	DirectX::XMFLOAT3 innerLastLeft, DirectX::XMFLOAT3 innerLastRight, 
	DirectX::XMFLOAT3 top)
{
	
}

DirectX::XMFLOAT3 GregoryPatch::CalculateVectorFromVectorField(DirectX::XMFLOAT3 a0, DirectX::XMFLOAT3 b0, DirectX::XMFLOAT3 a3, DirectX::XMFLOAT3 b3, float t)
{
	DirectX::XMFLOAT3 vector;
	DirectX::XMFLOAT3 g0, g1, g2;
	g0 = (a0 + b0) * 0.5f;
	g2 = (a3 + b3) * 0.5f;
	g1 = (g0 + g2) * 0.5f;
	float u = 1.f - t;

	DirectX::XMFLOAT3 g0p, g1p;

	g0p = u * g0 + t * g1;
	g1p = u * g1 + t * g2;
	vector = u * g0p + t * g1p;
	return vector;
}

void GregoryPatch::FillReferences(std::vector<std::weak_ptr<Node>>& edge1, std::vector<std::weak_ptr<Node>>& edgePrev1, std::vector<std::weak_ptr<Node>>& edge2, std::vector<std::weak_ptr<Node>>& edgePrev2, std::vector<std::weak_ptr<Node>>& edge3, std::vector<std::weak_ptr<Node>>& edgePrev3)
{
	// 0 - 3  - edge1
	// 4 - 7  - edgePrev1
	// 8 - 11 - edge2 
	// etc
	auto refs = GetReferences().GetAllRef();

	for (int i = 0; i < 4; i++)
	{
		edge1.push_back(refs[i].m_refered);
		edgePrev1.push_back(refs[i+4].m_refered);

		edge2.push_back(refs[i+8].m_refered);
		edgePrev2.push_back(refs[i+12].m_refered);

		edge3.push_back(refs[i+16].m_refered);
		edgePrev3.push_back(refs[i+20].m_refered);
	}	
}

void GregoryPatch::CalculateInnerPoints(PatchIntersectionDescription& corner,
	std::vector<DirectX::XMFLOAT3>& innerLeft,
	std::vector<DirectX::XMFLOAT3>& innerRight)
{
	for (int i = 0; i < 3; i++)
	{
		auto posL = (2.f * corner.leftRes[i] + (-1.f) * corner.leftPrevRes[i]);
		innerLeft.push_back(posL);

		auto posR = (2.f * corner.rightRes[i] + (-1.f) * corner.rightPrevRes[i]);
		innerRight.push_back(posR);
	}

}

PatchIntersectionDescription GregoryPatch::GetPatchDefiningPointsAtCorner(
	std::vector<std::weak_ptr<Node>> adjacentEdge1, 
	std::vector<std::weak_ptr<Node>> adjacentEdge2, 
	std::vector<std::weak_ptr<Node>> prevEdge1, 
	std::vector<std::weak_ptr<Node>> prevEdge2)
{
	PatchIntersectionDescription res;
	// We want to have order so that the common point is the last poitn of the "lefT" 
	// vector and the first of the "right" vector
	int beg = 0;
	int end = 3;

	auto e1 = DivideBernsteinCurve(adjacentEdge1);
	auto e2 = DivideBernsteinCurve(adjacentEdge2);
	auto pe1 = DivideBernsteinCurve(prevEdge1);
	auto pe2 = DivideBernsteinCurve(prevEdge2);
	if (adjacentEdge1[end].lock() == adjacentEdge2[beg].lock())
	{
		res.leftRes = e1->midEnd;
		res.leftPrevRes = pe1->midEnd;

		res.rightRes = e2->begMid;
		res.rightPrevRes = pe2->begMid;
	}
	else if (adjacentEdge1[end].lock() == adjacentEdge2[end].lock())
	{
		res.leftRes = e1->midEnd;
		res.leftPrevRes = pe1->midEnd;

		res.rightRes = RevertOrder(e2->midEnd);
		res.rightPrevRes = RevertOrder(pe2->midEnd);
	}
	else if (adjacentEdge1[beg].lock() == adjacentEdge2[beg].lock())
	{
		res.leftRes = RevertOrder(e2->begMid);
		res.leftPrevRes = RevertOrder(pe2->begMid);

		res.rightRes = e1->begMid;
		res.rightPrevRes = pe1->begMid;
	}
	else if (adjacentEdge1[beg].lock() == adjacentEdge2[end].lock())
	{
		res.leftRes = e2->midEnd;
		res.leftPrevRes = pe2->midEnd;

		res.rightRes = e1->begMid;
		res.rightPrevRes = pe1->begMid;
	}
	else {
		throw ArgumentException();
	}

	delete e1;
	delete e2;
	delete pe1;
	delete pe2;
	return res;
}

std::vector<XMFLOAT3> GregoryPatch::RevertOrder(std::vector<XMFLOAT3> vector)
{
	std::vector<XMFLOAT3> res;

	for (int i = vector.size() - 1; i >= 0; i--)
	{
		res.push_back(vector[i]);
	}

	return res;
}
