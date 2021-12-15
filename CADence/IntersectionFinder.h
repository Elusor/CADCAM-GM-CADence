#pragma once
#include <vector>
#include <xmemory>
#include "IParametricSurface.h"
#include "Node.h"
#include "ParamUtils.h"
#include "ObjectReferences.h"
#include "IntersectionSearchResult.h"
#include "IntersectionSearchResultOneDir.h"

class ObjectFactory;
enum IntersectedSurface;

struct IntersectionCurveData
{
	bool isFound;
	std::vector<DirectX::XMFLOAT2> surfQParams;
	std::vector<DirectX::XMFLOAT2> surfPParams;
	bool surfQClosed;
	bool surfPClosed;
	bool foundBegPoints;
};

struct ClampedPointData
{
	DirectX::XMFLOAT4 params;
	DirectX::XMFLOAT4 distances;
};

struct IntersectionPointSearchData
{
	IntersectionPointSearchData() {};

	bool found = false;
	bool qPointOnBorder = false;
	bool pPointOnBorder = false;

	ParameterQuad params;
	DirectX::XMFLOAT3 pos = { 0.f, 0.f, 0.f };
	std::vector<DirectX::XMFLOAT4> auxPoints;
};

class IntersectionFinder
{
public:
	IntersectionFinder(Scene* scene);
	bool AreObjectIntersectable(std::weak_ptr<Node> obj1, std::weak_ptr<Node> obj2);
	// TODO change return type to intersection curve
	void FindInterSection(ObjectRef qSurfNode, ObjectRef pSurfNode);
	IntersectionCurveData FindInterSection(IParametricSurface* qSurf, IParametricSurface* pSurf);
	void FindIntersectionWithCursor(ObjectRef obj1, ObjectRef obj2, DirectX::XMFLOAT3 cursorPos);
	IntersectionCurveData FindIntersectionWithCursor(IParametricSurface* qSurf, IParametricSurface* pSurf, DirectX::XMFLOAT3 cursorPos);
	void CreateParamsGui();
private:

	bool m_addCappedCurves;
	int m_minPointCount;
	int m_oneDirPointCap;
	float m_cursorSamples;
	float m_samples;

	float m_step;
	float m_precision;
	float m_alphaPrecision;
	float m_goldenRatioPrecision;
	float m_gradientPrecision;
	float m_cursorCGprecision;
	float m_loopPrecision;
	int m_iterationCounter;
	ObjectFactory* m_factory;
	Scene* m_scene;

	DirectX::XMFLOAT4X4 CalculateDerivativeMatrix(
		IParametricSurface* surface1, IParametricSurface* surface2,
		ParameterQuad x_k, DirectX::XMFLOAT3 stepDir);

	DirectX::XMFLOAT4 CalculateIntersectionDistanceFunctionValue(
		IParametricSurface* qSurface,
		IParametricSurface* pSurface,
		ParameterQuad parameters,
		DirectX::XMFLOAT3 prevPoint,
		DirectX::XMFLOAT3 stepDir,
		float stepDist);

	DirectX::XMFLOAT4 CalculateOptimalPointInDirection(
		IParametricSurface* qSurf, ParameterPair qParams,
		IParametricSurface* pSurf, ParameterPair pParams,
		DirectX::XMFLOAT4 x_k,
		DirectX::XMFLOAT4 searchDir);

	DirectX::XMFLOAT3 CalculateStepDirection(
		IParametricSurface* qSurf,
		IParametricSurface* pSurf,
		ParameterQuad params,
		bool reverseDirection);

	IntersectionPointSearchData FindBoundaryPoint(
		ParameterQuad outParams,
		ParameterQuad inParams,
		IParametricSurface* qSurface,
		IParametricSurface* pSurface,
		float step);

	IntersectionCurveData FindIntersectionForParameters(
		IParametricSurface* qSurface,
		IParametricSurface* pSurface,
		ParameterQuad params, bool selfIntersect);

	bool FindFirstIntersectionPoint(
		IParametricSurface* surface1,
		ParameterPair& surf1Params,
		IParametricSurface* surface2,
		ParameterPair& surf2Params,
		DirectX::XMFLOAT3& point);

	IntersectionSearchResult FindOtherIntersectionPoints(
		IParametricSurface* surfaceQ, ParameterPair surfQParams,
		IParametricSurface* surfaceP, ParameterPair surfPParams,
		DirectX::XMFLOAT3 firstPoint);

	IntersectionSearchResultOneDir FindPointsInDirection(
		IParametricSurface* qSurface, IParametricSurface* pSurface, ParameterQuad startParams,
		bool direction, bool checkLooped, int pointCap, DirectX::XMFLOAT3 firstPoint);

	IntersectionPointSearchData FindNextPoint(
		IParametricSurface* qSurf,
		IParametricSurface* pSurf,
		ParameterQuad parameters,
		DirectX::XMFLOAT3 prevPoint,
		bool reverseDirection, float step);

	IntersectionPointSearchData FindNextPointAdaptiveStep(
		IParametricSurface* qSurf,
		IParametricSurface* pSurf,
		ParameterQuad parameters,
		DirectX::XMFLOAT3 prevPoint,
		bool reverseDirection,
		float step);

	std::vector<DirectX::XMFLOAT4> GetC0AuxiliaryPoints(
		IParametricSurface* qSurface, IParametricSurface* pSurface,
		ParameterQuad x_k, ParameterQuad x_prev, float step);

	std::vector<DirectX::XMFLOAT4> GetAuxiliaryPoints(
		IParametricSurface* qSurface, IParametricSurface* pSurface,
		ParameterQuad x_k, ParameterQuad x_prev);

	ParameterQuad GetWrappedParameters(
		IParametricSurface* qSurface,
		IParametricSurface* pSurface,
		ParameterQuad parameters);

	float GoldenRatioMethod(
		IParametricSurface* qSurface,
		IParametricSurface* pSurface,
		float a, float b,
		DirectX::XMFLOAT4 x_k,
		DirectX::XMFLOAT4 d_k);

	float GoldenRatioMethodForCursor(
		IParametricSurface* qSurface,
		float a, float b,
		DirectX::XMFLOAT2 x_k,
		DirectX::XMFLOAT2 d_k,
		DirectX::XMFLOAT3 cursorPos);

	// Move To IParametric Surface
	DirectX::XMFLOAT3 GetSurfaceNormal(IParametricSurface* surface, ParameterPair params);

	void SetDefaultSettingsValues();

	IntersectionPointSearchData SimpleGradient(
		IParametricSurface* qSurface,
		ParameterPair qSurfParams,
		IParametricSurface* pSurface,
		ParameterPair pSurfParams);

	IntersectionPointSearchData SimpleGradientForCursor(
		IParametricSurface* qSurface,
		ParameterPair qSurfParams,
		DirectX::XMFLOAT3 cursorPos);

	bool SurfacesAreParallel(IParametricSurface* qSurface, IParametricSurface* pSurface, ParameterQuad parameters);

	bool SimpleGradientResultCheck(IParametricSurface* qSurface, IParametricSurface* pSurface, IntersectionPointSearchData searchRes, bool isSelfIntersection);
};