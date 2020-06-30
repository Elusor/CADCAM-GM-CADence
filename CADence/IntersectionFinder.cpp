#pragma once
#include <DirectXMath.h>
#include <vector>
#include "IntersectionFinder.h"
#include "mathUtils.h"
#include "ObjectFactory.h"
#include "Node.h"
#include "GeometricFunctions.h"

IntersectionFinder::IntersectionFinder(ObjectFactory* factory)
{
	m_factory = factory;
}

DirectX::XMFLOAT4X4 IntersectionFinder::CalculateDerivativeMatrix(
	IParametricSurface* surface1, IParametricSurface* surface2,
	DirectX::XMFLOAT4 x_k, DirectX::XMFLOAT3 stepDir)
{
	DirectX::XMFLOAT4X4 derivatives;

	float u, v, s, t;
	u = x_k.x;
	v = x_k.y;
	s = x_k.z;
	t = x_k.w;

	auto col1 = surface1->GetTangent(u, v, TangentDir::AlongU);
	auto col2 = surface1->GetTangent(u, v, TangentDir::AlongV);
	auto col3 = -1.0f * surface2->GetTangent(s, t, TangentDir::AlongU);
	auto col4 = -1.0f * surface2->GetTangent(s, t, TangentDir::AlongV);

	float col3Last = Dot(surface2->GetTangent(s, t, TangentDir::AlongU), stepDir);
	float col4Last = Dot(surface2->GetTangent(s, t, TangentDir::AlongV), stepDir);

	derivatives.m[0][0] = col1.x;
	derivatives.m[1][0] = col1.y;
	derivatives.m[2][0] = col1.z;
	derivatives.m[3][0] = 0.0f;

	derivatives.m[0][1] = col2.x;
	derivatives.m[1][1] = col2.y;
	derivatives.m[2][1] = col2.z;
	derivatives.m[3][1] = 0.0f;

	derivatives.m[0][2] = col3.x;
	derivatives.m[1][2] = col3.y;
	derivatives.m[2][2] = col3.z;
	derivatives.m[3][2] = col3Last;

	derivatives.m[0][3] = col4.x;
	derivatives.m[1][3] = col4.y;
	derivatives.m[2][3] = col4.z;
	derivatives.m[3][3] = col4Last;

	return derivatives;
}

DirectX::XMFLOAT4 IntersectionFinder::CalculateIntersectionDistanceFunctionValue(
	IParametricSurface* surface1, ParameterPair& surf1Params, 
	IParametricSurface* surface2, ParameterPair& surf2Params, 
	DirectX::XMFLOAT3 prevPoint, DirectX::XMFLOAT3 stepDir, float step)
{
	DirectX::XMFLOAT4 funcVal;

	DirectX::XMFLOAT3 pt1 = surface1->GetPoint(surf1Params.u, surf1Params.v);
	DirectX::XMFLOAT3 pt2 = surface2->GetPoint(surf2Params.u, surf2Params.v);
	DirectX::XMFLOAT3 posDiff = pt1 - pt2;

	auto stepDiff = pt2 - prevPoint;
	float w = Dot(stepDiff, stepDir) - step;

	funcVal.x = posDiff.x;
	funcVal.y = posDiff.y;
	funcVal.z = posDiff.z;
	funcVal.w = w;

	return funcVal;
}

DirectX::XMFLOAT3 IntersectionFinder::CalculateStepDirection(
	IParametricSurface* qSurf, ParameterPair qSurfParams, 
	IParametricSurface* pSurf, ParameterPair pSurfParams)
{
	auto n1 = GetSurfaceNormal(qSurf, qSurfParams);
	auto n2 = GetSurfaceNormal(pSurf, pSurfParams);
	auto stepDir = Cross(n1, n2);
	float stepDirLen = sqrt(Dot(stepDir, stepDir));
	auto stepVersor = stepDir / stepDirLen;

	return stepVersor;
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
	IParametricSurface* qSurf, ParameterPair& qSurfParams,
	IParametricSurface* pSurf, ParameterPair& pSurfParams, 
	DirectX::XMFLOAT3 prevPoint,
	DirectX::XMFLOAT3& pos)
{	
	DirectX::XMFLOAT4 x_k;
	bool found = false;

	// Calculate Step direction
	auto stepVersor = CalculateStepDirection(qSurf, qSurfParams, pSurf, pSurfParams);
	
	// Initialize x_0
	x_k.x = qSurfParams.u;
	x_k.y = qSurfParams.v;
	x_k.z = pSurfParams.u;
	x_k.w = pSurfParams.v;
	
	// TODO: Add condition so taht it is known if the method diverges
	int iterationCounter = 0;

	bool continueNewtonCalculation = true;
	while (continueNewtonCalculation)
	{		
		// Calculate F(x)
		ParameterPair curQParams = ParameterPair{ x_k.x, x_k.y };
		ParameterPair curPParams = ParameterPair{ x_k.z, x_k.w };
		auto funcVal = -1.f * CalculateIntersectionDistanceFunctionValue(
			qSurf, curQParams, 
			pSurf, curPParams, 
			prevPoint, stepVersor, m_step);

		// Solution is satisfying
		if (Dot(funcVal, funcVal) <= m_precision * m_precision) {
			continueNewtonCalculation = false;
			found = true;
			pos = pSurf->GetPoint(curPParams.u, curPParams.v);

			// Update structures to hold current parameters data for use outside this func
			qSurfParams = curQParams;
			pSurfParams = curPParams;
		} 
		else 
		{ // Calculate next iteration
			// Create a linear equation system and solve it
			DirectX::XMFLOAT4X4 derMatrix = CalculateDerivativeMatrix(qSurf, pSurf, x_k, stepVersor);
			auto deltaX = Geom::SolveGEPP(derMatrix, funcVal);

			// Find the next point using Newton's method to solve linear equation system
			x_k = x_k + deltaX;
		}

		// TODO: replace with a cleaner solution
		iterationCounter++;
		if (iterationCounter > 30)
		{
			found = false;
			continueNewtonCalculation = false;
		}
	}

	return found;
}

DirectX::XMFLOAT3 IntersectionFinder::GetSurfaceNormal(IParametricSurface* surface, ParameterPair params)
{
	DirectX::XMFLOAT3 tu = surface->GetTangent(params.u, params.v, TangentDir::AlongU);
	DirectX::XMFLOAT3 tv = surface->GetTangent(params.u, params.v, TangentDir::AlongV);

	return Cross(tu, tv);
}