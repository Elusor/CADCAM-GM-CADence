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
	m_step = 0.05f;
	m_precision = 0.05f;
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
	IParametricSurface* qSurface, ParameterPair& qSurfParams, 
	IParametricSurface* pSurface, ParameterPair& pSurfParams, 
	DirectX::XMFLOAT3 prevPoint, DirectX::XMFLOAT3 stepDir, float step)
{
	DirectX::XMFLOAT4 funcVal;

	DirectX::XMFLOAT3 pt1 = qSurface->GetPoint(qSurfParams.u, qSurfParams.v);
	DirectX::XMFLOAT3 pt2 = pSurface->GetPoint(pSurfParams.u, pSurfParams.v);
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

void IntersectionFinder::CreateParamsGui()
{
	float eps = m_precision;
	std::string label = "Precision##IntersectionFinder";
	ImGui::DragFloat(label.c_str(), &eps, 0.05f, 0.05f, 2.f);

	float step = m_step;
	std::string label2 = "Step size##IntersectionFinder";
	ImGui::DragFloat(label2.c_str(), &step, 0.05f, 0.05f, 2.f);

	m_precision = eps;
	m_step = step;
}

DirectX::XMFLOAT4X4 CalculateHessian(
	IParametricSurface* qSurf, ParameterPair qParams,
	IParametricSurface* pSurf, ParameterPair pParams)
{
	// Assumes function F(x) = <Q(u,v)-P(s,t), Q(u,v)-P(s,t)>
	float u, v, s, t;
	u = qParams.u; 
	v = qParams.v;
	s = pParams.u;
	t = pParams.v;

	DirectX::XMFLOAT4X4 hessian;
	auto Q = qSurf->GetPoint(u, v);
	auto P = pSurf->GetPoint(s, t);

	auto Qdu = qSurf->GetTangent(u, v, TangentDir::AlongU);
	auto Qdv = qSurf->GetTangent(u, v, TangentDir::AlongV);
	auto Qdudv = qSurf->GetSecondDarivativeMixed(u, v);
	auto Qdudu = qSurf->GetSecondDarivativeSameDirection(u, v, TangentDir::AlongU);
	auto Qdvdv = qSurf->GetSecondDarivativeSameDirection(u, v, TangentDir::AlongV);	

	auto Pds = pSurf->GetTangent(s, t, TangentDir::AlongU);
	auto Pdt = pSurf->GetTangent(s, t, TangentDir::AlongV);
	auto Pdsdt = pSurf->GetSecondDarivativeMixed(s, t);
	auto Pdsds = pSurf->GetSecondDarivativeSameDirection(s, t, TangentDir::AlongU);
	auto Pdtdt = pSurf->GetSecondDarivativeSameDirection(s, t, TangentDir::AlongV);

	// Calculate Mixed derivatives Same Func
	hessian._21 = hessian._12 = (Dot(Qdudv, Q - P) + Dot(Qdu, Qdv)) * 2.f;
	hessian._43 = hessian._34 = (Dot(Pdsdt, P - Q) + Dot(Pds, Pdt)) * 2.f;
	// Calculate Mixed derivates Mixed Func
	hessian._31 = hessian._13 = -2.f * Dot(Qdu, Pds);
	hessian._41 = hessian._14 = -2.f * Dot(Qdu, Pdt);
	hessian._32 = hessian._23 = -2.f * Dot(Qdv, Pds);
	hessian._42 = hessian._24 = -2.f * Dot(Qdv, Pdt);

	// Calculate Same Derivatives
	hessian._11 = 2 * (Dot(Qdudu, Q - P) + Dot(Qdu, Qdu));
	hessian._22 = 2 * (Dot(Qdvdv, Q - P) + Dot(Qdv, Qdv));
	hessian._33 = 2 * (Dot(Pdsds, P - Q) + Dot(Pds, Pds));
	hessian._44 = 2 * (Dot(Pdtdt, P - Q) + Dot(Pdt, Pdt));

	return hessian;
}

DirectX::XMFLOAT4 CalculateGradient(
	IParametricSurface* qSurf, ParameterPair qParams,
	IParametricSurface* pSurf, ParameterPair pParams)
{
	// Assumes function F(x) = <Q(u,v)-P(s,t), Q(u,v)-P(s,t)>
	float u, v, s, t;
	u = qParams.u;
	v = qParams.v;
	s = pParams.u;
	t = pParams.v;

	auto Q = qSurf->GetPoint(u, v);
	auto P = pSurf->GetPoint(s, t);
	auto Qdu = qSurf->GetTangent(u, v, TangentDir::AlongU);
	auto Qdv = qSurf->GetTangent(u, v, TangentDir::AlongV);
	auto Pds = pSurf->GetTangent(s, t, TangentDir::AlongU);
	auto Pdt = pSurf->GetTangent(s, t, TangentDir::AlongV);

	float du = Dot(Qdu, 2 * (Q - P));
	float dv = Dot(Qdv, 2 * (Q - P));
	float ds = Dot(Pds, 2 * (P - Q));
	float dt = Dot(Pdt, 2 * (P - Q));

	return DirectX::XMFLOAT4(du, dv, ds, dt);
}

float CalculateAlpha(
	IParametricSurface* qSurf, ParameterPair qParams, 
	IParametricSurface* pSurf, ParameterPair pParams, 
	DirectX::XMFLOAT4 p)
{
	DirectX::XMFLOAT4X4 Hf = CalculateHessian(qSurf, qParams, pSurf, pParams);
	DirectX::XMFLOAT4 gradient = CalculateGradient(qSurf, qParams, pSurf, pParams);

	float alpha = -1.f * Mul(gradient, p) / Mul(Mul(p, Hf), p);
	return alpha;
}

float CalculateBeta(DirectX::XMFLOAT4 r_cur, DirectX::XMFLOAT4 r_prev)
{
	float beta;
	DirectX::XMFLOAT4 rDiff = r_cur = r_prev;
	beta = Mul(r_cur, rDiff) / Mul(r_prev, r_prev);	
	return max(beta, 0.0f);
}

bool IntersectionFinder::FindFirstIntersectionPoint(
	IParametricSurface* qSurface,
	ParameterPair& qSurfParams,
	IParametricSurface* pSurface,
	ParameterPair& pSurfParams,
	DirectX::XMFLOAT3& point)
{	
	bool found = false;
	DirectX::XMFLOAT4 x_k; // x = [ u, v, s, t ]

	// Initialize x_0
	x_k.x = qSurfParams.u; // u 
	x_k.y = qSurfParams.v; // v
	x_k.z = pSurfParams.u; // s 
	x_k.w = pSurfParams.v; // t

	DirectX::XMFLOAT4 r_k = -1.f * CalculateGradient(qSurface, qSurfParams, pSurface, pSurfParams);
	DirectX::XMFLOAT4 p_k = r_k;

	// TODO: Add condition so taht it is known if the method diverges
	int iterationCounter = 0;

	bool continueSearch = true;
	while (continueSearch)
	{
		ParameterPair prevQParams = ParameterPair{ x_k.x, x_k.y };
		ParameterPair prevPParams = ParameterPair{ x_k.z, x_k.w };

		float alpha = CalculateAlpha(qSurface, prevQParams, pSurface, prevPParams, p_k);

		// Move in a conjugated direction
		x_k = x_k + alpha * p_k;

		//CalculateBeta according to Polak-Ribiere
		ParameterPair curQParams = ParameterPair{ x_k.x, x_k.y };
		ParameterPair curPParams = ParameterPair{ x_k.z, x_k.w };
		auto r_prev = r_k;
		r_k = -1.f * CalculateGradient(qSurface, curQParams, pSurface, curPParams);
		float beta = CalculateBeta(r_k, r_prev);
		// Calculate new conjugated direction
		p_k = r_k + beta * p_k;

		// TODO: replace with a cleaner solution
		iterationCounter++;
		if (iterationCounter > 30)
		{
			found = false;
			continueSearch = false;
		}

		// TODO add the end condition
		assert(false);
		if (false)
		{
			continueSearch = false;
			found = true;
			qSurfParams = curQParams;
			pSurfParams = curPParams;
			point = qSurface->GetPoint(curQParams.u, curQParams.v);
		}	
	}	

	return found;
}

void IntersectionFinder::FindInterSection(IParametricSurface* surface1, IParametricSurface* surface2)
{
	// X0 = u, v, s, t
	// P(s,t)
	// Q(u,v)


	std::vector<DirectX::XMFLOAT3> points;
	std::vector<IParametricSurface*> pSurfs, qSurfs;

	qSurfs.push_back(surface1);
	pSurfs.push_back(surface2);

	// Find All intersecting sections from surface 1 (Ps) and surface 2 (Qs) and Find intersection points for each combination (P,Q)
	// TODO: FIND ALL AFFECTED PATCHES
	// DetermineAffectedSurfaces(surface1, surface2, pSurfs, qSurfs);

	for (IParametricSurface* p : pSurfs)
	{
		for (IParametricSurface* q : qSurfs)
		{
			// Sample p and q in a lot of differnt places and search for Intersection Points

			// For each P, Q			
			ParameterPair pParams, qParams;
			DirectX::XMFLOAT3 firstPoint;

			// Calculate first point
			if (FindFirstIntersectionPoint(q, qParams, p, pParams, firstPoint))
			{
				// Calculate other points
				auto points = FindOtherIntersectionPoints(q, qParams, p, pParams, firstPoint);
			}
		}
	}
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
	x_k.x = qSurfParams.u; // u
	x_k.y = qSurfParams.v; // v
	x_k.z = pSurfParams.u; // s
	x_k.w = pSurfParams.v; // t
	
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

		if (Dot(funcVal, funcVal) <= m_precision * m_precision) 
		{ // Solution is satisfying
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