#pragma once
#include <vector>
#include <DirectXMath.h>
#include "IParametricSurface.h"
#include "MeshObject.h"

enum IntersectedSurface
{
	SurfaceQ,
	SurfaceP
};

class IntersectionCurve: MeshObject
{
public:
	IntersectionCurve();
	void Initialize(
		IParametricSurface* qSurface, std::vector<DirectX::XMFLOAT2> qParameters,
		IParametricSurface* pSurface, std::vector<DirectX::XMFLOAT2> pParameters);
	
	// Object Unique methods
	std::vector<DirectX::XMFLOAT2> GetParameterList(IntersectedSurface surface);
	std::vector<DirectX::XMFLOAT3> GetPointPositions();

private:
	IParametricSurface* m_qSurface;
	IParametricSurface* m_pSurface;
	std::vector<DirectX::XMFLOAT2> m_qParameters;
	std::vector<DirectX::XMFLOAT2> m_pParameters;

	std::vector<DirectX::XMFLOAT3> m_positions;	
};