#pragma once
#include <DirectXMath.h>
#include <vector>
#include "IntersectionFinder.h"
#include "mathUtils.h"
#include "ObjectFactory.h"
#include "Node.h"

IntersectionFinder::IntersectionFinder(ObjectFactory* factory)
{
	m_factory = factory;
}

void IntersectionFinder::DetermineAffectedSurfaces(
	IParametricSurface* surface1, 
	IParametricSurface* surface2, 
	std::vector<IParametricSurface*>& pSurfs, 
	std::vector<IParametricSurface*>& qSurfs)
{
	// Return two vectors with affected surfaces 
	// Not implemented
	assert(false);
}

void IntersectionFinder::FindInterSection(IParametricSurface* surface1, IParametricSurface* surface2)
{
	std::vector<DirectX::XMFLOAT3> points;
	std::vector<IParametricSurface*> pSurfs, qSurfs;
	// Find All intersecting sections from surface 1 (Ps) and surface 2 (Qs) and Find intersection points for each combination (P,Q)
	// TODO: FIND ALL AFFECTED PATCHES
	DetermineAffectedSurfaces(surface1, surface2, pSurfs, qSurfs);
	
	for (IParametricSurface* p : pSurfs)
	{
		for (IParametricSurface* q : qSurfs)
		{
			// For each P, Q			
			ParameterPair pParams, qParams;

			// Calculate first point
			auto firstPt = FindFirstIntersectionPoint(p, pParams, q, qParams);

			// Calculate other points
			auto points = FindOtherIntersectionPoints(p, pParams, q, qParams, firstPt);
		}
	}
}

DirectX::XMFLOAT3 IntersectionFinder::FindFirstIntersectionPoint(
	IParametricSurface* surface1,
	ParameterPair& surf1Params, 
	IParametricSurface* surface2,
	ParameterPair& surf2Params)
{
	// Determine the position of the first intersection point ( Points?) and the parameter values for it 
	
	// CalculateHessian(surf1,surf2, x_k)
		// CalculateABDerivatives()
		// CalculateAADerivatives()
		// CalculateSameADerivatives()		
		// Return Matrix

	// CalculateAlpha(surface1, surface2, p)
		// grad = CalculateGrad();		
		// Hf = CalculateHessian(surf1,surf2)
		// return <-grad,p> / (p * Hf * p);

	// CalculateBeta(r_k+1, r_k)
		// beta = <r_k+1, (r_k+1 - r_k)>
		// beta /= <r_k,r_k>
		// return max(beta, 0)

	// Guess the initial value
	// x0 = initial value
	// r0 = -grad(f(x0))
	// p0 = r0

	bool someCondition = false;
	while (someCondition)
	{
		// alfa = CalculateAlpha
		// x_k+1 = x_k + alpha * pk
		// r_k+1 = -grad(f(xk))
		// beta_k+1 = CalculateBeta according to Polak-Ribben
		// p_k+1 = r_k+1 + beta_k+1 * p_k
	}	

	// Return the proper x_k
	return DirectX::XMFLOAT3();
}

std::vector<std::shared_ptr<Node>> IntersectionFinder::FindOtherIntersectionPoints(
	IParametricSurface* surface1, ParameterPair surf1Params,
	IParametricSurface* surface2, ParameterPair surf2Params, 
	DirectX::XMFLOAT3 firstPoint)
{
	std::vector<std::shared_ptr<Node>> result;
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	bool nextPointInRange = FindNextPoint(surface1, surf1Params, surface2, surf2Params, firstPoint, position);
	while(nextPointInRange)
	{
		// Construct a point at the fiven location and 
		auto pt = m_factory->CreatePoint();
		pt->m_object->SetPosition(position);
		result.push_back(pt);

		// TODO: Bind the point to the appropriate data structures
		// For example make a structure that stores intersection data and bind the corresponding point/(u,v) values there.

		// Search for the next point
		auto prevPos = position;
		nextPointInRange = FindNextPoint(surface1, surf1Params, surface2, surf2Params, prevPos, position);
	}


	// TODO: check if after returning shared_ptrs are not expired/invalid
	return result;
}

bool IntersectionFinder::FindNextPoint(
	IParametricSurface* surface1, ParameterPair& surf1Params,
	IParametricSurface* surface2, ParameterPair& surf2Params, 
	DirectX::XMFLOAT3 prevPoint,
	DirectX::XMFLOAT3& pos)
{
	DirectX::XMFLOAT3 p0 = surface1->GetPoint(surf1Params.u, surf1Params.v);
	bool found;

	// DirectX::XMFLOAT4X4 CalculateDerivativeMatrix(IParametricSurface surface1, IParametricSurface surface2, XMFLOAT4 x_k, T)
		// col1 = [-surface1.GetTangent(u,v, Direction::U), 0]
		// col2 = [-surface1.GetTangent(u,v, Direction::V), 0]
		// col3 = [surface2.GetTangent(s,t, Direction::U), dot(surface2.GetTangent(s,t, Direction::U), T)]
		// col4 = [surface2.GetTangent(s,t, Direction::V), dot(surface2.GetTangent(s,t, Direction::V), T)]

	// Solves Ax = b
	// DirectX::XMFLOAT4 SolveGEPP(DirectX::XMFLOAT4X4 A, DirectX::XMFLOAT4 b)
		// Implement linear equation system solver 
		

	auto n1 = GetSurfaceNormal(surface1, surf1Params);
	auto n2 = GetSurfaceNormal(surface2, surf2Params);

	auto stepDir = Cross(n1, n2);
	float stepDirLen = sqrt(Dot(stepDir, stepDir));
	auto stepVersor = stepDir / stepDirLen;

	// x0 = surf1Params + surf2Params	
	// xk = x0;

	bool newtonConvergenceCond = true;
	while (newtonConvergenceCond)
	{
		// values = -f(x_k);
		// derMatrix = CalculateDerivativeMatrix(surface1, surface2, x_k, T);

		// deltaX = SolveGEPP(derMat, values); 		

		// Find the next point using Newton's method to solve linear equation system
		// x_k+1 = xk + deltaX;

		// Update convergenceCondition
		// if(norm(f(x)) < eps)
			// newtonConvergenceCond = false;
			// pos = P(x_k+1);
	}

	return false;
}

DirectX::XMFLOAT3 IntersectionFinder::GetSurfaceNormal(IParametricSurface* surface, ParameterPair params)
{
	DirectX::XMFLOAT3 tu = surface->GetTangent(params.u, params.v, TangentDir::AlongU);
	DirectX::XMFLOAT3 tv = surface->GetTangent(params.u, params.v, TangentDir::AlongV);

	return Cross(tu, tv);
}