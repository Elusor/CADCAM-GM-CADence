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
	IntersectionFinder(ObjectFactory* factory);
	// TODO change return type to intersection curve
	void FindInterSection(IParametricSurface* surface1, IParametricSurface* surface2);

private:
	float m_step;
	float m_precision;	
	ObjectFactory* m_factory;

	void DetermineAffectedSurfaces(IParametricSurface* surface1,
		IParametricSurface* surface2,
		std::vector<IParametricSurface*>& pSurfs,
		std::vector<IParametricSurface*>& qSurfs);

	DirectX::XMFLOAT3 FindFirstIntersectionPoint(
		IParametricSurface* surface1,
		ParameterPair& surf1Params,
		IParametricSurface* surface2,
		ParameterPair& surf2Params);

	std::vector<std::shared_ptr<Node>> FindOtherIntersectionPoints(
		IParametricSurface* surface1, ParameterPair surf1Params,
		IParametricSurface* surface2, ParameterPair surf2Params,
		DirectX::XMFLOAT3 firstPoint);

	bool FindNextPoint(
		IParametricSurface* surface1, ParameterPair& surf1Params,
		IParametricSurface* surface2, ParameterPair& surf2Params,
		DirectX::XMFLOAT3 prevPoint,
		DirectX::XMFLOAT3& pos);

	DirectX::XMFLOAT3 GetSurfaceNormal(IParametricSurface* surface, ParameterPair params);
};