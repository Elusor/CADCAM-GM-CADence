#pragma once
#include <DirectXMath.h>
#include <vector>
#include "IntersectionFinder.h"
#include "mathUtils.h"
#include "ObjectFactory.h"
#include "Node.h"
#include "GeometricFunctions.h"
#include "Scene.h"
#include "ObjectReferences.h"
#include "IntersectionSearchResult.h"
#include "IntersectionSearchResultOneDir.h"
#include <queue>

ClampedPointData FindClampedPosition(ParameterQuad x_kQuad, ParameterQuad x_prevQuad, float step, bool xkInQ, bool xkInP);

IntersectionFinder::IntersectionFinder(Scene* scene)
{
	m_scene = scene;
	m_factory = scene->m_objectFactory.get();
	m_step = 0.5f; 
	m_loopPrecision = 0.029;
	m_precision = 10E-5f;
	// To allow cursor found soulutions to be found in gradient
	m_gradientPrecision = 1E-4f;
	m_cursorCGprecision = 1E-5f;
	m_alphaPrecision = 10E-7f;
	m_goldenRatioPrecision = 10E-5f;
	m_iterationCounter = 100;

	m_samples = 5.0f;
	m_cursorSamples = 10.f;
}

bool IntersectionFinder::AreObjectIntersectable(std::weak_ptr<Node> obj1, std::weak_ptr<Node> obj2)
{
	IParametricSurface* surf1 = dynamic_cast<IParametricSurface*>(obj1.lock()->m_object.get());
	IParametricSurface* surf2 = dynamic_cast<IParametricSurface*>(obj2.lock()->m_object.get());

	// Check if surf1 and surf2 are child and parent
	auto p1Str = obj1.lock();
	auto p2Str = obj2.lock();

	// Check if p2 has p1Str
	// Check if p1 has p2Str
	auto p1Children = p1Str->GetChildren();
	auto p2Children = p2Str->GetChildren();

	bool isP1Child = false, isP2Child = false;
	auto p1It = p1Children.begin();
	while (p1It != p1Children.end())
	{
		auto tmp = p1It->lock();
		isP1Child |= obj2.lock() == tmp;
		p1It++;
	}
	 
	auto p2It = p2Children.begin();
	while (p2It != p2Children.end())
	{
		auto tmp = p2It->lock();
		isP2Child |= obj1.lock() == tmp;
		p2It++;
	}

	bool areParamericSurfaces = surf1 != nullptr && surf2 != nullptr;
	bool areNotFamily = (isP1Child == false && isP2Child == false);
		

	return areParamericSurfaces && areNotFamily;
}

DirectX::XMFLOAT4X4 IntersectionFinder::CalculateDerivativeMatrix(
	IParametricSurface* surface1, IParametricSurface* surface2,
	ParameterQuad x_k, DirectX::XMFLOAT3 stepDir)
{
	DirectX::XMFLOAT4X4 derivatives;
	
	auto QdU = surface1->GetTangent(x_k.GetQParams(), TangentDir::AlongU);
	auto QdV = surface1->GetTangent(x_k.GetQParams(), TangentDir::AlongV);
	auto PdS = surface2->GetTangent(x_k.GetPParams(), TangentDir::AlongU);
	auto PdT = surface2->GetTangent(x_k.GetPParams(), TangentDir::AlongV);

	float col1Last = Dot(QdU, stepDir);
	float col2Last = Dot(QdV, stepDir);

	derivatives(0,0) = QdU.x;
	derivatives(1,0) = QdU.y;
	derivatives(2,0) = QdU.z;
	derivatives(3,0) = col1Last;
				
	derivatives(0,1) = QdV.x;
	derivatives(1,1) = QdV.y;
	derivatives(2,1) = QdV.z;
	derivatives(3,1) = col2Last;
				
	derivatives(0,2) = -PdS.x;
	derivatives(1,2) = -PdS.y;
	derivatives(2,2) = -PdS.z;
	derivatives(3,2) = 0.0f;
				
	derivatives(0,3) = -PdT.x;
	derivatives(1,3) = -PdT.y;
	derivatives(2,3) = -PdT.z;
	derivatives(3,3) = 0.0f;

	return derivatives;
}

DirectX::XMFLOAT4 IntersectionFinder::CalculateIntersectionDistanceFunctionValue(
	IParametricSurface* qSurface,
	IParametricSurface* pSurface,
	ParameterQuad parameters,
	DirectX::XMFLOAT3 prevPoint, 
	DirectX::XMFLOAT3 stepDir, 
	float stepDist)
{
	// [Q - P, <Q - prev, t> - d ]
	DirectX::XMFLOAT4 funcVal;

	DirectX::XMFLOAT3 Qpt = qSurface->GetPoint(parameters.GetQParams());
	DirectX::XMFLOAT3 Ppt = pSurface->GetPoint(parameters.GetPParams());
	
	auto len = Dot(stepDir, stepDir);
	assert(len <= 1.1f);

	DirectX::XMFLOAT3 posDiff = Qpt - Ppt;

	auto stepDiff = Qpt - prevPoint;
	float w = Dot(stepDiff, stepDir) - stepDist;

	funcVal.x = posDiff.x;
	funcVal.y = posDiff.y;
	funcVal.z = posDiff.z;
	funcVal.w = w;

	return funcVal;
}

