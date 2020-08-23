#pragma once
#include <DirectXMath.h>
#include <vector>
#include "IntersectionFinder.h"
#include "mathUtils.h"
#include "ObjectFactory.h"
#include "Node.h"
#include "GeometricFunctions.h"
#include "Scene.h"

IntersectionFinder::IntersectionFinder(Scene* scene)
{
	m_scene = scene;
	m_factory = scene->m_objectFactory.get();
	m_step = 0.5f; 
	m_loopPrecision = 0.1f;
	m_precision = 10E-7f;
	m_CGprecision = 10E-7f;
	m_cursorCGprecision = 0.1f;
	m_alphaPrecision = 10E-7f;
	m_goldenRatioPrecision = 10E-7f;
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
	auto col3 = surface2->GetTangent(s, t, TangentDir::AlongU);
	auto col4 = surface2->GetTangent(s, t, TangentDir::AlongV);

	float col3Last = Dot(col3, stepDir);
	float col4Last = Dot(col4, stepDir);

	derivatives.m[0][0] = col1.x;
	derivatives.m[1][0] = col1.y;
	derivatives.m[2][0] = col1.z;
	derivatives.m[3][0] = 0.0f;

	derivatives.m[0][1] = col2.x;
	derivatives.m[1][1] = col2.y;
	derivatives.m[2][1] = col2.z;
	derivatives.m[3][1] = 0.0f;

	derivatives.m[0][2] = -col3.x;
	derivatives.m[1][2] = -col3.y;
	derivatives.m[2][2] = -col3.z;
	derivatives.m[3][2] = col3Last;

	derivatives.m[0][3] = -col4.x;
	derivatives.m[1][3] = -col4.y;
	derivatives.m[2][3] = -col4.z;
	derivatives.m[3][3] = col4Last;

	return derivatives;
}

