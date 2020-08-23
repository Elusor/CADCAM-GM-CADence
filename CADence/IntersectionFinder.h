#pragma once
#include <vector>
#include <xmemory>
#include "IParametricSurface.h"
#include "Node.h"
class ObjectFactory;

struct ParameterPair
{
	float u;
	float v;
};

class IntersectionFinder
{
public:
	IntersectionFinder(Scene* scene);
	// TODO change return type to intersection curve
	void FindInterSection(IParametricSurface* surface1, IParametricSurface* surface2);
	void FindIntersectionWithCursor(IParametricSurface* surface1, IParametricSurface* surface2, DirectX::XMFLOAT3 cursorPos);
	void CreateParamsGui();
private:
	float m_step;
	float m_precision;
	float m_alphaPrecision;
	float m_goldenRatioPrecision;
	float m_CGprecision;
	float m_cursorCGprecision;
	float m_loopPrecision;
	ObjectFactory* m_factory;
	Scene* m_scene;

	DirectX::XMFLOAT4X4 CalculateDerivativeMatrix(
		IParametricSurface* surface1, IParametricSurface* surface2,
		DirectX::XMFLOAT4 x_k, DirectX::XMFLOAT3 stepDir);

	DirectX::XMFLOAT4 CalculateIntersectionDistanceFunctionValue(
		IParametricSurface* surface1, ParameterPair& surf1Params,
		IParametricSurface* surface2, ParameterPair& surf2Params,
		DirectX::XMFLOAT3 prevPoint, DirectX::XMFLOAT3 stepDir, float step);

	DirectX::XMFLOAT4 CalculateOptimalPointInDirection(
		IParametricSurface* qSurf, ParameterPair qParams,
		IParametricSurface* pSurf, ParameterPair pParams,
		DirectX::XMFLOAT4 x_k,
		DirectX::XMFLOAT4 searchDir);

	DirectX::XMFLOAT3 CalculateStepDirection(
		IParametricSurface* surface1, ParameterPair surf1Params,
		IParametricSurface* surface2, ParameterPair surf2Params);

	void DetermineAffectedSurfaces(IParametricSurface* surface1,
		IParametricSurface* surface2,
		std::vector<IParametricSurface*>& pSurfs,
		std::vector<IParametricSurface*>& qSurfs);

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

	bool FindNextPoint(
		IParametricSurface* surface1, ParameterPair& surf1Params,
		IParametricSurface* surface2, ParameterPair& surf2Params,
		DirectX::XMFLOAT3 prevPoint,
		DirectX::XMFLOAT3& pos,
		bool reverseDirection);
	
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

	bool SimpleGradient(
		IParametricSurface* qSurface,
		ParameterPair& qSurfParams,
		IParametricSurface* pSurface,
		ParameterPair& pSurfParams,
		DirectX::XMFLOAT3& point);

	bool SimpleGradientForCursor(
		IParametricSurface* qSurface,
		ParameterPair& qSurfParams,
		DirectX::XMFLOAT3 cursorPos,
		DirectX::XMFLOAT3& point);
};