DirectX::XMFLOAT3 IntersectionFinder::CalculateStepDirection(
	IParametricSurface* qSurf,  
	IParametricSurface* pSurf, 
	ParameterQuad params,
	bool reverseDirection)
{
	auto n1 = GetSurfaceNormal(qSurf, params.GetQParams());
	auto n2 = GetSurfaceNormal(pSurf, params.GetPParams());
	auto stepDir = Cross(n2, n1);
	float stepDirLen = sqrt(Dot(stepDir, stepDir));
	auto stepVersor = stepDir / stepDirLen;

	if (reverseDirection)
	{
		stepVersor = -1.f * stepVersor;
	}

	return stepVersor;
}

IntersectionPointSearchData IntersectionFinder::FindBoundaryPoint(
	ParameterQuad outParams, 
	ParameterQuad inParams, 
	IParametricSurface* qSurface,
	IParametricSurface* pSurface, 
	float step)
{
	
	IntersectionPointSearchData result;
	result.params = outParams;

	bool initialQIn = qSurface->ParamsInsideBounds(outParams.GetQParams());
	bool initialPIn = pSurface->ParamsInsideBounds(outParams.GetPParams());	

	auto boundaryPointData = FindClampedPosition(outParams, inParams, step, initialQIn, initialPIn);
	ParameterQuad boundaryParams = ParameterQuad(boundaryPointData.params);


	if (qSurface->ParamsInsideBounds(boundaryParams.GetQParams()) &&
		pSurface->ParamsInsideBounds(boundaryParams.GetPParams()))			
	{
		//TODO return from here an information that a point has intersected a border
		
		if (initialPIn && !initialQIn)
		{
			result.qPointOnBorder = true;
		}
		
		if (!initialPIn && initialQIn)
		{
			result.pPointOnBorder = true;
		}

		if (!initialPIn && !initialQIn)
		{			
			auto distances = boundaryPointData.distances;
			auto minQStep = min(abs(distances.x), abs(distances.y));
			auto minPStep = min(abs(distances.z), abs(distances.w));

			if (minQStep > minPStep)
			{
				result.pPointOnBorder = true;
			}
			else if(minQStep < minPStep)
			{
				result.qPointOnBorder = true;
			}
			else {
				result.pPointOnBorder = true;
				result.qPointOnBorder = true;
			}

		}

		result.found = true;
		result.pos = pSurface->GetPoint(boundaryParams.GetPParams());
		result.params = boundaryParams;
	}

	return result;
}

void IntersectionFinder::CreateParamsGui()
{
	ImGui::Text("Intersection options");

	float eps = m_precision;
	std::string label = "Newton Precision##IntersectionFinder";
	ImGui::DragFloat(label.c_str(), &eps, 0.05f, 0.05f, 2.f, "%.1e");

	float step = m_step;
	std::string label2 = "Step size##IntersectionFinder";
	ImGui::DragFloat(label2.c_str(), &step, 0.05f, 0.05f, 2.f, "%.1e");

	float loopPrec = m_loopPrecision;
	std::string label7 = "Loop detection precision##IntersectionFinder";
	ImGui::DragFloat(label7.c_str(), &loopPrec, 0.05f, 0.05f, 2.f, "%.1e");

	int iterations = m_iterationCounter;
	std::string label8 = "Newton Iterations ##IterationsCounter";
	ImGui::DragInt(label8.c_str(), &iterations, 0, 1, 5);

	float goldenRation = m_goldenRatioPrecision;
	std::string label4 = "Golden Ratio Prec##IntersectionFinder";
	ImGui::DragFloat(label4.c_str(), &goldenRation, 0.05f, 0.05f, 2.f, "%.1e");

	ImGui::Spacing();	

	ImGui::Text("Default method");	
	float cgprec = m_gradientPrecision;
	std::string label5 = "Gradient method Precision##IntersectionFinder";
	ImGui::DragFloat(label5.c_str(), &cgprec, 0.05f, 0.05f, 2.f, "%.1e");

	int samp = m_samples;
	std::string label9 = "Domain samples##IterationsCounter";
	ImGui::DragInt(label9.c_str(), &samp, 0, 1, 5);

	ImGui::Spacing();
	ImGui::Text("Cursor approximation");

	float cursCGPrec = m_cursorCGprecision;
	std::string label6 = "Cursor Gradient method precision ##IntersectionFinder";
	ImGui::DragFloat(label6.c_str(), &cursCGPrec, 0.05f, 0.05f, 2.f, "%.1e");

	int CGsamp = m_cursorSamples;
	std::string label10 = "Cursor domain samples##IterationsCounter";
	ImGui::DragInt(label10.c_str(), &CGsamp, 0, 1, 5);	

	m_samples = samp;
	m_cursorSamples = CGsamp;
	m_iterationCounter = iterations;
	m_step = step;
	m_precision = eps;
	m_goldenRatioPrecision = goldenRation;
	m_gradientPrecision = cgprec;
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
	auto ptQ = qSurf->GetPoint(qParams);
	auto ptP = pSurf->GetPoint(pParams);
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

		if (curDist <= m_gradientPrecision)
		{
			continueSearch = false;
			found = true;
			qSurfParams = curQParams;
			pSurfParams = curPParams;
		}	
	}	

	return found;
}

