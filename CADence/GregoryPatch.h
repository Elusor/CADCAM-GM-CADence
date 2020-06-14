#pragma once
#include "MeshObject.h"

class Node;

struct PatchIntersectionDescription
{
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

class GregoryPatch : public MeshObject
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
	MeshDescription m_UDesc, m_VDesc;

	std::vector<DirectX::XMFLOAT3> m_patch1Positions;
	std::vector<DirectX::XMFLOAT3> m_patch2Positions;
	std::vector<DirectX::XMFLOAT3> m_patch3Positions;

	virtual void RenderPatch(std::unique_ptr<RenderState>& renderState);

private:
	void CalculateGergoryPositions();
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