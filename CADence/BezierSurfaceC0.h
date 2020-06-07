#pragma once
#include <DirectXMath.h>
#include "MeshObject.h"
#include "BezierPatch.h"
#include "BezierCurve.h"
#include "Node.h"

enum SurfaceWrapDirection
{
	Width,
	Height,
	None
};

class BezierSurfaceC0 : public MeshObject
{
public:
	BezierSurfaceC0(std::vector<std::shared_ptr<Node>> patches, int wCount, int hCount, SurfaceWrapDirection wrapDirection);
	virtual ~BezierSurfaceC0();

	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void UpdateObject() override;
	bool GetIsModified() override;
	void SetIsSelected(bool isSelected) override;
	void SetDivisions(int divsU, int divsV);
	int GetUDivisions();
	int GetVDivisions();
	void SetDisplayPolygon(bool displayPolygon);
	bool GetDisplayPolygon();
	SurfaceWrapDirection GetWrapDirection();

	std::vector<std::vector<std::weak_ptr<Node>>> GetPoints();
	void SetPoints(std::shared_ptr<Node>** points, int width, int height);
	void SetPoints(std::vector<std::vector<std::weak_ptr<Node>>> points, int width, int height);
private:
	std::vector<std::vector<std::weak_ptr<Node>>> m_points;
	SurfaceWrapDirection m_wrapDir;
	std::vector<std::shared_ptr<Node>> m_patches;
	void SetPolygonColor();
	void SetDisplayPolygon();
	void SetMeshColor();
	void SetDivisions();
	int m_patchW;
	int m_patchH;

	int m_divisionsU;
	int m_divisionsV;
	bool m_displayPatchesPolygon;
	DirectX::XMFLOAT3 m_polygonsColor;
};