void IntersectionFinder::FindInterSection(ObjectRef qSurfNode, ObjectRef pSurfNode)
{
	IParametricSurface* qSurface = dynamic_cast<IParametricSurface*>(qSurfNode.lock()->m_object.get());
	IParametricSurface* pSurface = dynamic_cast<IParametricSurface*>(pSurfNode.lock()->m_object.get());

	std::vector<DirectX::XMFLOAT2> qParamsList, pParamsList;
	ParameterPair pParams, qParams;

	ParameterPair paramsQ = qSurface->GetMaxParameterValues();
	ParameterPair paramsP = pSurface->GetMaxParameterValues();

	float stepCount = m_samples;
	float maxU, maxV, maxS, maxT;
	float uStep, vStep, sStep, tStep;

	maxU = paramsQ.u;
	maxV = paramsQ.v;
	maxS = paramsP.u;
	maxT = paramsP.v;

	uStep = maxU / stepCount;
	vStep = maxV / stepCount;
	sStep = maxS / stepCount;
	tStep = maxT / stepCount;

	for (float u = 0.0f; u <= maxU; u += uStep)
	{
		for (float v = 0.0f; v <= maxV; v += vStep)
		{
			for (float s = 0.0f; s <= maxS; s += sStep)
			{
				for (float t = 0.0f; t <= maxT; t += tStep)
				{					
					qParams.u = u;
					qParams.v = v;
					pParams.u = s;
					pParams.v = t;

					auto firstPt = SimpleGradient(qSurface, qParams, pSurface, pParams);
					// Calculate first point
					if (firstPt.found)
					{				
						// Update the search data
						auto foundPosition = firstPt.pos;
						auto foundParamsQ = firstPt.params.GetQParams();
						auto foundParamsP = firstPt.params.GetPParams();

						// Validity check
						float eps = 0.01f;
						auto ptQ = qSurface->GetPoint(foundParamsQ);
						auto ptP = pSurface->GetPoint(foundParamsP);
						auto dist = Dot(ptQ - ptP, ptQ - ptP);
						assert(dist <= eps * eps);						

						// Calculate other points				
						auto result = FindOtherIntersectionPoints(
							qSurface, foundParamsQ,
							pSurface, foundParamsP, foundPosition);

						// Create the interpolation curve
						auto curve = m_factory->CreateIntersectionCurve(
							qSurfNode, result.surfQParamsList, result.m_qIntersectionClosed,
							pSurfNode, result.surfPParamsList, result.m_pIntersectionClosed);

						m_scene->AttachObject(curve);
						return;
					}
				}
			}
		}
	}		
}

void IntersectionFinder::FindIntersectionWithCursor(
	ObjectRef qNode, 
	ObjectRef pNode,
	DirectX::XMFLOAT3 cursorPos)
{
	IParametricSurface* qSurface = dynamic_cast<IParametricSurface*>(qNode.lock()->m_object.get());
	IParametricSurface* pSurface = dynamic_cast<IParametricSurface*>(pNode.lock()->m_object.get());

	ParameterPair pParams, qParams;
	std::vector<DirectX::XMFLOAT2> qParamsList, pParamsList;	

	bool found1 = false, found2 = false;
	float dist1 = 0, dist2 = 0;
	ParameterPair endParams1, endParams2;

	float steps = m_cursorSamples;

	auto qBoundaries = qSurface->GetMaxParameterValues();
	float qMaxU = qBoundaries.u;
	float qMaxV = qBoundaries.v;

	auto pBoundaries = pSurface->GetMaxParameterValues();
	float pMaxU = pBoundaries.u;
	float pMaxV = pBoundaries.v;

	// Find closest point on one surface
	for (float u = 0.001f; u <= qMaxU; u += qMaxU / steps)
	{
		for (float v = 0.001f; v <= qMaxV; v += qMaxV / steps)
		{
			qParams.u = u;
			qParams.v = v;

			auto nearestPt = SimpleGradientForCursor(qSurface, qParams, cursorPos);
			if (nearestPt.found) {
				DirectX::XMFLOAT3 foundPoint = nearestPt.pos;
				// If found point is closer than the previous point, update it
				auto newDist = sqrtf(Dot(foundPoint - cursorPos, foundPoint - cursorPos));
				if (newDist < dist1 || found1 == false)
				{
					found1 = true;
					endParams1 = nearestPt.params.GetQParams();
					dist1 = newDist;				
				}
				
			}
		}
	}

	// Find closest point on the second surface
	for (float u = 0.0f; u <= pMaxU; u += pMaxU / steps)
	{
		for (float v = 0.0f; v <= pMaxV; v += pMaxV / steps)
		{
			pParams.u = u;
			pParams.v = v;

			auto nearestPt = SimpleGradientForCursor(pSurface, pParams, cursorPos);
			if (nearestPt.found)
			{
				DirectX::XMFLOAT3 foundPoint = nearestPt.pos;
				// If found point is closer than the previous point, update it
				auto newDist = sqrtf(Dot(foundPoint - cursorPos, foundPoint - cursorPos));
				if (newDist < dist1 || found2 == false)
				{
					found2 = true;
					// Intentional Q params
					endParams2 = nearestPt.params.GetQParams();
					dist2 = newDist;
				}
			}
		}
	}

	if (found1 && found2)
	{
		// Calculate first point
		auto foundPt = SimpleGradient(qSurface, endParams1, pSurface, endParams2);
		if (foundPt.found)
		{
			XMFLOAT3 foundPtPos = foundPt.pos;
			auto autoPtParamsQ = foundPt.params.GetQParams();
			auto autoPtParamsP = foundPt.params.GetPParams();

			qParamsList.push_back(XMFLOAT2(autoPtParamsQ.u, autoPtParamsQ.v));
			pParamsList.push_back(XMFLOAT2(autoPtParamsP.u, autoPtParamsP.v));

			// Calculate other points				
			auto result = FindOtherIntersectionPoints(
				qSurface, autoPtParamsQ,
				pSurface, autoPtParamsP, foundPtPos);

			// Create the interpolation curve
			auto curve = m_factory->CreateIntersectionCurve(
				qNode, result.surfQParamsList, result.m_qIntersectionClosed,
				pNode, result.surfPParamsList, result.m_pIntersectionClosed);
			m_scene->AttachObject(curve);
			return;
		}
		else {
			// TODO could not find and intersection
		}
	}	
}

