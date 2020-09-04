#pragma once
#include "MeshObject.h"

class Node;

struct PatchIntersectionDescription
{
	bool edgesSwapped = false;
	std::vector<DirectX::XMFLOAT3> leftRes;
	std::vector<DirectX::XMFLOAT3> leftPrevRes;
	std::vector<DirectX::XMFLOAT3> rightRes;
	std::vector<DirectX::XMFLOAT3> rightPrevRes;
};
//     Patch 1
//1 -------.1--------
//
//2 -------.1--------B
//				     |  |
//				     |  |
//				     |  |
//				    .2 .2  Patch 2
//				     |  |
//				     |  |
//				     |  |
//				     3  4
// In this situation .1 - B is consideres left res and B -.2 is considered right res

class GregoryPatch : public MeshObject<VertexPositionColor>
{
public:
	GregoryPatch();
	~GregoryPatch();

	void Initialize(
		std::vector<std::weak_ptr<Node>> edge1,
		std::vector<std::weak_ptr<Node>> edgePrev1,
		std::vector<std::weak_ptr<Node>> edge2,
		std::vector<std::weak_ptr<Node>> edgePrev2,
		std::vector<std::weak_ptr<Node>> edge3,
		std::vector<std::weak_ptr<Node>> edgePrev3);

	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void UpdateObject() override;
	bool GetIsModified() override;

protected:
	int m_uSize = 4;
	int m_vSize = 4;
	MeshDescription<VertexPositionColor> m_UDesc, m_VDesc;
	MeshDescription<VertexPositionColor> m_vectors;
	bool m_renderVectors = false;

	std::vector<DirectX::XMFLOAT3> m_patch1Positions;
	std::vector<DirectX::XMFLOAT3> m_patch2Positions;
	std::vector<DirectX::XMFLOAT3> m_patch3Positions;

	virtual void RenderPatch(std::unique_ptr<RenderState>& renderState);
	virtual void RenderPolygon(std::unique_ptr<RenderState>& renderState);
private:
	void CalculateGergoryPositions();

	void FindVectorFieldBase(
		std::vector<std::weak_ptr<Node>> edge1, std::vector<std::weak_ptr<Node>> edge2,
		DirectX::XMFLOAT3 pEdge1, DirectX::XMFLOAT3 pEdge2, DirectX::XMFLOAT3 pEdge3, DirectX::XMFLOAT3 top,
		DirectX::XMFLOAT3& leftVec, DirectX::XMFLOAT3& rightVec, bool edgesSwapped);

	XMFLOAT3 CalcBernstein(
		DirectX::XMFLOAT3 b0,
		DirectX::XMFLOAT3 b1,
		DirectX::XMFLOAT3 b2,
		DirectX::XMFLOAT3 b3,
		float t);

	void CalculateAuxiliaryPoints(
		std::vector<DirectX::XMFLOAT3> left,
		std::vector<DirectX::XMFLOAT3> right,
		std::vector<DirectX::XMFLOAT3> innerLeft,
		std::vector<DirectX::XMFLOAT3> innerRight,
		DirectX::XMFLOAT3& auxLeft, DirectX::XMFLOAT3& auxRight);

	void CalculateLastOuterPoint(
		DirectX::XMFLOAT3 auxLeft,
		DirectX::XMFLOAT3 auxRight,
		DirectX::XMFLOAT3 top,
		DirectX::XMFLOAT3& lastOuterLeft,
		DirectX::XMFLOAT3& lastOuterRight);

	void FillPatchDrawData(
		std::vector<DirectX::XMFLOAT3> left,
		std::vector<DirectX::XMFLOAT3> right,
		std::vector<DirectX::XMFLOAT3> innerLeft,
		std::vector<DirectX::XMFLOAT3> innerRight,
		DirectX::XMFLOAT3 outerLastLeft,
		DirectX::XMFLOAT3 outerLastRight,
		DirectX::XMFLOAT3 innerLastLeft,
		DirectX::XMFLOAT3 innerLastRight,
		DirectX::XMFLOAT3 top,
		std::vector<DirectX::XMFLOAT3>& filledPositions);

	DirectX::XMFLOAT3 CalculateVectorFromVectorField(
		DirectX::XMFLOAT3 a0, DirectX::XMFLOAT3 b0, 
		DirectX::XMFLOAT3 a3, DirectX::XMFLOAT3 b3, float t);

	void CalculateInnerPoints(PatchIntersectionDescription& corner,
		std::vector<DirectX::XMFLOAT3>& innerLeft,
		std::vector<DirectX::XMFLOAT3>& innerRight);
	
	void FillReferences(
		std::vector<std::weak_ptr<Node>>& edge1,
		std::vector<std::weak_ptr<Node>>& edgePrev1,
		std::vector<std::weak_ptr<Node>>& edge2,
		std::vector<std::weak_ptr<Node>>& edgePrev2,
		std::vector<std::weak_ptr<Node>>& edge3,
		std::vector<std::weak_ptr<Node>>& edgePrev3);

	PatchIntersectionDescription GetPatchDefiningPointsAtCorner(
		std::vector<std::weak_ptr<Node>> adjacentEdge1,
		std::vector<std::weak_ptr<Node>> adjacentEdge2,
		std::vector<std::weak_ptr<Node>> prevEdge1,
		std::vector<std::weak_ptr<Node>> prevEdge2);

	std::vector<XMFLOAT3> RevertOrder(std::vector<XMFLOAT3> vector);
};