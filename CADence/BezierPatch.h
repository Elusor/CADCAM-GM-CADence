#pragma once
#include "MeshObject.h"
#include "Node.h"
#include <direct.h>
#include <DirectXMath.h>
#include "IParametricSurface.h"
#include "bezierCalculator.h"
using namespace DirectX;

struct BezierPatchPointPos
{
	BezierCoeffs row0;
	BezierCoeffs row1;
	BezierCoeffs row2;
	BezierCoeffs row3;
};

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

class BezierPatch : public MeshObject, public IParametricSurface {
public:
	// Create a patch and allocates all points on the scene
	BezierPatch();
	virtual ~BezierPatch();

	void Initialize(std::vector<std::weak_ptr<Node>> first,
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

	BezierPatchPointPos GetPatchPointPositions();
	std::vector<std::weak_ptr<Node>> GetPoints(BoundaryDirection direction);
	std::vector<std::weak_ptr<Node>> GetPreBoundaryPoints(BoundaryDirection direction);
	std::vector<std::weak_ptr<Node>> GetPoints(RowPlace row);	

	// Inherited via IParametricSurface
	virtual DirectX::XMFLOAT3 GetPoint(float u, float v) override;
	virtual DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v) override;
protected:

	int m_uSize = 4;
	int m_vSize = 4;

	bool m_displayPolygon;
	MeshDescription m_PolygonDesc;
	MeshDescription m_UDesc, m_VDesc;

	/*void SetPoints(BoundaryDirection direction, std::vector<std::weak_ptr<Node>> points);
	void SetPoints(RowPlace row, std::vector<std::weak_ptr<Node>> points);*/
	virtual void RenderPolygon(std::unique_ptr<RenderState>& renderState);
	virtual void RenderPatch(std::unique_ptr<RenderState>& renderState);
	XMMATRIX GetCoordinates(Coord coord);	
};

class BezierPatchC2 : public BezierPatch {
public:
	BezierPatchC2();
	~BezierPatchC2();

	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void RenderPatch(std::unique_ptr<RenderState>& renderState) override;

	// Inherited via IParametricSurface
	virtual DirectX::XMFLOAT3 GetPoint(float u, float v) override;
	virtual DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v) override;
};