IntersectionPointSearchData IntersectionFinder::FindNextPointAdaptiveStep(
	IParametricSurface* qSurf,
	IParametricSurface* pSurf,
	ParameterQuad parameters,
	DirectX::XMFLOAT3 prevPoint,
	bool reverseDirection, 
	float step)
{
	IntersectionPointSearchData result;
	result.found = false;
	result.pos = prevPoint;
	result.params = parameters;	

	float currentStep = step;
	int maxStepDivisions = 5;
	int divisionsCounter = 0;

	auto searchRes = FindNextPoint(qSurf, pSurf, parameters, prevPoint, reverseDirection, step);
	while (searchRes.found == false && maxStepDivisions > divisionsCounter)
	{
		currentStep /= 2.f;
		divisionsCounter++;
		searchRes = FindNextPoint(qSurf, pSurf, parameters, prevPoint, reverseDirection, currentStep);		
	}

	if (searchRes.found)
	{
		result = searchRes;
	}

	return result;
}


// distance <0 signals that an intersection with given border is not in this direction
DirectX::XMFLOAT4 GetBorderDistancesInDir(IParametricSurface* qSurface, IParametricSurface* pSurface, ParameterQuad x_kParam, ParameterQuad x_prevParam)
{
	XMFLOAT4 x_k = x_kParam.GetVector();
	XMFLOAT4 x_prev = x_prevParam.GetVector();

	XMFLOAT4 distances;
	ParameterPair qMax = qSurface->GetMaxParameterValues();
	ParameterPair pMax = pSurface->GetMaxParameterValues();
	ParameterQuad maxVals;
	maxVals.Set(qMax, pMax);
	XMFLOAT4 maxValsF4 = maxVals.GetVector();

	auto deltaParams = (x_k - x_prev);
	DirectX::XMFLOAT4 distancesMax, distancesZero;

	for (int i = 0; i < 4; i++)
	{
		SetAt(distances, i, FLT_MAX);
		SetAt(distancesMax, i, FLT_MAX);
		SetAt(distancesZero, i, FLT_MAX);
	}

	for (int i = 0; i < 4; i++)
	{	
		float deltaCoord = GetAt(deltaParams, i);
		if (deltaCoord != 0.0f)
		{
			float zeroDist = (0.f - GetAt(x_prev, i)) / deltaCoord;
			float maxDist = (GetAt(maxValsF4,i) - GetAt(x_prev, i)) / deltaCoord;			
			
			SetAt(distancesMax, i, maxDist);
			SetAt(distancesZero, i, zeroDist);

			float distInDir = maxDist >= 0.0f ? maxDist : zeroDist;
			SetAt(distances, i, distInDir);
		}
	}

	return distances;
}

ParameterQuad GetParamsOnOpposingEdge(IParametricSurface* qSurface, IParametricSurface* pSurface, ParameterQuad params)
{
	XMFLOAT4 wrappedParam;
	ParameterQuad maxParams;
	ParameterPair maxQParams = qSurface->GetMaxParameterValues();
	ParameterPair maxPParams = pSurface->GetMaxParameterValues();
	maxParams.Set(maxQParams, maxPParams);
	XMFLOAT4 maxParamsF4 = maxParams.GetVector();

	for (int i = 0; i < 4; i++)
	{
		float iMaxParam = GetAt(maxParamsF4, i);
		float curParam = GetAt(params.GetVector(), i);
		
		if (curParam == 0.0f)
		{
			curParam = iMaxParam;
		}
		else if(curParam == iMaxParam) 
		{
			curParam = 0.0f;
		}
		
		SetAt(wrappedParam, i, curParam);
	}

	return ParameterQuad(wrappedParam);
}

