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
	m_step = 2.75f; 
	m_precision = 0.01f;
	m_alphaPrecision = 0.0001f;
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

	float col1Last = Dot(col1, stepDir);
	float col2Last = Dot(col2, stepDir);

	derivatives.m[0][0] = col1.x;
	derivatives.m[1][0] = col1.y;
	derivatives.m[2][0] = col1.z;
	derivatives.m[3][0] = col1Last;

	derivatives.m[0][1] = col2.x;
	derivatives.m[1][1] = col2.y;
	derivatives.m[2][1] = col2.z;
	derivatives.m[3][1] = col2Last;

	derivatives.m[0][2] = col3.x;
	derivatives.m[1][2] = col3.y;
	derivatives.m[2][2] = col3.z;
	derivatives.m[3][2] = 0.0f;

	derivatives.m[0][3] = col4.x;
	derivatives.m[1][3] = col4.y;
	derivatives.m[2][3] = col4.z;
	derivatives.m[3][3] = 0.0f;

	return derivatives;
}

DirectX::XMFLOAT4 IntersectionFinder::CalculateIntersectionDistanceFunctionValue(
	IParametricSurface* qSurface, ParameterPair& qSurfParams, 
	IParametricSurface* pSurface, ParameterPair& pSurfParams, 
	DirectX::XMFLOAT3 prevPoint, DirectX::XMFLOAT3 stepDir, float step)
{
	// [Q - P, <P - prev, t> - d ]
	DirectX::XMFLOAT4 funcVal;

	DirectX::XMFLOAT3 pt1 = qSurface->GetPoint(qSurfParams.u, qSurfParams.v);
	DirectX::XMFLOAT3 pt2 = pSurface->GetPoint(pSurfParams.u, pSurfParams.v);
	DirectX::XMFLOAT3 posDiff = pt1 - pt2;

	auto stepDiff = pt2 - prevPoint;
	float w = Dot(stepDiff, -1 * stepDir) - step;

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

		if (curDist <= m_precision)
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
							if (SimpleGradient(q, qParams, p, pParams, firstPoint))
							{
								qParamsList.push_back(XMFLOAT2(qParams.u, qParams.v));
								pParamsList.push_back(XMFLOAT2(pParams.u, pParams.v));

								// Calculate other points				
								FindOtherIntersectionPoints(
									q, qParams, qParamsList,
									p, pParams, pParamsList, firstPoint);

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
	}
}

void IntersectionFinder::FindOtherIntersectionPoints(
	IParametricSurface* surface1, ParameterPair surf1Params, std::vector<DirectX::XMFLOAT2>& surf1ParamsList,
	IParametricSurface* surface2, ParameterPair surf2Params, std::vector<DirectX::XMFLOAT2>& surf2ParamsList,
	DirectX::XMFLOAT3 firstPoint)
{
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	bool nextPointInRange = FindNextPoint(surface1, surf1Params, surface2, surf2Params, firstPoint, position);
	while(nextPointInRange)
	{	
		surf1ParamsList.push_back(XMFLOAT2(surf1Params.u, surf1Params.v));
		surf2ParamsList.push_back(XMFLOAT2(surf2Params.u, surf2Params.v));
		
		// Search for the next point
		// prevPos is -1000000
		auto prevPos = position;
		nextPointInRange = FindNextPoint(surface1, surf1Params, surface2, surf2Params, prevPos, position);		
	}
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
	auto stepVersor = -1 * CalculateStepDirection(qSurf, qSurfParams, pSurf, pSurfParams);
	
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
		if (ParamsOutOfBounds(curQParams.u, curQParams.v, curPParams.u, curPParams.v))
		{
			// Stop the algorithm
			continueNewtonCalculation = false;
			found = false;
			break;
		}
		auto funcVal = -1.f * CalculateIntersectionDistanceFunctionValue(
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

float GoldenRatioMethod(
	IParametricSurface* qSurface,
	IParametricSurface* pSurface, 
	float a, float b, 
	DirectX::XMFLOAT4 x_k, 
	DirectX::XMFLOAT4 d_k)
{
	float eps = 0.0001f;

	// wspó³czynnik z³otego podzia³u
	float k = (sqrt(5) - 1) / 2;

	// lewa i prawa próbka
	float xL = b - k * (b - a);
	float xR = a + k * (b - a);

	// pêtla póki nie zostanie spe³niony warunek stopu
	while ((b - a) > eps)
	{
		auto fL = CalcFunc(qSurface, pSurface, x_k, d_k, xL);
		auto fR = CalcFunc(qSurface, pSurface, x_k, d_k, xR);
		// porównaj wartoœci funkcji celu lewej i prawej próbki
		if (fL < fR)
		{
			// wybierz przedzia³ [a, xR]
			b = xR;
			xR = xL;
			xL = b - k * (b - a);
		}
		else
		{
			// wybierz przedzia³ [xL, b]
			a = xL;
			xL = xR;
			xR = a + k * (b - a);
		}
	}

	// zwróæ wartoœæ œrodkow¹ przedzia³u
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
	DirectX::XMFLOAT4 d_k = -1.f * CalculateGradient(qSurface, qSurfParams, pSurface, pSurfParams);
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
		float alpha = GoldenRatioMethod(qSurface, pSurface, -0.5f, 0.5f, x_k, d_k);
		x_k = x_k + alpha * d_k;

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

		// Update Residual 
		d_k = -1.f * CalculateGradient(qSurface, curQParams, pSurface, curPParams);		

		// After 3 iterations reset the method
		iterationCounter++;
		if (iterationCounter > 30)
		{
			found = false;
			continueSearch = false;
		}

		if (curDist >= dist)
		{
			continueSearch = false;
			found = false;
		}

		if (curDist <= m_precision)
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
