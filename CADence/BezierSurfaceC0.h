#pragma once
#include <DirectXMath.h>
#include "MeshObject.h"
#include "BezierPatch.h"
#include "BezierCurve.h"
#include "Node.h"
#include "IParametricSurface.h"
#include "IntersectableSurface.h"
#include "SurfaceWrapDirection.h"

class BezierSurfaceC0 : public MeshObject<VertexPositionColor>, public IParametricSurface, public TrimmableSurface
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
	std::vector<std::weak_ptr<Node>> GetDisplayChildren() override;
	std::vector<std::weak_ptr<Node>> GetPatches(int& height, int& width);
	void SetPoints(std::shared_ptr<Node>** points, int width, int height);
	void SetPoints(std::vector<std::vector<std::weak_ptr<Node>>> points, int width, int height);
	std::weak_ptr<Node> GetPatch(int w, int h);
	
	// Inherited via IParametricSurface
	virtual ParameterPair GetMaxParameterValues() override;
	virtual ParameterPair GetNormalizedParams(float u, float v) override;
	DirectX::XMFLOAT2 GetParameterSpaceDistance(ParameterPair point1, ParameterPair point2);
	virtual DirectX::XMFLOAT3 GetPoint(float u, float v) override;
	virtual DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v) override;
	virtual bool IsWrappedInDirection(SurfaceWrapDirection wrapDir) override;
	bool ParamsInsideBounds(float u, float v) override;
	ParameterPair GetWrappedParams(float u, float v) override;
	float GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep) override;

	virtual void SetIntersectionData(IntersectionData data) override;
protected:
	std::vector<std::vector<std::weak_ptr<Node>>> m_points;
	SurfaceWrapDirection m_wrapDir;
	std::vector<std::shared_ptr<Node>> m_patches;
	
	int m_patchW;
	int m_patchH;

	int m_pointHeight;
	int m_pointWidth;

	int m_divisionsU;
	int m_divisionsV;
	bool m_displayPatchesPolygon;
	DirectX::XMFLOAT3 m_polygonsColor;

	bool m_recalculateMesh = false;

	void SetPolygonColor();
	void SetDisplayPolygon();
	void SetMeshColor();
	void SetDivisions();
	virtual BezierPatch* GetPatchAtParameter(float& u, float& v);

	void RenderObjectSpecificContextOptions(Scene& scene) override;
	void UpdateTrimmedChildrenMeshes(TrimmedSpace trimmedMesh);
	std::vector<unsigned short> DetermineIndicesForPatch(
		std::vector<VertexParameterColor> vertices, 
		std::vector<unsigned short> indices, int patchW, int patchH);
	std::vector<VertexParameterColor> ScaleVerticesForPatch(
		std::vector<VertexParameterColor> vertices, int patchW, int patchH);
};

class BezierSurfaceC2 : public BezierSurfaceC0
{
public:
	BezierSurfaceC2();
	BezierSurfaceC2(std::vector<std::shared_ptr<Node>> patches, int wCount, int hCount, SurfaceWrapDirection wrapDirection);
	virtual ~BezierSurfaceC2();
};