std::vector<XMFLOAT4> IntersectionFinder::GetAuxiliaryPoints(IParametricSurface* qSurface, IParametricSurface* pSurface, ParameterQuad x_k, ParameterQuad x_prev)
{
	std::vector<XMFLOAT4> res;
	bool qInsideBounds = qSurface->ParamsInsideBounds(x_k.GetQParams());
	bool pInsideBounds = pSurface->ParamsInsideBounds(x_k.GetPParams());

	if (qInsideBounds && pInsideBounds)
	{
		auto diff = x_k - x_prev;

		// IF x_k - x_prev line intersects one of the border lines - add auxiliary points
		XMFLOAT4 distances = GetBorderDistancesInDir(qSurface, pSurface, x_k, x_prev);			
		std::priority_queue<float, std::vector<float>, std::greater<float>> distancesQueue;

		for(int i = 0; i < 4; i++)
		{
			float dist = GetAt(distances, i);
			if (dist < 1.0f) // Intersection is between these two points
			{
				distancesQueue.push(dist);
			}
		}

		float lastDist = -1E-6f;
		while (distancesQueue.empty() == false)
		{
			float curDist = distancesQueue.top();
			if(lastDist < curDist) // To not add the same point several times
			{
				lastDist = curDist; 
				if (curDist == 0.0f) {
					// Add only the wrapped point
					ParameterQuad wrappedBoundParams = GetParamsOnOpposingEdge(qSurface, pSurface, x_prev);
					res.push_back(wrappedBoundParams.GetVector());
				}
				else {
					ParameterQuad boundParams = x_prev + curDist * diff.GetVector();
					ParameterQuad wrappedBoundParams = GetParamsOnOpposingEdge(qSurface, pSurface, boundParams);
					res.push_back(boundParams.GetVector());
					res.push_back(wrappedBoundParams.GetVector());
				}
			}

			distancesQueue.pop();
		}
	}

	//TODO: wrapp all of the parameters?
	return res;
}

ParameterQuad IntersectionFinder::GetWrappedParameters(
	IParametricSurface* qSurface, 
	IParametricSurface* pSurface, 
	ParameterQuad parameters)
{
	float u = parameters.u;
	float v = parameters.v;
	float s = parameters.s;
	float t = parameters.t;

	auto wrappedQParams = qSurface->GetWrappedParams(u, v);
	auto wrappedPParams = pSurface->GetWrappedParams(s, t);
	
	ParameterQuad res;
	res.Set(wrappedQParams, wrappedPParams);
	return res;
}

IntersectionSearchResultOneDir IntersectionFinder::FindPointsInDirection(
	IParametricSurface* qSurface, IParametricSurface* pSurface, ParameterQuad startParams, 
	bool direction, bool checkLooped, int pointCap, XMFLOAT3 firstPoint)
{	
	ParameterQuad wrappedStartParams = GetWrappedParameters(qSurface, pSurface, startParams);
	IntersectionSearchResultOneDir result;

	auto GetNormalizedParams = [](IParametricSurface* surface, ParameterPair params)
	{
		ParameterPair res;
		auto maxParams = surface->GetMaxParameterValues();
		float maxU = maxParams.u;
		float maxV = maxParams.v;
		res.u = params.u / maxU;
		res.v = params.v / maxV;
		return res;
	};

	auto GetParamSpaceDistance = [GetNormalizedParams](IParametricSurface* surface, ParameterPair params1, ParameterPair params2)
	{	
		auto maxParams = surface->GetMaxParameterValues();
		auto paramDist = surface->GetParameterSpaceDistance(params1, params2);
	
		XMFLOAT2 scaledDist = { paramDist.x / maxParams.u, paramDist.y / maxParams.v };
		return sqrt(Dot(scaledDist, scaledDist));
	};

	bool borderPoint = false;
	LoopData loopData;
	IntersectionPointSearchData curSearchData;
	curSearchData.found = false;
	curSearchData.pos = firstPoint;
	curSearchData.params = wrappedStartParams;


	// Find the first point
	curSearchData = FindNextPointAdaptiveStep(qSurface, pSurface, curSearchData.params, curSearchData.pos, direction, m_step);
	if (curSearchData.found)
	{
		loopData.m_pOnBorder = curSearchData.pPointOnBorder;
		loopData.m_qOnBorder = curSearchData.qPointOnBorder;
		borderPoint = loopData.m_pOnBorder || loopData.m_qOnBorder;
	}

	float minDistTotal = FLT_MAX;
	//TODO stop if result is on the border?
	bool sizeNotCapped = result.surfQParamsList.size() < pointCap;
	while (curSearchData.found && sizeNotCapped && !borderPoint)
	{
		bool looped = false;
		if (checkLooped)
		{
			// Check if current point is very close to the first point
			auto diff = firstPoint - curSearchData.pos;
			auto dist = Dot(diff, diff);
			bool trueLoop = false;

			// TODO check distance here based on PROPER WRAPPED PARAM SPACE 
			//Check if parameters are close to each other
			auto qParamDiff = GetParamSpaceDistance(qSurface, wrappedStartParams.GetQParams(), curSearchData.params.GetQParams());
			auto pParamDiff = GetParamSpaceDistance(pSurface, wrappedStartParams.GetPParams(), curSearchData.params.GetPParams());
			
			trueLoop = min(qParamDiff, pParamDiff) < m_loopPrecision;
			minDistTotal = min(minDistTotal, min(qParamDiff, pParamDiff));
			if (dist <= m_step / 2.f && trueLoop) {
				result.surfQParamsList.push_back(curSearchData.params.GetQParams().GetVector());
				result.surfPParamsList.push_back(curSearchData.params.GetPParams().GetVector());
				result.surfQParamsList.push_back(wrappedStartParams.GetQParams().GetVector());
				result.surfPParamsList.push_back(wrappedStartParams.GetPParams().GetVector());


				curSearchData.found = false;
				loopData.m_qLooped = true;
				loopData.m_pLooped = true;
				/*if (qParamDiff < m_loopPrecision * 1.5f)
					
				if (pParamDiff < m_loopPrecision * 1.5f)
				*/	
				looped = true;
			}

		}
		if (!checkLooped || !looped)
		{
			// Do not check for loops or didnt find any
			result.surfQParamsList.push_back(curSearchData.params.GetQParams().GetVector());
			result.surfPParamsList.push_back(curSearchData.params.GetPParams().GetVector());
			curSearchData = FindNextPointAdaptiveStep(qSurface, pSurface, curSearchData.params, curSearchData.pos, direction, m_step);
			if (curSearchData.found)
			{
				loopData.m_pOnBorder = curSearchData.pPointOnBorder;
				loopData.m_qOnBorder = curSearchData.qPointOnBorder;
				borderPoint = loopData.m_pOnBorder || loopData.m_qOnBorder;
				if (borderPoint)
				{
					result.surfQParamsList.push_back(curSearchData.params.GetQParams().GetVector());
					result.surfPParamsList.push_back(curSearchData.params.GetPParams().GetVector());
				}
			}
		}

		sizeNotCapped = result.surfQParamsList.size() < pointCap;
	}

	result.m_loopData.m_qOnBorder = loopData.m_qOnBorder;
	result.m_loopData.m_pOnBorder = loopData.m_pOnBorder;
	result.m_loopData.m_qLooped = loopData.m_qLooped;
	result.m_loopData.m_pLooped = loopData.m_pLooped;
	return result;
}

