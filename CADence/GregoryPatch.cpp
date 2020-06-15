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
	RenderPolygon(renderState);
}

bool GregoryPatch::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	bool patchChanged = false;	
	ImGui::Spacing();
	
	std::string dimDrag = "Grid density u" + GetIdentifier();
	patchChanged |= ImGui::DragInt(dimDrag.c_str(), &m_uSize, 1.0f, 2, 64);
	std::string dimDrag2 = "Grid density v" + GetIdentifier();
	patchChanged |= ImGui::DragInt(dimDrag2.c_str(), &m_vSize, 1.0f, 2, 64);
	if (m_uSize < 2)
		m_uSize = 2;
	if (m_uSize > 64)
		m_uSize = 64;

	if (m_vSize < 2)
		m_vSize = 2;
	if (m_vSize > 64)
		m_vSize = 64;

	ImGui::End();
	return patchChanged;
}

void GregoryPatch::UpdateObject()
{
	CalculateGergoryPositions();
	m_UDesc.vertices.clear();
	m_UDesc.indices.clear();
	m_UDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
	// Push lines in one direction
	for (int i = 0; i < m_patch1Positions.size(); i++)
	{
		m_UDesc.vertices.push_back(VertexPositionColor{
			{m_patch1Positions[i]},
			{XMFLOAT3(1.0f,0.0f,0.0f)} });
		m_UDesc.indices.push_back(i);
	}

	for (int i = 0; i < m_patch2Positions.size(); i++)
	{
		m_UDesc.vertices.push_back(VertexPositionColor{
			{m_patch2Positions[i]},
			{XMFLOAT3(0.0f,1.0f,0.0f)} });
		m_UDesc.indices.push_back(i + 20);
	}

	for (int i = 0; i < m_patch3Positions.size(); i++)
	{
		m_UDesc.vertices.push_back(VertexPositionColor{
			{m_patch3Positions[i]},
			{XMFLOAT3(0.0f,0.0f,1.0f)} });
		m_UDesc.indices.push_back(i + 40);
	}

	// TODO fix to represent "rotated patches"
	m_VDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
	// Push lines in one direction
	m_VDesc.vertices.clear();
	m_VDesc.indices.clear();
	m_VDesc.vertices = m_UDesc.vertices;

	for (int i = 0; i < 3; i++)
	{	
		m_VDesc.indices.push_back(3 + 20 * i);
		m_VDesc.indices.push_back(9 + 20 * i);
		m_VDesc.indices.push_back(15 + 20 * i);
		m_VDesc.indices.push_back(19 + 20 * i);
		m_VDesc.indices.push_back(2 + 20 * i);
		m_VDesc.indices.push_back(7 + 20 * i);
		m_VDesc.indices.push_back(8 + 20 * i);
		m_VDesc.indices.push_back(13 + 20 * i);
		m_VDesc.indices.push_back(14 + 20 * i);
		m_VDesc.indices.push_back(18 + 20 * i);
		m_VDesc.indices.push_back(1 + 20 * i);
		m_VDesc.indices.push_back(5 + 20 * i);
		m_VDesc.indices.push_back(6 + 20 * i);
		m_VDesc.indices.push_back(11 + 20 * i);
		m_VDesc.indices.push_back(12 + 20 * i);
		m_VDesc.indices.push_back(17 + 20 * i);
		m_VDesc.indices.push_back(0 + 20 * i);
		m_VDesc.indices.push_back(4 + 20 * i);
		m_VDesc.indices.push_back(10 + 20 * i);
		m_VDesc.indices.push_back(16 + 20 * i);
	}

	m_vectors.indices.clear();
	m_vectors.vertices.clear();
	m_vectors.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	m_vectors.vertices = m_UDesc.vertices;
	for (int i = 0; i < 3; i++)
	{
		// outer edges
		m_vectors.indices.push_back(0 + 20 * i);
		m_vectors.indices.push_back(1 + 20 * i);

		m_vectors.indices.push_back(1 + 20 * i);
		m_vectors.indices.push_back(2 + 20 * i);

		m_vectors.indices.push_back(2 + 20 * i);
		m_vectors.indices.push_back(3 + 20 * i);

		m_vectors.indices.push_back(3 + 20 * i);
		m_vectors.indices.push_back(9 + 20 * i);

		m_vectors.indices.push_back(9 + 20 * i);
		m_vectors.indices.push_back(15 + 20 * i);

		m_vectors.indices.push_back(15 + 20 * i);
		m_vectors.indices.push_back(19 + 20 * i);

		m_vectors.indices.push_back(19 + 20 * i);
		m_vectors.indices.push_back(18 + 20 * i);

		m_vectors.indices.push_back(18 + 20 * i);
		m_vectors.indices.push_back(17 + 20 * i);

		m_vectors.indices.push_back(17 + 20 * i);
		m_vectors.indices.push_back(16 + 20 * i);

		m_vectors.indices.push_back(16 + 20 * i);
		m_vectors.indices.push_back(10 + 20 * i);

		m_vectors.indices.push_back(10 + 20 * i);
		m_vectors.indices.push_back(4 + 20 * i);

		m_vectors.indices.push_back(4 + 20 * i);
		m_vectors.indices.push_back(0 + 20 * i);

		// inner edges
		m_vectors.indices.push_back(1 + 20 * i);
		m_vectors.indices.push_back(6 + 20 * i);

		m_vectors.indices.push_back(2 + 20 * i);
		m_vectors.indices.push_back(7 + 20 * i);

		m_vectors.indices.push_back(4 + 20 * i);
		m_vectors.indices.push_back(5 + 20 * i);

		m_vectors.indices.push_back(8 + 20 * i);
		m_vectors.indices.push_back(9 + 20 * i);

		m_vectors.indices.push_back(10 + 20 * i);
		m_vectors.indices.push_back(11 + 20 * i);

		m_vectors.indices.push_back(12 + 20 * i);
		m_vectors.indices.push_back(17 + 20 * i);

		m_vectors.indices.push_back(13 + 20 * i);
		m_vectors.indices.push_back(18 + 20 * i);

		m_vectors.indices.push_back(14 + 20 * i);
		m_vectors.indices.push_back(15 + 20 * i);		
	}

}

