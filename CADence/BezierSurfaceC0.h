#pragma once
#include <DirectXMath.h>
#include "MeshObject.h"
#include "BezierPatch.h"
#include "BezierCurve.h"
#include "Node.h"
class BezierSurfaceC0 : public MeshObject
{
public:
	BezierSurfaceC0(std::vector<std::shared_ptr<Node>> patches);
	virtual ~BezierSurfaceC0();

	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void UpdateObject() override;
	bool GetIsModified() override;

private:
	std::vector<std::shared_ptr<Node>> m_patches;
	void SetPolygonColor();
	void SetDisplayPolygon();
	void SetDivisions();

	int m_divisionsU;
	int m_divisionsV;
	bool m_displayPatchesPolygon;
	DirectX::XMFLOAT3 m_polygonsColor;
};