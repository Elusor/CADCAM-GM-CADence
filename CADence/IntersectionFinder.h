#pragma once
#include <vector>
#include <xmemory>
#include "IParametricSurface.h"
#include "Node.h"
#include "ParamUtils.h"
#include "ObjectReferences.h"

class ObjectFactory;
enum IntersectedSurface;

struct IntersectionPointSearchData
{
	IntersectionPointSearchData() {};

	bool found = false;
	ParameterQuad params;
	DirectX::XMFLOAT3 pos = { 0.f, 0.f, 0.f };
};

class IntersectionFinder
{
public:
	IntersectionFinder(Scene* scene);
	bool AreObjectIntersectable(std::weak_ptr<Node> obj1, std::weak_ptr<Node> obj2);
	// TODO change return type to intersection curve
	void FindInterSection(ObjectRef qSurfNode, ObjectRef pSurfNode);
	void FindIntersectionWithCursor(ObjectRef obj1, ObjectRef obj2, DirectX::XMFLOAT3 cursorPos);
	void CreateParamsGui();
private:

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

	bool FindFirstIntersectionPoint(
		IParametricSurface* surface1,
		ParameterPair& surf1Params,
		IParametricSurface* surface2,
		ParameterPair& surf2Params,
		DirectX::XMFLOAT3& point);

	void FindOtherIntersectionPoints(
		IParametricSurface* surface1, ParameterPair surf1Params,
		std::vector<DirectX::XMFLOAT2>& surf1ParamsList,
		IParametricSurface* surface2, ParameterPair surf2Params,
		std::vector<DirectX::XMFLOAT2>& surf2ParamsList,
		DirectX::XMFLOAT3 firstPoint);

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

	IntersectionPointSearchData SimpleGradient(
		IParametricSurface* qSurface,
		ParameterPair qSurfParams,
		IParametricSurface* pSurface,
		ParameterPair pSurfParams);

	IntersectionPointSearchData SimpleGradientForCursor(
		IParametricSurface* qSurface,
		ParameterPair qSurfParams,
		DirectX::XMFLOAT3 cursorPos);
};