bool GregoryPatch::GetIsModified()
{
	bool modified = m_modified;
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

	context->HSSetShader(renderState->m_patchGregHullShader.get(), 0, 0);
	ID3D11Buffer* hsCb[] = { renderState->m_cbPatchDivisions.get() };
	context->HSSetConstantBuffers(0, 1, hsCb);

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

void GregoryPatch::RenderPolygon(std::unique_ptr<RenderState>& renderState)
{
	RenderMesh(renderState, m_vectors);
}

void GregoryPatch::CalculateGergoryPositions()
{
	std::vector<std::weak_ptr<Node>> edge1, edge2, edge3;
	std::vector<std::weak_ptr<Node>> edgePrev1, edgePrev2, edgePrev3;
	// fill these with object fron the ObjectReferences class
	FillReferences(edge1, edgePrev1, edge2, edgePrev2, edge3, edgePrev3);
	auto corner12 = GetPatchDefiningPointsAtCorner(edge1, edge2, edgePrev1, edgePrev2);
	auto corner23 = GetPatchDefiningPointsAtCorner(edge2, edge3, edgePrev2, edgePrev3);
	auto corner13 = GetPatchDefiningPointsAtCorner(edge1, edge3, edgePrev1, edgePrev3);	

	std::vector<DirectX::XMFLOAT3> innerLeft12, innerRight12;
	std::vector<DirectX::XMFLOAT3> innerLeft23, innerRight23;
	std::vector<DirectX::XMFLOAT3> innerLeft13, innerRight13;

	CalculateInnerPoints(corner12, innerLeft12, innerRight12);
	CalculateInnerPoints(corner23, innerLeft23, innerRight23);
	CalculateInnerPoints(corner13, innerLeft13, innerRight13);

	DirectX::XMFLOAT3 patch12auxLeft, patch12auxRight;
	DirectX::XMFLOAT3 patch23auxLeft, patch23auxRight;
	DirectX::XMFLOAT3 patch13auxLeft, patch13auxRight;

	// Calculate "Q" points withour the information about patch direction
	CalculateAuxiliaryPoints(corner12.leftRes, corner12.rightRes, innerLeft12, innerRight12, patch12auxLeft, patch12auxRight);
	CalculateAuxiliaryPoints(corner23.leftRes, corner23.rightRes, innerLeft23, innerRight23, patch23auxLeft, patch23auxRight);
	CalculateAuxiliaryPoints(corner13.leftRes, corner13.rightRes, innerLeft13, innerRight13, patch13auxLeft, patch13auxRight);

	XMFLOAT3 Q1, Q2, Q3;
	// calculate Qs
	// Determine how to check which corner is which (figure out how to acces the points which are made from the respective curve
	if (corner12.edgesSwapped){ 
		// edge 2 is the left edge
		Q2 = patch12auxLeft;
		Q1 = patch12auxRight;
	} else {
		// edge 1 is the left edge
		Q1 = patch12auxLeft;
		Q2 = patch12auxRight;
	}

	if (corner23.edgesSwapped){
		// edge 3 is the left edge
		Q3 = patch23auxLeft;
		Q2 = patch23auxRight;
	} else {
		// edge 2 is the left edgeS
		Q2 = patch23auxLeft;
		Q3 = patch23auxRight;
	}

	if (corner13.edgesSwapped) {
		// edge 3 is the left edge
		Q3 = patch13auxLeft;
		Q1 = patch13auxRight;
	} else {
		// edge 1 is the left edge
		Q1 = patch13auxLeft;
		Q3 = patch13auxRight;
	}

	XMFLOAT3 top = (Q1 + Q2 + Q3) * (1.f / 3.f); // cant it just be summ left and right from all patches and divide by 6?
	XMFLOAT3 P1, P2, P3;
	P1 = (2.f * Q1 + top) * (1.f / 3.f);
	P2 = (2.f * Q2 + top) * (1.f / 3.f);
	P3 = (2.f * Q3 + top) * (1.f / 3.f);

	// Assign p1 p2 and p3 in the patches corresponding places including the info about edge swap
	DirectX::XMFLOAT3 patch12LastOuterLeft, patch12LastOuterRight;
	DirectX::XMFLOAT3 patch23LastOuterLeft, patch23LastOuterRight;
	DirectX::XMFLOAT3 patch13LastOuterLeft, patch13LastOuterRight;

	// calculate  P1 P2 and P3 and assign them to their corresponding patches without knowing the relative patch location
	CalculateLastOuterPoint(patch12auxLeft, patch12auxRight, top, patch12LastOuterLeft, patch12LastOuterRight);
	CalculateLastOuterPoint(patch23auxLeft, patch23auxRight, top, patch23LastOuterLeft, patch23LastOuterRight);
	CalculateLastOuterPoint(patch13auxLeft, patch13auxRight, top, patch13LastOuterLeft, patch13LastOuterRight);

	DirectX::XMFLOAT3 leftTwoThirdsPatch12 = CalcBernstein(corner12.leftRes[0], innerLeft12[0], patch12LastOuterLeft, top, 2.f / 3.f);
	DirectX::XMFLOAT3 rightTwoThirdsPatch12 = CalcBernstein(corner12.rightRes[3], innerRight12[2], patch12LastOuterRight, top, 2.f / 3.f);
	DirectX::XMFLOAT3 leftTwoThirdsPatch23 = CalcBernstein(corner23.leftRes[0], innerLeft23[0], patch23LastOuterLeft, top, 2.f / 3.f);
	DirectX::XMFLOAT3 rightTwoThirdsPatch23 = CalcBernstein(corner23.rightRes[3], innerRight23[2], patch23LastOuterRight, top, 2.f / 3.f);
	DirectX::XMFLOAT3 leftTwoThirdsPatch13 = CalcBernstein(corner13.leftRes[0], innerLeft13[0], patch13LastOuterLeft, top, 2.f / 3.f);
	DirectX::XMFLOAT3 rightTwoThirdsPatch13 = CalcBernstein(corner13.rightRes[3], innerRight13[2], patch13LastOuterRight, top, 2.f / 3.f);

	// DETERMINE VECTORS FOR VECTOR FIELD CALCULATION

	// calculate the last two inner points
	// this vectors are directed like so 
	// P0 -------------- ^ --------------- Top ( the arrow in the middle)
	DirectX::XMFLOAT3 patch12LastLeftInner, patch12LastRightInner;
	DirectX::XMFLOAT3 patch23LastLeftInner, patch23LastRightInner;
	DirectX::XMFLOAT3 patch13LastLeftInner, patch13LastRightInner;

	DirectX::XMFLOAT3 patch12LastLeftInnerVec, patch12LastRightInnerVec;
	DirectX::XMFLOAT3 patch23LastLeftInnerVec, patch23LastRightInnerVec;
	DirectX::XMFLOAT3 patch13LastLeftInnerVec, patch13LastRightInnerVec;

	FindVectorFieldBase(edge1, edge2, P1, P2, P3, top, patch12LastLeftInnerVec, patch12LastRightInnerVec, corner12.edgesSwapped);
	FindVectorFieldBase(edge2, edge3, P2, P3, P1, top, patch23LastLeftInnerVec, patch23LastRightInnerVec, corner23.edgesSwapped);
	FindVectorFieldBase(edge1, edge3, P1, P3, P2, top, patch13LastLeftInnerVec, patch13LastRightInnerVec, corner13.edgesSwapped);

	patch12LastLeftInner = leftTwoThirdsPatch12 + patch12LastLeftInnerVec;
	patch23LastLeftInner = leftTwoThirdsPatch23 + patch23LastLeftInnerVec;
	patch13LastLeftInner = leftTwoThirdsPatch13 + patch13LastLeftInnerVec;
	patch12LastRightInner = rightTwoThirdsPatch12 + patch12LastRightInnerVec;
	patch23LastRightInner = rightTwoThirdsPatch23 + patch23LastRightInnerVec;
	patch13LastRightInner = rightTwoThirdsPatch13 + patch13LastRightInnerVec;

	// Fill draw data for each patch
	FillPatchDrawData(
		corner12.leftRes,
		corner12.rightRes, 
		innerLeft12, innerRight12, 
		patch12LastOuterLeft, patch12LastOuterRight,
		patch12LastLeftInner, patch12LastRightInner, top,
		m_patch1Positions); // call for patch12
	FillPatchDrawData(
		corner23.leftRes, 
		corner23.rightRes, 
		innerLeft23, 
		innerRight23, 
		patch23LastOuterLeft, patch23LastOuterRight,
		patch23LastLeftInner, patch23LastRightInner, top,
		m_patch2Positions); // call for patch23
	FillPatchDrawData(
		corner13.leftRes,  corner13.rightRes, 
		innerLeft13, innerRight13, 
		patch13LastOuterLeft, patch13LastOuterRight,
		patch13LastLeftInner, patch13LastRightInner, top,
		m_patch3Positions); // call for patch13
}

void GregoryPatch::FindVectorFieldBase(
	std::vector<std::weak_ptr<Node>> edge1, std::vector<std::weak_ptr<Node>> edge2,
	DirectX::XMFLOAT3 pEdge1, DirectX::XMFLOAT3 pEdge2, DirectX::XMFLOAT3 otherPoint, DirectX::XMFLOAT3 top,
	DirectX::XMFLOAT3& leftVec, DirectX::XMFLOAT3& rightVec, bool edgesSwapped)
{
	XMFLOAT3 a0, b0, a3, b3;
	// rotate the point in a manner that the end points are the same
	// TODO SWAP THOSE EDGES
	std::vector<XMFLOAT3> edge1pos, edge2pos;
	for (int i = 0; i < 4; i++)
	{
		edge1pos.push_back(edge1[i].lock()->m_object->GetPosition());
		edge2pos.push_back(edge2[i].lock()->m_object->GetPosition());
	}

	int beg = 0, end = 3;
	if (edge1[end].lock() == edge2[end].lock()) {
		// done
	} else if (edge1[end].lock() == edge2[beg].lock()) {
		// swap edge2
		edge2pos = RevertOrder(edge2pos);
	} else if (edge1[beg].lock() == edge2[end].lock()) {
		// swap edge1
		edge1pos = RevertOrder(edge1pos);
	} else if (edge1[beg].lock() == edge2[beg].lock()) {
		// swap both
		edge1pos = RevertOrder(edge1pos);
		edge2pos = RevertOrder(edge2pos);
	}

	// Divide the rotated edges
	// Assume that the edges meet in the "end" that is the last vertex from both is common
	auto edge1Div = DivideBernsteinCurvePos(edge1pos);
	auto edge2Div = DivideBernsteinCurvePos(edge2pos);

	// For edge 1 
	a0 = edge1Div->begMid[3] - edge1Div->begMid[2];
	b0 = edge1Div->midEnd[1] - edge1Div->midEnd[0];
	a3 = top - otherPoint;
	b3 = pEdge2 - top;	
	auto vec1 = CalculateVectorFromVectorField(a0, b0, a3, b3, 2.f / 3.f);	

	// For edge 2
	a0 = edge2Div->begMid[3] - edge2Div->begMid[2];
	b0 = edge2Div->midEnd[1] - edge2Div->midEnd[0];
	a3 = top - otherPoint;
	b3 = pEdge1 - top;	
	auto vec2 =CalculateVectorFromVectorField(a0, b0, a3, b3, 2.f / 3.f);

	if (edgesSwapped)
	{
		leftVec = vec2;
		rightVec = vec1;
	}
	else {
		leftVec = vec1;
		rightVec = vec2;
	}
}

XMFLOAT3 GregoryPatch::CalcBernstein(DirectX::XMFLOAT3 b0, DirectX::XMFLOAT3 b1, DirectX::XMFLOAT3 b2, DirectX::XMFLOAT3 b3, float t)
{
	float u = 1.f - t;
	DirectX::XMFLOAT3 bp0, bp1, bp2;
	DirectX::XMFLOAT3 bb0, bb1;

	bp0 = u * b0 + t * b1;
	bp1 = u * b1 + t * b2;
	bp2 = u * b2 + t * b3;

	bb0 = u * bp0 + t * bp1;
	bb1 = u * bp1 + t * bp2;

	return bb0 * u + bb1 * t;
}

void GregoryPatch::CalculateAuxiliaryPoints(
	std::vector<DirectX::XMFLOAT3> left, 
	std::vector<DirectX::XMFLOAT3> right, 
	std::vector<DirectX::XMFLOAT3> innerLeft, 
	std::vector<DirectX::XMFLOAT3> innerRight,
	DirectX::XMFLOAT3& auxLeft, DirectX::XMFLOAT3& auxRight)
{
	auxLeft = (3 * innerLeft[0] + (-1.f) * left[0]) * 0.5f;
	auxRight = (3 * innerRight[2] + (-1.f) * right[3]) * 0.5f;
}

void GregoryPatch::CalculateLastOuterPoint(
	DirectX::XMFLOAT3 auxLeft, DirectX::XMFLOAT3 auxRight, DirectX::XMFLOAT3 top, 
	DirectX::XMFLOAT3& lastOuterLeft, DirectX::XMFLOAT3& lastOuterRight)
{
	lastOuterLeft = (2 * auxLeft + top) * (1.f / 3.f);
	lastOuterRight = (2 * auxRight + top) * (1.f / 3.f);
}

void GregoryPatch::FillPatchDrawData(
	std::vector<DirectX::XMFLOAT3> left, 
	std::vector<DirectX::XMFLOAT3> right, 
	std::vector<DirectX::XMFLOAT3> innerLeft, 
	std::vector<DirectX::XMFLOAT3> innerRight, 
	DirectX::XMFLOAT3 outerLastLeft, DirectX::XMFLOAT3 outerLastRight, 
	DirectX::XMFLOAT3 innerLastLeft, DirectX::XMFLOAT3 innerLastRight, 
	DirectX::XMFLOAT3 top, std::vector<DirectX::XMFLOAT3>& filledPositions)
{
	filledPositions.clear(); 

	// first row - assumes that the patch corner is in the upper right corner
	for (int i = 0; i < 4; i++)
	{
		filledPositions.push_back(left[i]);
	}

	// second row
	filledPositions.push_back(innerLeft[0]);
	filledPositions.push_back(innerLeft[1]);
	filledPositions.push_back(innerLeft[1]); // double point 
	filledPositions.push_back(innerLeft[2]);
	filledPositions.push_back(innerRight[0]);
	filledPositions.push_back(right[1]);

	// third row
	filledPositions.push_back(outerLastLeft);
	filledPositions.push_back(innerLastLeft);
	filledPositions.push_back(innerLastRight);
	filledPositions.push_back(innerRight[1]);
	filledPositions.push_back(innerRight[1]);
	filledPositions.push_back(right[2]);

	// fourth row
	filledPositions.push_back(top);
	filledPositions.push_back(outerLastRight);
	filledPositions.push_back(innerRight[2]);
	filledPositions.push_back(right[3]);
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
	}

	for (int i = 1; i < 4; i++)
	{
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
		res.edgesSwapped = true;
		res.leftRes = RevertOrder(e2->begMid);
		res.leftPrevRes = RevertOrder(pe2->begMid);

		res.rightRes = e1->begMid;
		res.rightPrevRes = pe1->begMid;
	}
	else if (adjacentEdge1[beg].lock() == adjacentEdge2[end].lock())
	{
		res.edgesSwapped = true;
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
