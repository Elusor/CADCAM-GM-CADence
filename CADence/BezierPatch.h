#pragma once
#include "MeshObject.h"
#include "Node.h"
#include <direct.h>
using namespace DirectX;
/*
Class describing Bezier patch.
Implementation requirements:
- grid of constant parameter curves
- obliczenia punktów siatki tworz¹cej powierzchnie powinny byæ przeprowadzane na procesorze graficznym???
- resizeable U/V grid size
-
*/
/*
	  ------------->
	| 0  ---  1 ---- 2 ---  3 -- u0
	| |       |      |     | 
	| |       |      |     | 
	| 4  ---  5 ---- 6 ---  7 -- u1
	| |       |      |     | 
	| |       |      |     | 
	| 8  ---  9 --- 1O --- 11 -- u2
	| |       |      |     |
	| |       |      |     |
	V  12 --- 13 --- 14 --- 15 -- u3
*/

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

	std::vector<std::weak_ptr<Node>> GetPoints(BoundaryDirection direction);

private:
	// Bezier patch unique
	std::weak_ptr<Node> m_u0[4];
	std::weak_ptr<Node> m_u1[4];
	std::weak_ptr<Node> m_u2[4];
	std::weak_ptr<Node> m_u3[4];

	int m_uSize = 4;
	int m_vSize = 4;

	void SetPoints(BoundaryDirection direction, std::vector<std::weak_ptr<Node>> points);
	void SetPoints(RowPlace row, std::vector<std::weak_ptr<Node>> points);
	

	XMMATRIX GetCoordinates(Coord coord);
};