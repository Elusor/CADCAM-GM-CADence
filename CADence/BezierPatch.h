#pragma once
#include "MeshObject.h"
#include "Node.h"
#include <direct.h>
#include <DirectXMath.h>
using namespace DirectX;

enum Coord {
	Xpos,
	Ypos,
	Zpos
};

enum BoundaryDirection {
	Top,
	Bottom,
	Left,
	Right
};

enum RowPlace {
	First,
	Second,
	Third,
	Fourth
};

class BezierPatch : public MeshObject {
public:
	// Create a patch and allocates all points on the scene
	BezierPatch();
	virtual ~BezierPatch();
	// Create a patch with given edges (edges with size 0 will be allocated and new points will be created
	BezierPatch(
		std::vector<std::weak_ptr<Node>> top,
		std::vector<std::weak_ptr<Node>> bottom,
		std::vector<std::weak_ptr<Node>> left, 
		std::vector<std::weak_ptr<Node>> right,
		std::vector<std::weak_ptr<Node>> inner);

	BezierPatch(
		std::vector<std::weak_ptr<Node>> first,
		std::vector<std::weak_ptr<Node>> second,
		std::vector<std::weak_ptr<Node>> third,
		std::vector<std::weak_ptr<Node>> fourth);

	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void UpdateObject() override;
	bool GetIsModified() override;

	void SetDivisions(int divsU, int divsV);
	void SetPolygonVisible(bool state);
	void SetPolygonColor(DirectX::XMFLOAT3 col);
	void SetPatchColor(DirectX::XMFLOAT3 col);

	std::vector<std::weak_ptr<Node>> GetPoints(BoundaryDirection direction);
	std::vector<std::weak_ptr<Node>> GetPoints(RowPlace row);
private:
	// Bezier patch unique
	std::weak_ptr<Node> m_u0[4];
	std::weak_ptr<Node> m_u1[4];
	std::weak_ptr<Node> m_u2[4];
	std::weak_ptr<Node> m_u3[4];

	int m_uSize = 4;
	int m_vSize = 4;

	bool m_displayPolygon;
	MeshDescription m_PolygonDesc;

	MeshDescription m_UDesc, m_VDesc;

	void SetPoints(BoundaryDirection direction, std::vector<std::weak_ptr<Node>> points);
	void SetPoints(RowPlace row, std::vector<std::weak_ptr<Node>> points);
	void RenderPolygon(std::unique_ptr<RenderState>& renderState);
	void RenderPatch(std::unique_ptr<RenderState>& renderState);
	XMMATRIX GetCoordinates(Coord coord);
};