IntersectionSearchResult IntersectionFinder::FindOtherIntersectionPoints(
	IParametricSurface* surfaceQ, ParameterPair surfQParams,
	IParametricSurface* surfaceP, ParameterPair surfPParams,
	DirectX::XMFLOAT3 firstPoint)
{
	int oneDirectionPointCap = 500;
	IntersectionSearchResult algorithmResult;
	IntersectionSearchResultOneDir resForward, resBackward;
	ParameterQuad startParams;
	startParams.Set(surfQParams, surfPParams);
	startParams = GetWrappedParameters(surfaceQ, surfaceP, startParams);

	resForward = FindPointsInDirection(
		surfaceQ, surfaceP, startParams, false, true, oneDirectionPointCap, firstPoint);

	if (resForward.m_loopData.GetIsLooped() == false)
	{
		resBackward = FindPointsInDirection(
			surfaceQ, surfaceP, startParams, true, false, oneDirectionPointCap, firstPoint);
	}
	
	// Copy Param list Data
	std::vector<DirectX::XMFLOAT2> forwardsQ = resForward.surfQParamsList;
	std::vector<DirectX::XMFLOAT2> forwardsP = resForward.surfPParamsList;
	std::vector<DirectX::XMFLOAT2> backwardsQ = resBackward.surfQParamsList;
	std::vector<DirectX::XMFLOAT2> backwardsP = resBackward.surfPParamsList;

	std::reverse(forwardsQ.begin(), forwardsQ.end());
	std::reverse(forwardsP.begin(), forwardsP.end());

	

	for (int i = 0; i < forwardsQ.size(); i++)
	{
		algorithmResult.surfQParamsList.push_back(forwardsQ[i]);
		algorithmResult.surfPParamsList.push_back(forwardsP[i]);
	}

	algorithmResult.surfQParamsList.push_back(startParams.GetQParams().GetVector());
	algorithmResult.surfPParamsList.push_back(startParams.GetPParams().GetVector());

	for (int i = 0; i < backwardsQ.size(); i++)
	{
		algorithmResult.surfQParamsList.push_back(backwardsQ[i]);
		algorithmResult.surfPParamsList.push_back(backwardsP[i]);
	}


	// Copy Loop Data
	bool qClosed = resForward.m_loopData.m_qOnBorder && resBackward.m_loopData.m_qOnBorder;
	bool pClosed = resForward.m_loopData.m_pOnBorder && resBackward.m_loopData.m_pOnBorder;
	bool qLooped = resForward.m_loopData.m_qLooped || resBackward.m_loopData.m_qLooped;
	bool pLooped = resForward.m_loopData.m_pLooped || resBackward.m_loopData.m_pLooped;

	algorithmResult.m_qIntersectionClosed = qClosed || qLooped;
	algorithmResult.m_pIntersectionClosed = pClosed || pLooped;

	return algorithmResult;
}

ClampedPointData FindClampedPosition(ParameterQuad x_kQuad, ParameterQuad x_prevQuad, float step, bool xkInQ, bool xkInP)
{
	ClampedPointData res;

	DirectX:XMFLOAT4 x_k, x_prev;
	x_k = x_kQuad.GetVector();
	x_prev = x_prevQuad.GetVector();
	// Given a point x_prev inside a 4 dimensional cube [0,1]x[0,1]x[0,1]x[0,1] and the next step x_k
	// Find a point on the boundary of the cube that lies on the line intersecting x_k and x_prev

	auto delta = x_k - x_prev;
	bool isZeroDist = false;
	float dist = FLT_MAX;
	float eps = 10E-5;

	DirectX::XMFLOAT4 distances;
	for (int i = 0; i < 4; i++)
	{
		SetAt(distances, i, FLT_MAX);

		float deltaCoord = GetAt(delta, i);
		if (deltaCoord != 0.0f)
		{
			float zeroDist = (0.f - GetAt(x_prev, i)) / deltaCoord;
			float oneDist = (1.f - GetAt(x_prev, i)) / deltaCoord;

			float val = FLT_MAX;
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
			
			SetAt(distances, i, val);
			if (val < dist)
			{
				dist = val;
				isZeroDist = isValZeroDist;
			}
		}
	}

	float minQDist = min(abs(distances.x), abs(distances.y));
	float minPDist = min(abs(distances.z), abs(distances.w));
	// by default dist is the smallest value, but this value can be discarded here
	if (xkInQ)
	{
		dist = minPDist;
	}

	if (xkInP)
	{
		dist = minQDist;
	}

	assert(dist != FLT_MAX);

	auto normDelta = delta / sqrt(Dot(delta, delta));
	auto resParams = x_prev + delta * dist;

	for (int i = 0; i < 4; i++)
	{
		float val = GetAt(delta, i);
		if (val < eps)
			val = 0.0f;
		if (val > 1.f - eps)
			val = 1.0f;
		SetAt(delta, i, val);
	}
	
	auto modifiedStep = delta * dist;

	if (Dot(modifiedStep, modifiedStep) > step * step)
	{
		resParams = x_k;
	}

	res.params = resParams;
	res.distances = distances;
	return res;
}