DirectX::XMFLOAT4 IntersectionFinder::CalculateIntersectionDistanceFunctionValue(
	IParametricSurface* qSurface, ParameterPair& qSurfParams, 
	IParametricSurface* pSurface, ParameterPair& pSurfParams, 
	DirectX::XMFLOAT3 prevPoint, DirectX::XMFLOAT3 stepDir, float step)
{
	// [Q - P, <P - prev, t> - d ]
	DirectX::XMFLOAT4 funcVal;

	DirectX::XMFLOAT3 Qpt = qSurface->GetPoint(qSurfParams.u, qSurfParams.v);
	DirectX::XMFLOAT3 Ppt = pSurface->GetPoint(pSurfParams.u, pSurfParams.v);
	DirectX::XMFLOAT3 posDiff = Qpt - Ppt;

	// TODO CHECK IF T IS A VERSOR (NORMALIZED)
	auto len = Dot(stepDir, stepDir);

	auto stepDiff = Ppt - prevPoint;
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
	ImGui::DragFloat(label.c_str(), &eps, 0.05f, 0.05f, 2.f, "%.1e");

	float step = m_step;
	std::string label2 = "Step size##IntersectionFinder";
	ImGui::DragFloat(label2.c_str(), &step, 0.05f, 0.05f, 2.f, "%.1e");

	float alphaprec = m_alphaPrecision;
	std::string label3 = "Alpha precision##IntersectionFinder";
	ImGui::DragFloat(label3.c_str(), &alphaprec, 0.05f, 0.05f, 2.f, "%.1e");

	float goldenRation = m_goldenRatioPrecision;
	std::string label4 = "Golden Ratio Prec##IntersectionFinder";
	ImGui::DragFloat(label4.c_str(), &goldenRation, 0.05f, 0.05f, 2.f, "%.1e");

	float cgprec = m_CGprecision;
	std::string label5 = "CG prec##IntersectionFinder";
	ImGui::DragFloat(label5.c_str(), &cgprec, 0.05f, 0.05f, 2.f, "%.1e");

	float cursCGPrec = m_cursorCGprecision;
	std::string label6 = "Cursor CG##IntersectionFinder";
	ImGui::DragFloat(label6.c_str(), &cursCGPrec, 0.05f, 0.05f, 2.f, "%.1e");

	float loopPrec = m_loopPrecision;
	std::string label7 = "Loop precision##IntersectionFinder";
	ImGui::DragFloat(label7.c_str(), &loopPrec, 0.05f, 0.05f, 2.f, "%.1e");
	
	m_step = step;
	m_precision = eps;
	m_alphaPrecision = alphaprec;
	m_goldenRatioPrecision = goldenRation;
	m_CGprecision = cgprec;
	m_cursorCGprecision = cursCGPrec;
	m_loopPrecision = loopPrec;
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

DirectX::XMFLOAT2 CalculateGradientForCursor(
	IParametricSurface* qSurf, ParameterPair qParams, DirectX::XMFLOAT3 cursorPos)
{
	// Assumes function F(x) = <Q(u,v)-cursorPos, Q(u,v)-cursorPos>
	float u, v;
	u = qParams.u;
	v = qParams.v;

	auto Q = qSurf->GetPoint(u, v);
	auto Qdu = qSurf->GetTangent(u, v, TangentDir::AlongU);
	auto Qdv = qSurf->GetTangent(u, v, TangentDir::AlongV);

	float du = Dot(Qdu, 2 * (Q - cursorPos));
	float dv = Dot(Qdv, 2 * (Q - cursorPos));

	return DirectX::XMFLOAT2(du, dv);
}

float CalculateAlpha(
	IParametricSurface* qSurf, ParameterPair qParams, 
	IParametricSurface* pSurf, ParameterPair pParams, 
	DirectX::XMFLOAT4 p)
{
	DirectX::XMFLOAT4X4 Hf = CalculateHessian(qSurf, qParams, pSurf, pParams);
	DirectX::XMFLOAT4 gradient = CalculateGradient(qSurf, qParams, pSurf, pParams);
	float a = Mul(gradient, p);
	float b = Mul(Mul(p, Hf), p);
	float alpha = - a / b;
	return alpha;
}

bool ParamsOutOfBounds(float u, float v, float s, float t)
{
	bool val = !((u >= 0.f && u <= 1.f) && (v >= 0.f && v <= 1.f) && (s >= 0.f && s <= 1.f) && (t >= 0.f && t <= 1.f));
	return val;
}

bool ParamsOutOfBounds(float u, float v)
{
	bool val = !((u >= 0.f && u <= 1.f) && (v >= 0.f && v <= 1.f));
	return val;
}

float CalculateBeta(DirectX::XMFLOAT4 r_cur, DirectX::XMFLOAT4 r_prev)
{
	float beta;
	DirectX::XMFLOAT4 rDiff = r_cur - r_prev;
	beta = Mul(r_cur, rDiff) / Mul(r_prev, r_prev);
	return beta;
}

DirectX::XMFLOAT4 IntersectionFinder::CalculateOptimalPointInDirection(
	IParametricSurface* qSurf, ParameterPair qParams,
	IParametricSurface* pSurf, ParameterPair pParams,
	DirectX::XMFLOAT4 x_k,
	DirectX::XMFLOAT4 searchDir)
{
	auto originalX = x_k;
	float alpha = 0.f;
	alpha = CalculateAlpha(qSurf, qParams, pSurf, pParams, searchDir);
	int newtonIt = 0;
	DirectX::XMFLOAT4 step = alpha * searchDir;
	float stepLength = sqrt(Dot(step, step));
	
	qParams = ParameterPair{ x_k.x, x_k.y };
	pParams = ParameterPair{ x_k.z, x_k.w };
	// TODO do sth if alhpa is too big and hit the iteration limit
	while ((abs(stepLength) > m_alphaPrecision) && !ParamsOutOfBounds(qParams.u, qParams.v, pParams.u, pParams.v))
	{
		x_k = x_k + step;
		qParams = ParameterPair{ x_k.x, x_k.y };
		pParams = ParameterPair{ x_k.z, x_k.w };
		if (!ParamsOutOfBounds(qParams.u, qParams.v, pParams.u, pParams.v))
		{
			alpha = CalculateAlpha(qSurf, qParams, pSurf, pParams, searchDir);
			step = alpha * searchDir;
			stepLength = sqrt(Dot(step, step));
		}
		newtonIt++;
		if (newtonIt > 250)
		{
			break;
		}
	}

	return x_k - originalX;
}

float GetCurrentFuncValue(
	IParametricSurface* qSurf, ParameterPair qParams,
	IParametricSurface* pSurf, ParameterPair pParams)
{
	auto ptQ = qSurf->GetPoint(qParams.u, qParams.v);
	auto ptP = pSurf->GetPoint(pParams.u, pParams.v);
	return sqrtf(Dot(ptQ - ptP, ptQ - ptP));
}

float GetCurrentFuncValueForCursor(
	IParametricSurface* qSurf, ParameterPair qParams, DirectX::XMFLOAT3 cursorPos)
{
	auto ptQ = qSurf->GetPoint(qParams.u, qParams.v);	
	return sqrtf(Dot(ptQ - cursorPos, ptQ - cursorPos));
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
	// Calculate residual direction and conjugated move direction
	DirectX::XMFLOAT4 r_k = -1.f * CalculateGradient(qSurface, qSurfParams, pSurface, pSurfParams);
	DirectX::XMFLOAT4 r_zero = r_k;
	DirectX::XMFLOAT4 d_k = r_k;
	float dist = GetCurrentFuncValue(qSurface, qSurfParams, pSurface, pSurfParams);
	float curDist = dist;

	// TODO: Add condition so taht it is known if the method diverges
	int iterationCounter = 0;	
	bool restart = false;
	bool continueSearch = true;

	while (continueSearch)
	{
		ParameterPair prevQParams = ParameterPair{ x_k.x, x_k.y };
		ParameterPair prevPParams = ParameterPair{ x_k.z, x_k.w };		

		if (restart)
		{
			// Reset the method and treat the previous point as a x_0
			r_k = -1.f * CalculateGradient(qSurface, prevQParams, pSurface, prevPParams);
			d_k = r_k;
		}		

		// X_k = x_k-1 + alpha * d_k
		// Get x_k
		auto xDiff = CalculateOptimalPointInDirection(qSurface, prevQParams, pSurface, prevPParams, x_k, d_k);	
		x_k = x_k + xDiff;

		ParameterPair curQParams = ParameterPair{ x_k.x, x_k.y };
		ParameterPair curPParams = ParameterPair{ x_k.z, x_k.w };

		// Check if the parameters from next iteration are in bound
		if (ParamsOutOfBounds(curQParams.u, curQParams.v, curPParams.u, curPParams.v))
		{
			// Stop the algorithm
			continueSearch = false;
			found = false;		
			break;
		}

		// Inside bounds
		   // Get the updated parameters
		dist = curDist;
		curDist = GetCurrentFuncValue(qSurface, curQParams, pSurface, curPParams);

		// Store old residual
		auto r_prev = r_k;

		// Update Residual 
		r_k = -1.f * CalculateGradient(qSurface, curQParams, pSurface, curPParams);

		//CalculateBeta according to Polak-Ribiere
		float beta = CalculateBeta(r_k, r_prev);

		// Calculate new conjugated direction
		d_k = r_k + beta * d_k;


		restart = Dot(r_k, d_k) <= 0;

		// After 3 iterations reset the method
		iterationCounter++;
		if (iterationCounter > 30000)
		{
			found = false;
			continueSearch = false;
		}

		if (curDist >= dist)
		{
			continueSearch = false;
			found = false;
		}

		if (curDist <= m_CGprecision)
		{
			continueSearch = false;
			found = true;
			qSurfParams = curQParams;
			pSurfParams = curPParams;
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

	std::vector<DirectX::XMFLOAT2> qParamsList, pParamsList;

	std::vector<std::vector<DirectX::XMFLOAT3>> pts;
	std::vector<std::vector<DirectX::XMFLOAT3>> tangentsU;
	std::vector<std::vector<DirectX::XMFLOAT3>> tangentsV;

	/*auto surf = dynamic_cast<BezierSurfaceC0*>(surface1);
	if (surf != nullptr)
	{
		
		int i = 0;
		for (float u = 0.0f; u <= 1.0f; u += 0.1f)
		{
			pts.push_back(std::vector<DirectX::XMFLOAT3>());
			tangentsU.push_back(std::vector<DirectX::XMFLOAT3>());
			tangentsV.push_back(std::vector<DirectX::XMFLOAT3>());

			for (float v = 0.0f; v <= 1.0f; v += 0.1f)
			{
				pts[(int)i].push_back(surf->GetPoint(u, v));
				tangentsU[(int)i].push_back(surf->GetTangent(u, v, AlongU));
				tangentsV[(int)i].push_back(surf->GetTangent(u, v, AlongV));
			}
			i++;
		}
	}*/

	/*for (float u = 0.0f; u <= 1.0f; u += 0.1f)
	{
		qParamsList.push_back(XMFLOAT2(u, u));
		pParamsList.push_back(XMFLOAT2(u, u));
	}
	auto curve = m_factory->CreateIntersectionCurve(surface1, qParamsList, surface2, pParamsList);
	m_scene->AttachObject(curve);
	return;*/


	// Find All intersecting sections from surface 1 (Ps) and surface 2 (Qs) and Find intersection points for each combination (P,Q)
	// TODO: FIND ALL AFFECTED PATCHES
	// DetermineAffectedSurfaces(surface1, surface2, pSurfs, qSurfs);

	
	// Sample p and q in a lot of differnt places and search for Intersection Points

	// For each P, Q	
	ParameterPair pParams, qParams;
	qParams.v = qParams.u = 0.5f;
	pParams.v = pParams.u = 0.5f;
	DirectX::XMFLOAT3 firstPoint;

	for (float u = 0.0f; u <= 1.0f; u += 0.1f)
	{
		for (float v = 0.0f; v <= 1.0f; v += 0.1f)
		{
			for (float s = 0.0f; s <= 1.0f; s += 0.1f)
			{
				for (float t = 0.0f; t <= 1.0f; t += 0.1f)
				{
					qParams.u = u;
					qParams.v = v;
					pParams.u = s;
					pParams.v = t;

					// Calculate first point
					if (SimpleGradient(surface1, qParams, surface2, pParams, firstPoint))
					{					
						qParamsList.push_back(XMFLOAT2(qParams.u, qParams.v));
						pParamsList.push_back(XMFLOAT2(pParams.u, pParams.v));

						// Simple gradient seems to work fine 
						// TODO DOUBLE CHECK IF PARAMETERS ARE SET UP CORRECTLY
						auto ptQ = surface1->GetPoint(qParams.u, qParams.v);
						auto ptP = surface2->GetPoint(pParams.u, pParams.v);

						float eps = 0.01f;
						auto dist = Dot(ptQ - ptP, ptQ - ptP);
						assert(dist <= eps * eps);

						//// Calculate other points				
						/*FindOtherIntersectionPoints(
							q, qParams, qParamsList,
							p, pParams, pParamsList, firstPoint);*/

							// Create the interpolation curve
						auto curve = m_factory->CreateIntersectionCurve(surface1, qParamsList, surface2, pParamsList);
						m_scene->AttachObject(curve);
						return;
					}
				}
			}
		}
	}		
}

void IntersectionFinder::FindIntersectionWithCursor(
	IParametricSurface* surface1, 
	IParametricSurface* surface2, 
	DirectX::XMFLOAT3 cursorPos)
{

	std::vector<DirectX::XMFLOAT3> points;
	std::vector<DirectX::XMFLOAT2> qParamsList, pParamsList;
	ParameterPair pParams, qParams;
	DirectX::XMFLOAT3 foundPoint;

	bool found1 = false, found2 = false;
	float dist1 = 0, dist2 = 0;
	ParameterPair endParams1, endParams2;

	// Find closest point on one surface
	for (float u = 0.0f; u <= 1.0f; u += 0.1f)
	{
		for (float v = 0.0f; v <= 1.0f; v += 0.1f)
		{
			qParams.u = u;
			qParams.v = v;
			if (SimpleGradientForCursor(surface1, qParams, cursorPos, foundPoint)) {
				// If found point is closer than the previous point, update it
				auto newDist = sqrtf(Dot(foundPoint-cursorPos, foundPoint - cursorPos));
				if (newDist < dist1 || found1 == false)
				{
					found1 = true;
					endParams1 = qParams;
					dist1 = newDist;				
				}
				
			}
		}
	}

	// Find closest point on the second surface
	for (float u = 0.0f; u <= 1.0f; u += 0.1f)
	{
		for (float v = 0.0f; v <= 1.0f; v += 0.1f)
		{
			pParams.u = u;
			pParams.v = v;
			if (SimpleGradientForCursor(surface2, pParams, cursorPos, foundPoint))
			{
				// If found point is closer than the previous point, update it
				auto newDist = sqrtf(Dot(foundPoint - cursorPos, foundPoint - cursorPos));
				if (newDist < dist1 || found2 == false)
				{
					found2 = true;
					endParams2 = pParams;
					dist2 = newDist;
				}
			}
		}
	}

	if (found1 && found2)
	{
		// Calculate first point
		if (SimpleGradient(surface1, endParams1, surface2, endParams2, foundPoint))
		{
			qParamsList.push_back(XMFLOAT2(endParams1.u, endParams1.v));
			pParamsList.push_back(XMFLOAT2(endParams2.u, endParams2.v));

			// Calculate other points				
			FindOtherIntersectionPoints(
				surface1, endParams1, qParamsList,
				surface2, endParams2, pParamsList, foundPoint);

			// Create the interpolation curve
			auto curve = m_factory->CreateIntersectionCurve(surface1, qParamsList, surface2, pParamsList);
			m_scene->AttachObject(curve);
			return;
		}
		else {
			// TODO could not find and intersection
		}
	}	
}

void IntersectionFinder::FindOtherIntersectionPoints(
	IParametricSurface* surface1, ParameterPair surf1Params, std::vector<DirectX::XMFLOAT2>& surf1ParamsList,
	IParametricSurface* surface2, ParameterPair surf2Params, std::vector<DirectX::XMFLOAT2>& surf2ParamsList,
	DirectX::XMFLOAT3 firstPoint)
{

	auto params1For = surf1Params;
	auto params2For = surf2Params;
	auto params1Back = surf1Params;
	auto params2Back = surf2Params;


	std::vector<DirectX::XMFLOAT2> forwards1;
	std::vector<DirectX::XMFLOAT2> forwards2;
	std::vector<DirectX::XMFLOAT2> backwards1;
	std::vector<DirectX::XMFLOAT2> backwards2;

	bool looped = false;
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	bool nextPointInRange = FindNextPoint(surface1, params1For, surface2, params2For, firstPoint, position, false);
	while (nextPointInRange)
	{
		//Check if found point makes a loop
		auto diff = firstPoint - position;
		auto dist = Dot(diff, diff);

		// TODO: CHECK IF TRUE LOOPED
		// Check if this is a circle of infinity sign like interjection
		auto begParams = DirectX::XMFLOAT4(surf1Params.u, surf1Params.v, surf2Params.u, surf2Params.v);
		auto candidateParams = DirectX::XMFLOAT4(params1For.u, params1For.v, params2For.u, params2For.v);
		auto paramDiff = begParams - candidateParams;
		auto paramDiffLen = Dot(paramDiff, paramDiff);
		bool trueLoop =  paramDiffLen < m_loopPrecision * m_loopPrecision;

		// Watch out for cases when something that is not a true loop 
		// TODO Do a check after finding all the points to detect "trimmable loops"
		// Those should be connected based on distance in the scene, not parameter space
		// Maybe determine "loopness" for surfaces individually
		// Mark boundary looped straight line as looped if the position of boudnary + delta is nearly the same as the other end and distance in r^3 is less than a step
		// Do we even need to discern between true loops and boundary loops?

		if (dist <= m_step * m_step / 4.f && trueLoop) {
			// Add points and end the loop 
			// Add the point as usual
			forwards1.push_back(XMFLOAT2(surf1Params.u, surf1Params.v));
			forwards2.push_back(XMFLOAT2(surf2Params.u, surf2Params.v));
			// end search
			nextPointInRange = false;
			looped = true;	
			// TODO: return looped info and mark it in the intersection curve structure
		}
		else {
			// Add point normally (no loop or a ribbon with no connecting ends)
			// TODO: after all calculations check for loops based on distance?
			forwards1.push_back(XMFLOAT2(params1For.u, params1For.v));
			forwards2.push_back(XMFLOAT2(params2For.u, params2For.v));
			auto prevPos = position;
			nextPointInRange = FindNextPoint(surface1, params1For, surface2, params2For, prevPos, position, false);
		}		
	}
	if (!looped)
	{
		position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		nextPointInRange = FindNextPoint(surface1, params1Back, surface2, params2Back, firstPoint, position, true);
		while (nextPointInRange)
		{
			backwards1.push_back(XMFLOAT2(params1Back.u, params1Back.v));
			backwards2.push_back(XMFLOAT2(params2Back.u, params2Back.v));
			auto prevPos = position;
			nextPointInRange = FindNextPoint(surface1, params1Back, surface2, params2Back, prevPos, position, true);
			// DEBUG PURPOSES ONLY
			if (backwards1.size() > 2000)
				break;
		}
	}
	
	std::reverse(forwards1.begin(), forwards1.end());
	std::reverse(forwards2.begin(), forwards2.end());

	std::vector<DirectX::XMFLOAT2> res1;
	std::vector<DirectX::XMFLOAT2> res2;

	for (int i = 0; i < forwards1.size(); i++)
	{
		res1.push_back(forwards1[i]);
		res2.push_back(forwards2[i]);
	}

	res1.push_back(surf1ParamsList[0]);
	res2.push_back(surf2ParamsList[0]);

	for (int i = 0; i < backwards1.size(); i++)
	{
		res1.push_back(backwards1[i]);
		res2.push_back(backwards2[i]);
	}

	surf1ParamsList = res1;
	surf2ParamsList = res2;
	return;
}

DirectX::XMFLOAT4 FindClampedPosition(DirectX::XMFLOAT4 x_k, DirectX::XMFLOAT4 x_prev, float step)
{
	// Given a point x_prev inside a 4 dimensional cube [0,1]x[0,1]x[0,1]x[0,1] and the next step x_k
	// Find a point on the boundary of the cube that lies on the line intersecting x_k and x_prev

	auto delta = x_k - x_prev;
	bool isZeroDist = false;
	float dist = 20.f;
	float eps = 10E-5;
	for (int i = 0; i < 4; i++)
	{
		float deltaCoord = GetNthFieldValue(delta, i);
		if (deltaCoord != 0.0f)
		{
			float zeroDist = (0.f - GetNthFieldValue(x_prev, i)) / deltaCoord;
			float oneDist = (1.f - GetNthFieldValue(x_prev, i)) / deltaCoord;

			float val = 2.0f;
			float isValZeroDist = false;

			if (zeroDist > 0.0f)
			{
				val = zeroDist;
				isValZeroDist = true;
			}

			if (oneDist > 0.0f && oneDist <val )
			{
				val = oneDist;
				isValZeroDist = false;
			}
			
			if (val < dist)
			{
				dist = val;
				isZeroDist = isValZeroDist;
			}
		}
	}

	auto normDelta = delta / sqrt(Dot(delta, delta));

	auto res = x_prev + delta * dist;

	for (int i = 0; i < 4; i++)
	{
		float val = GetNthFieldValue(delta, i);
		if (val < eps)
			val = 0.0f;
		if (val > 1.f - eps)
			val = 1.0f;
		SetNthFieldValue(delta, i, val);
	}
	
	auto modifiedStep = delta * dist;

	if (Dot(modifiedStep, modifiedStep) > step * step)
	{
		res = x_k;
	}

	return res;
}

bool IntersectionFinder::FindNextPoint(
	IParametricSurface* qSurf, ParameterPair& qSurfParams,
	IParametricSurface* pSurf, ParameterPair& pSurfParams, 
	DirectX::XMFLOAT3 prevPoint,
	DirectX::XMFLOAT3& pos,
	bool reverseDirection)
{	
	DirectX::XMFLOAT4 x_k, x_prev;
	bool found = false;
	
	// Initialize x_0
	x_k.x = qSurfParams.u; // u
	x_k.y = qSurfParams.v; // v
	x_k.z = pSurfParams.u; // s
	x_k.w = pSurfParams.v; // t	
	x_prev = x_k;

	ParameterPair curQParams = ParameterPair{ x_k.x, x_k.y };
	ParameterPair curPParams = ParameterPair{ x_k.z, x_k.w };

	// TODO: Add condition so taht it is known if the method diverges
	int iterationCounter = 0;

	bool continueNewtonCalculation = true;
	while (continueNewtonCalculation)
	{		
		// Calculate Step direction
		auto stepVersor = CalculateStepDirection(qSurf, curQParams, pSurf, curPParams);
		if (reverseDirection)
		{
			stepVersor = -1 * stepVersor;
		}
		
		// Calculate F(x)		
		auto funcVal = CalculateIntersectionDistanceFunctionValue( //Why the minus?
			qSurf, curQParams, 
			pSurf, curPParams, 
			prevPoint, stepVersor, m_step);

		auto val = Dot(funcVal, funcVal);
		if (val <= m_precision * m_precision) 
		{ // Solution is satisfying
			continueNewtonCalculation = false;
			found = true;
			pos = pSurf->GetPoint(curPParams.u, curPParams.v);

			// Update structures to hold current parameters data for use outside this func
			qSurfParams = curQParams;
			pSurfParams = curPParams;
		} 
		else 
		{ 
			// Calculate next iteration
			// Create a linear equation system and solve it
			DirectX::XMFLOAT4X4 derMatrix = CalculateDerivativeMatrix(qSurf, pSurf, x_k, stepVersor);

			//auto deltaX = Geom::SolveGEPP(derMatrix, funcVal);
			auto deltaXGetp = Geom::SolveGEPP(derMatrix, -1 * funcVal);
			// Find the next point using Newton's method to solve linear equation system
			// There should not be a minus here, for some reason there is. Check someday. 
			// This works.
			x_prev = x_k;
			x_k = x_k + deltaXGetp; // Why the minus?

			curQParams = ParameterPair{ x_k.x, x_k.y };
			curPParams = ParameterPair{ x_k.z, x_k.w };
			if (ParamsOutOfBounds(curQParams.u, curQParams.v, curPParams.u, curPParams.v))
			{
				// Check if the distance between clamped params and prev point is smaller than step 
				//if yes - connect the curve to the patch boundary and flag this end as looped

				auto boundaryPoint = FindClampedPosition(x_k, x_prev, m_step);
				if (ParamsOutOfBounds(boundaryPoint.x, boundaryPoint.y, boundaryPoint.z, boundaryPoint.w))
				{
					// Stop the algorithm
					continueNewtonCalculation = false;
					found = false;
					break;
				}
				else {
					// Solution is satisfying
					continueNewtonCalculation = false;
					found = true;
					pos = pSurf->GetPoint(boundaryPoint.z, boundaryPoint.w);

					// Update structures to hold current parameters data for use outside this func
					qSurfParams = { boundaryPoint.x, boundaryPoint.y};
					pSurfParams = { boundaryPoint.z, boundaryPoint.w};
				}		
			}
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

float CalcFunc(
	IParametricSurface* qSurface,
	IParametricSurface* pSurface,
	DirectX::XMFLOAT4 params,
	DirectX::XMFLOAT4 dir,
	float alpha)
{
	// Caclulcate < Q - P, Q - P >

	auto movedParams = params + dir * alpha;

	auto q = qSurface->GetPoint(movedParams.x, movedParams.y);
	auto p = pSurface->GetPoint(movedParams.z, movedParams.w);

	return Dot(q - p, q - p);
}


float CalcFuncForCursor(
	IParametricSurface* qSurface,
	DirectX::XMFLOAT2 params,
	DirectX::XMFLOAT2 dir,
	DirectX::XMFLOAT3 cursorPos,
	float alpha)
{
	// Caclulcate < Q - P, Q - P >
	auto movedParams = params + dir * alpha;
	auto q = qSurface->GetPoint(movedParams.x, movedParams.y);
	return Dot(q - cursorPos, q - cursorPos);
}

float IntersectionFinder::GoldenRatioMethod(
	IParametricSurface* qSurface,
	IParametricSurface* pSurface, 
	float a, float b, 
	DirectX::XMFLOAT4 x_k, 
	DirectX::XMFLOAT4 d_k)
{
	float eps = m_goldenRatioPrecision;

	// wsp�czynnik z�otego podzia�u
	float k = (sqrt(5.f) - 1.f) / 2.f;

	auto qDir = DirectX::XMFLOAT2(d_k.x, d_k.y);
	auto pDir = DirectX::XMFLOAT2(d_k.z, d_k.w);

	float xLq = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, qDir, b);
	float xLp = pSurface->GetFarthestPointInDirection(x_k.z, x_k.w, pDir, b);
	b = abs(xLq) > abs(xLp) ? xLp : xLq;

	float xRq = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, qDir, a);
	float xRp = pSurface->GetFarthestPointInDirection(x_k.z, x_k.w, pDir, a);
	a = abs(xRq) > abs(xRp) ? xRp : xRq;

	// lewa i prawa pr�bka
	float xL = b - k * (b - a);
	float xR = a + k * (b - a);

	// p�tla p�ki nie zostanie spe�niony warunek stopu
	while ((b - a) > eps)
	{
		// if x_k + d_k * xL is out of bounds
		// update bounds and update x_l and calculate fL		

		// TODO if the smallest value is zero then no move can be made 		

		auto fL = CalcFunc(qSurface, pSurface, x_k, d_k, xL);
		auto fR = CalcFunc(qSurface, pSurface, x_k, d_k, xR);
		// por�wnaj warto�ci funkcji celu lewej i prawej pr�bki
		if (fL < fR)
		{
			// wybierz przedzia� [a, xR]
			b = xR;
			xR = xL;
			xL = b - k * (b - a);
		}
		else
		{
			// wybierz przedzia� [xL, b]
			a = xL;
			xL = xR;
			xR = a + k * (b - a);
		}
	}

	// zwr�� warto�� �rodkow� przedzia�u
	return (a + b) / 2.f;
}

float IntersectionFinder::GoldenRatioMethodForCursor(
	IParametricSurface* qSurface,
	float a, float b,
	DirectX::XMFLOAT2 x_k,
	DirectX::XMFLOAT2 d_k,
	DirectX::XMFLOAT3 cursorPos)
{
	float eps = m_alphaPrecision;

	// wsp�czynnik z�otego podzia�u
	float k = (sqrt(5) - 1) / 2;

	b = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, d_k, b);
	a = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, d_k, a);

	// lewa i prawa pr�bka
	float xL = b - k * (b - a);
	float xR = a + k * (b - a);

	// p�tla p�ki nie zostanie spe�niony warunek stopu
	while ((b - a) > eps)
	{

		// TODO if the smallest value is zero then no move can be made 
		if (xL * xR == 0.0f)
		{
			break;
		}

		auto fL = CalcFuncForCursor(qSurface, x_k, d_k, cursorPos, xL);
		auto fR = CalcFuncForCursor(qSurface, x_k, d_k, cursorPos, xR);
		// por�wnaj warto�ci funkcji celu lewej i prawej pr�bki
		if (fL < fR)
		{
			// wybierz przedzia� [a, xR]
			b = xR;
			xR = xL;
			xL = b - k * (b - a);
		}
		else
		{
			// wybierz przedzia� [xL, b]
			a = xL;
			xL = xR;
			xR = a + k * (b - a);
		}
	}

	// zwr�� warto�� �rodkow� przedzia�u
	return (a + b) / 2;
}

bool IntersectionFinder::SimpleGradient(
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
	// Calculate residual direction and conjugated move direction	
	DirectX::XMFLOAT4 dir_k = -1.f * CalculateGradient(qSurface, qSurfParams, pSurface, pSurfParams);
	float dist = GetCurrentFuncValue(qSurface, qSurfParams, pSurface, pSurfParams);
	float curDist = dist;

	// TODO: Add condition so taht it is known if the method diverges
	int iterationCounter = 0;
	bool restart = false;
	bool continueSearch = true;

	while (continueSearch)
	{
		// X_k = x_k-1 + alpha * d_k
		// Get x_k
		float alpha = GoldenRatioMethod(qSurface, pSurface, -0.5f, 0.5f, x_k, dir_k);
		x_k = x_k + alpha * dir_k;

		ParameterPair curQParams = ParameterPair{ x_k.x, x_k.y };
		ParameterPair curPParams = ParameterPair{ x_k.z, x_k.w };

		// Check if the parameters from next iteration are in bound
		if (!qSurface->ParamsInsideBounds(curQParams.u, curQParams.v) ||
			!pSurface->ParamsInsideBounds(curPParams.u, curPParams.v))
		{
			// Stop the algorithm
			continueSearch = false;
			found = false;
			break;
		}

		// Inside bounds
		   // Get the updated parameters
		dist = curDist;
		curDist = GetCurrentFuncValue(qSurface, curQParams, pSurface, curPParams);

		// Update Residual 
		dir_k = -1.f * CalculateGradient(qSurface, curQParams, pSurface, curPParams);		

		// After 3 iterations reset the method
		iterationCounter++;
		if (iterationCounter > 100)
		{
			found = false;
			continueSearch = false;
		}

		if (curDist >= dist)
		{
			continueSearch = false;
			found = false;
		}

		if (curDist <= m_CGprecision)
		{
			continueSearch = false;
			found = true;
			qSurfParams = curQParams;
			pSurfParams = curPParams;
			point = qSurface->GetPoint(qSurfParams.u, qSurfParams.v);
		}
	}

	return found;
}

bool IntersectionFinder::SimpleGradientForCursor(IParametricSurface* qSurface, ParameterPair& qSurfParams, DirectX::XMFLOAT3 cursorPos, DirectX::XMFLOAT3& point)
{
	bool found = false;
	DirectX::XMFLOAT2 x_k, x_prev; // x = [ u, v, s, t ]

	// Initialize x_0
	x_k.x = qSurfParams.u; // u 
	x_k.y = qSurfParams.v; // v	
	x_prev = x_k;
	// Calculate residual direction and conjugated move direction	
	DirectX::XMFLOAT2 d_k = -1.f * CalculateGradientForCursor(qSurface, qSurfParams, cursorPos);
	float dist = GetCurrentFuncValueForCursor(qSurface, qSurfParams, cursorPos);
	float curDist = dist;

	// TODO: Add condition so taht it is known if the method diverges
	int iterationCounter = 0;
	bool restart = false;
	bool continueSearch = true;

	while (continueSearch)
	{
		// X_k = x_k-1 + alpha * d_k
		// Get x_k
		float alpha = GoldenRatioMethodForCursor(qSurface, -0.5f, 0.5f, x_k, d_k, cursorPos);
		x_prev = x_k;
		x_k = x_k + alpha * d_k;

		ParameterPair curQParams = ParameterPair{ x_k.x, x_k.y };

		// Check if the parameters from next iteration are in bound
		if (!qSurface->ParamsInsideBounds(curQParams.u, curQParams.v))		
		{
			// Stop the algorithm
			continueSearch = false;
			found = false;
			break;
		}

		// Inside bounds
		   // Get the updated parameters
		dist = curDist;
		curDist = GetCurrentFuncValueForCursor(qSurface, curQParams, cursorPos);

		// Update Residual 
		d_k = -1.f * CalculateGradientForCursor(qSurface, curQParams, cursorPos);


		iterationCounter++;
		if (iterationCounter > 10)
		{
			continueSearch = false;
			found = true;
			qSurfParams = curQParams;
			point = qSurface->GetPoint(qSurfParams.u, qSurfParams.v);
		}

		if (curDist > dist)
		{
			continueSearch = false;
			found = true;
			qSurfParams = ParameterPair{x_prev.x, x_prev.y};
			point = qSurface->GetPoint(qSurfParams.u, qSurfParams.v);
		}

		if (curDist <= m_cursorCGprecision)
		{
			continueSearch = false;
			found = true;
			qSurfParams = curQParams;
			point = qSurface->GetPoint(qSurfParams.u, qSurfParams.v);
		}
	}

	return found;
}