IntersectionPointSearchData IntersectionFinder::FindNextPoint(
	IParametricSurface* qSurf, 
	IParametricSurface* pSurf, 
	ParameterQuad parameters,
	DirectX::XMFLOAT3 prevPoint,
	bool isReverseStepDirection, float step)
{	
	int iterationCounter = 0;

	IntersectionPointSearchData result;
	result.params = parameters;
	result.pos = prevPoint;

	// Initialize x_0
	ParameterQuad x_k, x_prev;
	x_k = parameters;
	x_prev = x_k = GetWrappedParameters(qSurf, pSurf, x_k);

	bool continueNewtonCalculation = true;
	while (continueNewtonCalculation)
	{						
		// Calculate Step direction
		auto stepVersor = CalculateStepDirection(qSurf, pSurf, x_k, isReverseStepDirection);	
		// Calculate F(x)		
		auto funcVal = CalculateIntersectionDistanceFunctionValue(
			qSurf, pSurf, x_k,
			prevPoint, stepVersor, step);

		auto val = Dot(funcVal, funcVal);
		if (val <= m_precision * m_precision)
		{ 
			// Solution is satisfying, is not on a parameter border
			continueNewtonCalculation = false;
			
			result.found = true;
			result.pos = pSurf->GetPoint(x_k.GetPParams());
			result.params = x_k;
		}
		else
		{
			// Calculate next iteration
			// Create a linear equation system and solve it
			DirectX::XMFLOAT4X4 derMatrix = CalculateDerivativeMatrix(qSurf, pSurf, x_k, stepVersor);
			DirectX::XMFLOAT4 deltaXGetp = Geom::SolveLinearEquationSystem(derMatrix, -1.f * funcVal, Geom::SolverType::GE);

			// Find the next point using Newton's method to solve linear equation system
			x_prev = x_k;
			x_k = x_k + ParameterQuad(deltaXGetp); // Why the minus?	

			// TODO check if point insersect boundaries. If yes - add points on the boundaries to the result and return them in the modified struct
			result.auxPoints = GetAuxiliaryPoints(qSurf, pSurf, x_k, x_prev);
			x_k = GetWrappedParameters(qSurf, pSurf, x_k);

			// One or more surfaces are out of bounds - try to find a boundary closing point
			if ((qSurf->ParamsInsideBounds(x_k.GetQParams()) &&
				 pSurf->ParamsInsideBounds(x_k.GetPParams())
				) == false)
			{
				// Check if the distance between clamped params and prev point is smaller than step 
				//if yes - connect the curve to the patch boundary and flag this end as looped

				auto boundarySearchRes = FindBoundaryPoint(x_k, x_prev, qSurf, pSurf, step);
				if (boundarySearchRes.found)
				{
					result = boundarySearchRes;
				}

				// Boundary solution found or boundary params out of bounds - either way end Newton
				continueNewtonCalculation = false;
			}
		}
	
		// TODO: replace with a cleaner solution
		iterationCounter++;
		if (iterationCounter > m_iterationCounter)
		{
			continueNewtonCalculation = false;
		}
	}

	return result;
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

	// wspó³czynnik z³otego podzia³u
	float k = (sqrt(5.f) - 1.f) / 2.f;

	auto qDir = DirectX::XMFLOAT2(d_k.x, d_k.y);
	auto pDir = DirectX::XMFLOAT2(d_k.z, d_k.w);

	float xLq = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, qDir, b);
	float xLp = pSurface->GetFarthestPointInDirection(x_k.z, x_k.w, pDir, b);
	b = abs(xLq) > abs(xLp) ? xLp : xLq;

	float xRq = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, qDir, a);
	float xRp = pSurface->GetFarthestPointInDirection(x_k.z, x_k.w, pDir, a);
	a = abs(xRq) > abs(xRp) ? xRp : xRq;

	// lewa i prawa próbka
	float xL = b - k * (b - a);
	float xR = a + k * (b - a);

	// pêtla póki nie zostanie spe³niony warunek stopu
	while ((b - a) > eps)
	{
		// if x_k + d_k * xL is out of bounds
		// update bounds and update x_l and calculate fL		

		// TODO if the smallest value is zero then no move can be made 		

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
	return (a + b) / 2.f;
}

float IntersectionFinder::GoldenRatioMethodForCursor(
	IParametricSurface* qSurface,
	float a, float b,
	DirectX::XMFLOAT2 x_k,
	DirectX::XMFLOAT2 d_k,
	DirectX::XMFLOAT3 cursorPos)
{
	float eps = m_goldenRatioPrecision;

	// wspó³czynnik z³otego podzia³u
	float k = (sqrt(5) - 1) / 2;

	b = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, d_k, b);
	a = qSurface->GetFarthestPointInDirection(x_k.x, x_k.y, d_k, a);

	// lewa i prawa próbka
	float xL = b - k * (b - a);
	float xR = a + k * (b - a);

	// pêtla póki nie zostanie spe³niony warunek stopu
	while ((b - a) > eps)
	{

		// TODO if the smallest value is zero then no move can be made 
		if (xL * xR == 0.0f)
		{
			break;
		}

		auto fL = CalcFuncForCursor(qSurface, x_k, d_k, cursorPos, xL);
		auto fR = CalcFuncForCursor(qSurface, x_k, d_k, cursorPos, xR);
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

IntersectionPointSearchData IntersectionFinder::SimpleGradient(
	IParametricSurface* qSurface,
	ParameterPair qSurfParams,
	IParametricSurface* pSurface,
	ParameterPair pSurfParams)
{

	// Initialize x_0
	ParameterQuad x_k;
	x_k.Set(qSurfParams, pSurfParams);

	IntersectionPointSearchData res;
	res.found = false;
	res.params = x_k;
	res.pos = XMFLOAT3(NAN, NAN, NAN);

	bool continueSearch = true;
	int iterationCounter = 0;

	DirectX::XMFLOAT4 dir_k;
	float curDist, prevDist;
	curDist = prevDist = GetCurrentFuncValue(qSurface, x_k.GetQParams(), pSurface, x_k.GetPParams());

	while (continueSearch)
	{
		// Determine residual direction
		dir_k = -1.f * CalculateGradient(qSurface, x_k.GetQParams(), pSurface, x_k.GetPParams());

		float alpha = GoldenRatioMethod(qSurface, pSurface, -0.5f, 0.5f, x_k.GetVector(), dir_k);
		// X_k = x_k-1 + (step dist) * (step dir)
		x_k = x_k + alpha * dir_k;
		
		// Check if the parameters from next iteration are in bound
		if (!qSurface->ParamsInsideBounds(x_k.GetQParams()) ||
			!pSurface->ParamsInsideBounds(x_k.GetPParams()))
		{
			// Stop the algorithm
			continueSearch = false;
			break;
		}

		// Inside bounds
		// Get the updated parameters
		prevDist = curDist;
		curDist = GetCurrentFuncValue(qSurface, x_k.GetQParams(), pSurface, x_k.GetPParams());

		// After 3 iterations reset the method
		iterationCounter++;
		if (iterationCounter > 200 ||
			curDist >= prevDist)
		{
			continueSearch = false;
		}
		
		if (curDist <= m_gradientPrecision)
		{
			continueSearch = false;
			res.found = true;
			res.params = x_k;
			res.pos = qSurface->GetPoint(x_k.GetQParams());			
		}
	}

	return res;
}

IntersectionPointSearchData IntersectionFinder::SimpleGradientForCursor(
	IParametricSurface* qSurface, 
	ParameterPair qSurfParams, 
	DirectX::XMFLOAT3 cursorPos)
{
	IntersectionPointSearchData result;
	result.found = false;
	result.pos = XMFLOAT3(NAN, NAN, NAN);
	result.params.Set(qSurfParams, { NAN, NAN });

	// Initialize x_0	
	ParameterPair x_k, x_prev;
	x_prev = x_k = qSurfParams;

	// Calculate residual direction and conjugated move direction	
	float curDist = GetCurrentFuncValueForCursor(qSurface, qSurfParams, cursorPos);
	float prevDist = curDist;

	int iterationCounter = 0;
	bool continueSearch = true;
	while (continueSearch)
	{
		DirectX::XMFLOAT2  d_k = -1.f * CalculateGradientForCursor(qSurface, x_k, cursorPos);

		float alpha = GoldenRatioMethodForCursor(qSurface, -0.5f, 0.5f, x_k.GetVector(), d_k, cursorPos);
		x_prev = x_k;
		x_k = x_k + alpha * d_k;
		
		// Check if the parameters from next iteration are in bound
		if (qSurface->ParamsInsideBounds(x_k) == false)		
		{
			// Stop the algorithm
			continueSearch = false;			
			break;
		}

		// Inside bounds
		// Get the updated parameters
		prevDist = curDist;
		curDist = GetCurrentFuncValueForCursor(qSurface, x_k, cursorPos);

		iterationCounter++;
		// Points here could be approximate - they will act as an input to a second method which ther calculates precise position
		if (iterationCounter > 3)
		{
			continueSearch = false;
			result.found = true;
			result.params.Set(x_k, { NAN, NAN });
			result.pos = qSurface->GetPoint(x_k);
		}

		if (curDist > prevDist)
		{
			continueSearch = false;
			result.found = true;
			result.params.Set(x_prev, { NAN, NAN });
			result.pos = qSurface->GetPoint(x_prev);
		}

		if (curDist <= m_cursorCGprecision)
		{
			continueSearch = false;
			result.found = true;
			result.params.Set(x_k, { NAN, NAN });
			result.pos = qSurface->GetPoint(x_k);
		}
	}

	return result;
}
