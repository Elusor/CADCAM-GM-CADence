#include "DetailPathsCreationManager.h"
#include "ParametricOffsetSurface.h"
#include "IParametricSurface.h"
#include "Scene.h"
#include "PathUtils.h"

DetailPathsCreationManager::DetailPathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene)
{
	m_intersectionFinder = intersectionFinder;
	m_scene = scene;
}

void DetailPathsCreationManager::CreateDetailPaths(PathModel* model)
{
	float offset = -0.09F; // 0.08 + 0.01 eps

	auto modelObjects = model->GetModelObjects();
	auto bodyObject		= dynamic_cast<IParametricSurface*>(modelObjects[0].lock().get()->m_object.get());
	auto eyeObject		= dynamic_cast<IParametricSurface*>(modelObjects[1].lock().get()->m_object.get());
	auto hairObject		= dynamic_cast<IParametricSurface*>(modelObjects[2].lock().get()->m_object.get());
	auto backFinObject	= dynamic_cast<IParametricSurface*>(modelObjects[3].lock().get()->m_object.get());
	auto sideFinObject	= dynamic_cast<IParametricSurface*>(modelObjects[4].lock().get()->m_object.get());
	auto sideSpikes		= dynamic_cast<IParametricSurface*>(modelObjects[5].lock().get()->m_object.get());

	// Create offset surfaces for each object in the model
	auto bodyOffsetObject		= ParametricOffsetSurface(bodyObject, offset);
	auto eyeOffsetObject		= ParametricOffsetSurface(eyeObject, offset);
	auto hairOffsetObject		= ParametricOffsetSurface(hairObject, offset);
	auto backFinOffsetObject	= ParametricOffsetSurface(backFinObject, offset);
	auto sideFinOffsetObject	= ParametricOffsetSurface(sideFinObject, offset);
	auto sideSpikesOffsetObject = ParametricOffsetSurface(sideSpikes, offset);
	
#pragma region Calculate intersections
	// Intersect the models that should be intersected
	auto bodyXbackFinIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&backFinOffsetObject,
		DirectX::XMFLOAT3(3.8f, 0.5f, -0.5f)
	);

	auto bodyXsideFinIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&sideFinOffsetObject,
		DirectX::XMFLOAT3(3.8f, 0.5f, -0.5f)
	);

	auto bodyXeyeIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&eyeOffsetObject,
		DirectX::XMFLOAT3(-5.12f, 1.53f, -0.95f)
	);

	auto bodyXhairIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(-3.1f, 4.5f, -0.65f)
	);

	auto bodyXhairIntersection2 = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(2.37f, 1.5f, -0.225F)
	);

	auto leftUpper = DirectX::XMFLOAT3(-3.25f, 1.66f, -1.62f);
	auto leftLower = DirectX::XMFLOAT3(-3.39f, -0.54f, -1.8f);
	auto rightUpper = DirectX::XMFLOAT3(-1.32f, 1.71f, -1.46f);
	auto rightLower = DirectX::XMFLOAT3(-1.32f, -0.48f, -1.68f);

	auto bodyXsideSpikeLU = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&sideSpikesOffsetObject,
		leftUpper
	);
	auto bodyXsideSpikeLL = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&sideSpikesOffsetObject,
		leftLower
	);

	//auto bodyXsideSpikeRU = m_intersectionFinder->FindIntersectionWithCursor(
	//	&bodyOffsetObject,
	//	&sideSpikesOffsetObject,
	//	rightUpper
	//);
	//auto bodyXsideSpikeRL = m_intersectionFinder->FindIntersectionWithCursor(
	//	&bodyOffsetObject,
	//	&sideSpikesOffsetObject,
	//	rightLower
	//);
#pragma endregion

	auto normalizedBackFin = NormalizeParameters(bodyXbackFinIntersection.surfPParams, backFinObject);
	auto backFinPathPointsParams = PrepareBackFin(normalizedBackFin);
	auto denormalizedBackFinPathPointParams = DenormalizeParameters(backFinPathPointsParams, backFinObject);
	// VisualizeCurve(&backFinOffsetObject, denormalizedBackFinPathPointParams);

	auto normalizedSideFin = NormalizeParameters(bodyXsideFinIntersection.surfPParams, sideFinObject);
	auto sideFinPathPointsParams = PrepareSideFin(normalizedSideFin);
	auto denormalizedSideFinPathPointParams = DenormalizeParameters(sideFinPathPointsParams, sideFinObject);
	// VisualizeCurve(&sideFinOffsetObject, denormalizedSideFinPathPointParams);

	auto normalizedEye = NormalizeParameters(bodyXeyeIntersection.surfPParams, eyeObject);
	auto eyePathPointsParams = PrepareEye(normalizedEye);
	auto denomarlizedEyePathPointParams = DenormalizeParameters(eyePathPointsParams, eyeObject);
	// VisualizeCurve(&eyeOffsetObject, denomarlizedEyePathPointParams);

	auto normalizeBodySideFin = NormalizeParameters(bodyXsideFinIntersection.surfQParams, bodyObject);
	auto normalizeBodyBackFin = NormalizeParameters(bodyXbackFinIntersection.surfQParams, bodyObject);
	auto normalizeBodyEye = NormalizeParameters(bodyXeyeIntersection.surfQParams, bodyObject);
	auto normalizeBodyHair = NormalizeParameters(bodyXhairIntersection.surfQParams, bodyObject);
	auto normalizeBodyHair2 = NormalizeParameters(bodyXhairIntersection2.surfQParams, bodyObject);


	std::vector<DirectX::XMFLOAT2> spikeParams;
	auto spikeQParamUp = bodyXsideSpikeLU.surfQParams;
	auto spikeQParamBot = bodyXsideSpikeLL.surfQParams;
	spikeParams.insert(spikeParams.end(), spikeQParamUp.begin(), spikeQParamUp.end());
	spikeParams.insert(spikeParams.end(), spikeQParamBot.begin(), spikeQParamBot.end());
	spikeParams.push_back(spikeParams[0]);

	auto normalizeBodySpikes = NormalizeParameters(spikeParams, bodyObject);

	auto bodyPathPointsParams = PrepareBody(
		normalizeBodySideFin,
		normalizeBodyBackFin,
		normalizeBodyEye,
		normalizeBodyHair,
		normalizeBodyHair2,
		normalizeBodySpikes
	);
	auto denormalizedBodyPathPointParams = DenormalizeParameters(bodyPathPointsParams, bodyObject);
	VisualizeCurve(&bodyOffsetObject, denormalizedBodyPathPointParams);
	
	auto normalizeHair = NormalizeParameters(bodyXhairIntersection.surfPParams, hairObject);
	auto normalizeHair2 = NormalizeParameters(bodyXhairIntersection2.surfPParams, hairObject);
	auto hairPathPointParams = PrepareHair(
		normalizeHair,
		normalizeHair2
	);
	auto denormalizedHairPathPointParams = DenormalizeParameters(hairPathPointParams, hairObject);
	VisualizeCurve(&hairOffsetObject, denormalizedHairPathPointParams);
	// Trim the "single intersection objects"

	// Merge the trimmed ares of the base parts

	// Merge the trimmed areas of the main body


#pragma region Visualize intersections
	/*VisualizeCurve(&bodyOffsetObject, bodyXbackFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXsideFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXeyeIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection2);
	*/
	//VisualizeCurve(&bodyOffsetObject, spikeParams);
#pragma endregion



}

void DetailPathsCreationManager::VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params)
{
	std::vector<ObjectRef> points;
	for (auto param : params)
	{
		auto pos = surface->GetPoint(param.x, param.y);
		Transform ptTransform;
		ptTransform.SetPosition(pos);
		ptTransform.Translate(0, 0, 0);
		auto point = m_factory.CreatePoint(ptTransform);
		point->SetIsVirtual(true);
		m_scene->AttachObject(point);
		points.push_back(point);
	}
	m_scene->AttachObject(m_factory.CreateInterpolBezierCurveC2(points));
}

bool DetailPathsCreationManager::SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions, std::string name)
{
	return false;
}

void DetailPathsCreationManager::PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr)
{
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::NormalizeParameters(
	const std::vector<DirectX::XMFLOAT2>& intersectionCurve,
	IParametricSurface* surface)
{
	// Todo maybe its better to normalize at the end?
	std::vector<DirectX::XMFLOAT2> normalizedPoints;
	normalizedPoints.reserve(intersectionCurve.size());
	auto maxValues = surface->GetMaxParameterValues();

	for (auto pt : intersectionCurve)
	{
		normalizedPoints.push_back(
			DirectX::XMFLOAT2(
				pt.x / maxValues.u,
				pt.y / maxValues.v
			)
		);
	}

	return normalizedPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::DenormalizeParameters(const std::vector<DirectX::XMFLOAT2>& intersectionCurve, IParametricSurface* surface)
{
	std::vector<DirectX::XMFLOAT2> result;

	auto maxParams = surface->GetMaxParameterValues();
	for (auto pt : intersectionCurve)
	{
		auto params = surface->GetWrappedParams(pt.x * maxParams.u, pt.y * maxParams.v);
		result.push_back(params.GetVector());
	}

	return result;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareBackFin(const std::vector<DirectX::XMFLOAT2>& bodyXbackFinIntersectionCurve)
{
	auto x = 2;
	// Adjust this to make sure the points at the end are ok
	float maxParam = 0.65f;

	size_t index = 1;
	for (; index < bodyXbackFinIntersectionCurve.size(); index++)
	{
		auto pt = bodyXbackFinIntersectionCurve[index];
		auto ptPrev = bodyXbackFinIntersectionCurve[index - 1];

		if(pt.y < ptPrev.y)
		{
			break;
		}
	}

	// Make a proper list from 0 to 1 parameter vertically
	std::vector<DirectX::XMFLOAT2> reorganizedPoints(bodyXbackFinIntersectionCurve.begin() + index, bodyXbackFinIntersectionCurve.end());
	reorganizedPoints.insert(reorganizedPoints.end(), bodyXbackFinIntersectionCurve.begin(), bodyXbackFinIntersectionCurve.begin() + index);

	float baseParamsCutoffstart = 1.f / 6.f;
	float baseParamsCutoffend = baseParamsCutoffstart + 0.5f;

	std::vector<DirectX::XMFLOAT2> lineStart{
		DirectX::XMFLOAT2{ 0.0f , baseParamsCutoffstart},
		DirectX::XMFLOAT2{ maxParam , baseParamsCutoffstart}
	};

	std::vector<DirectX::XMFLOAT2> lineEnd{
		DirectX::XMFLOAT2{ 0.0f , baseParamsCutoffend},
		DirectX::XMFLOAT2{ maxParam , baseParamsCutoffend}
	};

	auto startIntersection = IntersectCurves(reorganizedPoints, lineStart);
	auto endIntersection = IntersectCurves(reorganizedPoints, lineEnd);
	
	std::vector<DirectX::XMFLOAT2> frontLine;	
	frontLine.push_back(startIntersection[0].intersectionPoint);
	frontLine.insert(
		frontLine.end(),
		reorganizedPoints.begin() + startIntersection[0].qLineIndex + 1, 
		reorganizedPoints.begin() + endIntersection[0].qLineIndex + 1);
	frontLine.push_back(endIntersection[0].intersectionPoint);
	
	std::vector<DirectX::XMFLOAT2> backLine;
	backLine.push_back(DirectX::XMFLOAT2(maxParam, baseParamsCutoffstart));
	backLine.push_back(DirectX::XMFLOAT2(maxParam, baseParamsCutoffend));

	float stepsVertical = 15;
	float stepsHorizontal = 30;

	float stepWidthVertical = (baseParamsCutoffend - baseParamsCutoffstart) / (stepsVertical + 1);

	std::vector<DirectX::XMFLOAT2> pathPoints;

#pragma region  add the fist line end to beg
	auto startLineBeg = frontLine[0];
	auto startLineEnd = frontLine[0];
	startLineEnd.x = maxParam;

	pathPoints.push_back(startLineEnd);
	// add multiple intermediary points
	float stepHor0 = (startLineBeg.x - startLineEnd.x) / (stepsHorizontal + 1);
	for (size_t step = 1; step < stepsHorizontal; step++)
	{
		pathPoints.push_back(DirectX::XMFLOAT2(
			startLineEnd.x + step * stepHor0,
			startLineBeg.y
		));
	}
#pragma endregion

	size_t lastCurveIndex = 0;
	for (size_t step = 1; step <= stepsVertical; step+=2)
	{
		float currentVerticalParam = baseParamsCutoffstart + step * stepWidthVertical;
		float nextVerticalParam = baseParamsCutoffstart + (step + 1)* stepWidthVertical;
		
		std::vector<DirectX::XMFLOAT2> currentParamLine{
			DirectX::XMFLOAT2{ 0.0f , currentVerticalParam},
			DirectX::XMFLOAT2{ 1.5f , currentVerticalParam}
		};

		std::vector<DirectX::XMFLOAT2> nextParamLine{
			DirectX::XMFLOAT2{ 0.0f , nextVerticalParam},
			DirectX::XMFLOAT2{ 1.5f , nextVerticalParam}
		};

		auto frontIntersectionCur = IntersectCurves(frontLine, currentParamLine);
		auto backIntersectionCur = IntersectCurves(backLine, currentParamLine);
		
		auto frontIntersectionNext = IntersectCurves(frontLine, nextParamLine);
		auto backIntersectionNext = IntersectCurves(backLine, nextParamLine);

		// Add last curve index to front intersection
		pathPoints.insert(pathPoints.end(), 
			frontLine.begin() + lastCurveIndex, 
			frontLine.begin() + frontIntersectionCur[0].qLineIndex + 1// offset due to past-iterator argument
		);

		// cur intersection interPoint to end		
		pathPoints.push_back(frontIntersectionCur[0].intersectionPoint);
		// add multiple intermediary points
		float stepHor = (backIntersectionCur[0].intersectionPoint.x - frontIntersectionCur[0].intersectionPoint.x) / (stepsHorizontal + 1);
		for (size_t step = 1; step < stepsHorizontal; step++)
		{
			pathPoints.push_back(DirectX::XMFLOAT2(
				frontIntersectionCur[0].intersectionPoint.x + step * stepHor,
				backIntersectionCur[0].intersectionPoint.y
			));
		}		
		pathPoints.push_back(backIntersectionCur[0].intersectionPoint);		

		// next intersection end to interPoint
		pathPoints.push_back(backIntersectionNext[0].intersectionPoint);
		float stepHor2 = (frontIntersectionNext[0].intersectionPoint.x - backIntersectionNext[0].intersectionPoint.x) / (stepsHorizontal + 1);
		for (size_t step = 1; step < stepsHorizontal; step++)
		{
			pathPoints.push_back(DirectX::XMFLOAT2(
				backIntersectionNext[0].intersectionPoint.x + step * stepHor2,
				backIntersectionNext[0].intersectionPoint.y
			));
		}
		// add multiple intermediary points
		pathPoints.push_back(frontIntersectionNext[0].intersectionPoint);
				
		// update lastCurveIndex
		lastCurveIndex = frontIntersectionNext[0].qLineIndex + 1;
	}
	pathPoints.insert(pathPoints.end(), frontLine.begin() + lastCurveIndex, frontLine.end() - 1);

	// add the last line beg to end
	auto lastLineBeg = frontLine[frontLine.size()-1];
	auto lastLineEnd = lastLineBeg;
	startLineEnd.x = maxParam;

	pathPoints.push_back(startLineEnd);
	// add multiple intermediary points
	float stepHor3 = (lastLineEnd.x - lastLineBeg.x) / (stepsHorizontal + 1);
	for (size_t step = 1; step < stepsHorizontal; step++)
	{
		pathPoints.push_back(DirectX::XMFLOAT2(
			lastLineBeg.x + step * stepHor3,
			lastLineBeg.y
		));
	}

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareSideFin(const std::vector<DirectX::XMFLOAT2>& intersectionParams)
{
	float minHParam = 0.0f;
	float maxHParam = 1.0f;

	size_t steps = 40;
	size_t vSteps = 30;
	float stepHWidth = (maxHParam - minHParam) / steps;

	std::vector<DirectX::XMFLOAT2> pathPoints;

	std::vector<LineIntersectionData> allIntersections;
	for (size_t step = 0; step <= steps; step++)
	{
		float currentStep = step * stepHWidth;

		std::vector<DirectX::XMFLOAT2> scanLine{
			DirectX::XMFLOAT2(minHParam, currentStep),
			DirectX::XMFLOAT2(maxHParam, currentStep)
		};

		auto intersections = IntersectCurves(scanLine, intersectionParams);
		if (intersections.size() >= 2)
		{
			allIntersections.push_back(intersections[0]);
			allIntersections.push_back(intersections[1]);
		}
	}

	std::swap(
		allIntersections[allIntersections.size() - 1], 
		allIntersections[allIntersections.size() - 2]
	);

	auto segmentBot = ExtractSegmentFromOutline(
		intersectionParams, 
		allIntersections[1].pLineIndex, 
		allIntersections[0].pLineIndex);

	auto segmentUp = ExtractSegmentFromOutline(
		intersectionParams, 
		allIntersections[allIntersections.size()-1].pLineIndex, 
		allIntersections[allIntersections.size()-2].pLineIndex
	);

	// Left side
	// first point
	//pathPoints.push_back(allIntersections[0].intersectionPoint);
	//// segment arc
	//pathPoints.insert(pathPoints.end(), segmentBot.begin(), segmentBot.end());
	//// second point
	//pathPoints.push_back(allIntersections[1].intersectionPoint);

	auto lastIntersection = allIntersections[1];
	bool reversed = false;
	for (size_t index = 0; index < allIntersections.size() / 2; index++)
	{
		LineIntersectionData begIntersection;
		LineIntersectionData endIntersection;
		size_t startIndex = index * 2;

		if (reversed)
		{
			begIntersection = allIntersections[startIndex + 1];
			endIntersection = allIntersections[startIndex];
		}
		else
		{
			begIntersection = allIntersections[startIndex];
			endIntersection = allIntersections[startIndex + 1];
		}

		auto segment = ExtractSegmentFromOutline(
			intersectionParams, 
			lastIntersection.pLineIndex, 
			begIntersection.pLineIndex);
		if (!index || index == (allIntersections.size() / 2) - 1)
		{
			std::reverse(segment.begin(), segment.end());
		}

		if (segment.size() < intersectionParams.size() / 2)
		{
			pathPoints.insert(pathPoints.end(), segment.begin(), segment.end());
		}
		else
		{
			auto smaller = lastIntersection.pLineIndex < begIntersection.pLineIndex ? lastIntersection.pLineIndex : begIntersection.pLineIndex;
			auto bigger = lastIntersection.pLineIndex + begIntersection.pLineIndex - smaller;

			auto segment1 = ExtractSegmentFromOutline(
				intersectionParams,
				0,
				smaller);
			auto segment2 = ExtractSegmentFromOutline(
				intersectionParams,
				bigger,
				intersectionParams.size() - 1);

			pathPoints.insert(pathPoints.end(), segment2.begin(), segment2.end());
			pathPoints.insert(pathPoints.end(), segment1.begin(), segment1.end());
		}
		
		pathPoints.push_back(begIntersection.intersectionPoint);

		float diff = endIntersection.intersectionPoint.x - begIntersection.intersectionPoint.x;
		float diffStep = diff / (vSteps + 1);
		for (size_t vStep = 0; vStep < vSteps; vStep++)
		{
			pathPoints.push_back(
				DirectX::XMFLOAT2(
					begIntersection.intersectionPoint.x + vStep * diffStep,
					begIntersection.intersectionPoint.y
				)
			);
		}
		pathPoints.push_back(endIntersection.intersectionPoint);

		lastIntersection = endIntersection;
		reversed = !reversed;
	}

	// last point
	// second to last point
	std::reverse(segmentUp.begin(), segmentUp.end());
	pathPoints.insert(pathPoints.end(), segmentUp.begin(), segmentUp.end());
	//pathPoints.push_back(allIntersections[allIntersections.size()-1].intersectionPoint);

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareEye(const std::vector<DirectX::XMFLOAT2>& intersectionParams)
{

	auto segment1 = std::vector<DirectX::XMFLOAT2>(intersectionParams.begin(), intersectionParams.begin() + 19);
	auto segment2 = std::vector<DirectX::XMFLOAT2>(intersectionParams.begin() + 19, intersectionParams.begin() + 47);
	auto segment3 = std::vector<DirectX::XMFLOAT2>(intersectionParams.begin() + 47, intersectionParams.end());

	// Segments 1 and 3 add 0.5 Y
	// Segment 2 add 0.5 and substract 1

	float offset = 0.5f;

	for (auto& pt : segment1)
	{
		pt.y += offset;
	}

	for (auto& pt : segment2)
	{
		pt.y += (offset - 1.0f);
	}

	for (auto& pt : segment3)
	{
		pt.y += offset;
	}

	auto newIntersectionParams = segment1;
	newIntersectionParams.insert(newIntersectionParams.end(), segment2.begin() + 1, segment2.end());
	newIntersectionParams.insert(newIntersectionParams.end(), segment3.begin() + 1, segment3.end());

	float minHParam = 0.0f;
	float maxHParam = 1.0f;

	size_t steps = 30;
	size_t vSteps = 11;
	float stepHWidth = (maxHParam - minHParam) / steps;

	std::vector<DirectX::XMFLOAT2> pathPoints;

	std::vector<LineIntersectionData> allIntersections;
	for (size_t step = 0; step <= steps; step++)
	{
		float currentStep = step * stepHWidth;

		std::vector<DirectX::XMFLOAT2> scanLine{
			DirectX::XMFLOAT2(minHParam, currentStep),
			DirectX::XMFLOAT2(maxHParam, currentStep)
		};

		auto intersections = IntersectCurves(scanLine, newIntersectionParams);
		if (intersections.size() >= 2)
		{
			allIntersections.push_back(intersections[0]);
			allIntersections.push_back(intersections[1]);
		}
	}

	std::swap(
		allIntersections[allIntersections.size() - 1],
		allIntersections[allIntersections.size() - 2]
	);

	auto segmentBot = ExtractSegmentFromOutline(
		newIntersectionParams,
		allIntersections[1].pLineIndex,
		allIntersections[0].pLineIndex);

	auto segmentUp = ExtractSegmentFromOutline(
		newIntersectionParams,
		allIntersections[allIntersections.size() - 1].pLineIndex,
		allIntersections[allIntersections.size() - 2].pLineIndex
	);

	// Left side
	// first point
	//pathPoints.push_back(allIntersections[0].intersectionPoint);
	//// segment arc
	//pathPoints.insert(pathPoints.end(), segmentBot.begin(), segmentBot.end());
	//// second point
	//pathPoints.push_back(allIntersections[1].intersectionPoint);

	auto lastIntersection = allIntersections[1];
	bool reversed = false;
	for (size_t index = 0; index < allIntersections.size() / 2; index++)
	{
		LineIntersectionData begIntersection;
		LineIntersectionData endIntersection;
		size_t startIndex = index * 2;

		if (reversed)
		{
			begIntersection = allIntersections[startIndex + 1];
			endIntersection = allIntersections[startIndex];
		}
		else
		{
			begIntersection = allIntersections[startIndex];
			endIntersection = allIntersections[startIndex + 1];
		}


		auto segment = ExtractSegmentFromOutline(
			newIntersectionParams,
			lastIntersection.pLineIndex,
			begIntersection.pLineIndex);
		if (!index || index == (allIntersections.size() / 2) - 1)
		{
			std::reverse(segment.begin(), segment.end());
		}

		if (segment.size() < newIntersectionParams.size() / 2)
		{
			pathPoints.insert(pathPoints.end(), segment.begin(), segment.end());
		}
		else
		{
			auto smaller = lastIntersection.pLineIndex < begIntersection.pLineIndex ? lastIntersection.pLineIndex : begIntersection.pLineIndex;
			auto bigger = lastIntersection.pLineIndex + begIntersection.pLineIndex - smaller;

			auto segment1 = ExtractSegmentFromOutline(
				newIntersectionParams,
				0,
				smaller);
			auto segment2 = ExtractSegmentFromOutline(
				newIntersectionParams,
				bigger,
				newIntersectionParams.size() - 1);

			pathPoints.insert(pathPoints.end(), segment2.begin(), segment2.end());
			pathPoints.insert(pathPoints.end(), segment1.begin(), segment1.end());
		}

		pathPoints.push_back(begIntersection.intersectionPoint);

		float diff = endIntersection.intersectionPoint.x - begIntersection.intersectionPoint.x;
		float diffStep = diff / (vSteps + 1);
		for (size_t vStep = 0; vStep < vSteps; vStep++)
		{
			pathPoints.push_back(
				DirectX::XMFLOAT2(
					begIntersection.intersectionPoint.x + vStep * diffStep,
					begIntersection.intersectionPoint.y
				)
			);
		}
		pathPoints.push_back(endIntersection.intersectionPoint);

		lastIntersection = endIntersection;
		reversed = !reversed;
	}

	// last point
	// second to last point
	std::reverse(segmentUp.begin(), segmentUp.end());
	//pathPoints.insert(pathPoints.end(), segmentUp.begin(), segmentUp.end());
	//pathPoints.push_back(allIntersections[allIntersections.size()-1].intersectionPoint);

	for (auto& pt : pathPoints)
	{
		pt.y -= 0.5;
	}

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> ExtractMultipleSegments(
	const std::vector<LineIntersectionData>& intersections,
	const std::vector<DirectX::XMFLOAT2>& originalLine)
{
	std::vector<DirectX::XMFLOAT2> mergedPoints;
	std::vector<std::vector<DirectX::XMFLOAT2>> segments;
	for (size_t index = 0; index < intersections.size(); index += 2)
	{
		std::vector<DirectX::XMFLOAT2> segment;

		auto intersectionBeg = intersections[index];
		auto intersectionEnd = intersections[index + 1];

		segment.push_back(intersectionBeg.intersectionPoint);

		auto extracted = ExtractSegmentFromOutline(
			originalLine,
			intersectionBeg.qLineIndex,
			intersectionEnd.qLineIndex + 1
		);

		segment.insert(
			segment.end(),
			extracted.begin(),
			extracted.end()
		);

		segment.push_back(intersectionEnd.intersectionPoint);
		segments.push_back(segment);
	}

	// Insert identifying points between segments
	for (size_t segmentId = 0; segmentId < segments.size(); segmentId++)
	{
		mergedPoints.insert(mergedPoints.end(), segments[segmentId].begin(), segments[segmentId].end());
	}

	return mergedPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareBody(
	const std::vector<DirectX::XMFLOAT2>& intersectionParamSideFin, 
	const std::vector<DirectX::XMFLOAT2>& intersectionParamBackFin, 
	const std::vector<DirectX::XMFLOAT2>& intersectionParamEye, 
	const std::vector<DirectX::XMFLOAT2>& intersectionHair1, 
	const std::vector<DirectX::XMFLOAT2>& intersectionHair2,
	const std::vector<DirectX::XMFLOAT2>& intersectionSideSpikes)
{
	float lowerParamBaseLineY = 0.25f;
	float upperParamBaseLineY = 0.75f;

#pragma region reorder Points
	std::vector<DirectX::XMFLOAT2> intersectionParamBackFinOrdered(
		intersectionParamBackFin.begin() + 87,
		intersectionParamBackFin.end()
	);
	intersectionParamBackFinOrdered.insert(
		intersectionParamBackFinOrdered.end(),
		intersectionParamBackFin.begin(),
		intersectionParamBackFin.begin() + 87
	);

	std::vector<DirectX::XMFLOAT2> intersectionParamHairOrdered1(
		intersectionHair1.begin() + 69,
		intersectionHair1.end()
	);
	intersectionParamHairOrdered1.insert(
		intersectionParamHairOrdered1.end(),
		intersectionHair1.begin(),
		intersectionHair1.begin() + 69
	);

	std::vector<DirectX::XMFLOAT2> intersectionParamHairOrdered2(
		intersectionHair2.begin() + 24,
		intersectionHair2.end()
	);
	intersectionParamHairOrdered2.insert(
		intersectionParamHairOrdered2.end(),
		intersectionHair2.begin(),
		intersectionHair2.begin() + 24
	);
#pragma endregion

	std::vector<DirectX::XMFLOAT2> lowerParamBaseLine {
			DirectX::XMFLOAT2(-0.5f, lowerParamBaseLineY),
			DirectX::XMFLOAT2(1.5f, lowerParamBaseLineY)
	};

	std::vector<DirectX::XMFLOAT2> upperParamBaseLine{
			DirectX::XMFLOAT2(-0.5f, upperParamBaseLineY),
			DirectX::XMFLOAT2(1.5f, upperParamBaseLineY)
	};

	auto backFinLowerEnd = IntersectCurves(lowerParamBaseLine, intersectionParamBackFinOrdered);
	auto backFinUpperEnd = IntersectCurves(upperParamBaseLine, intersectionParamBackFinOrdered);

	std::vector<DirectX::XMFLOAT2> backFinFinal;
#pragma region fillBackFin
	backFinFinal.push_back(backFinLowerEnd[0].intersectionPoint);
	auto backFinSegment = ExtractSegmentFromOutline(
		intersectionParamBackFinOrdered,
		backFinLowerEnd[0].pLineIndex,
		backFinUpperEnd[0].pLineIndex);
	backFinFinal.insert(backFinFinal.end(), backFinSegment.begin(), backFinSegment.end());
	backFinFinal.push_back(backFinUpperEnd[0].intersectionPoint);
#pragma endregion

#pragma region Calculate fronLine final
	std::vector<DirectX::XMFLOAT2> frontLine{
DirectX::XMFLOAT2(0.0f, 0.0f),
DirectX::XMFLOAT2(0.0f, 1.0f)
	};
	auto frontLineLowIntersection = IntersectCurves(lowerParamBaseLine, frontLine);
	auto frontLineUpperIntersection = IntersectCurves(upperParamBaseLine, frontLine);

#pragma endregion

	std::vector<DirectX::XMFLOAT2> frontLineFinal{
			frontLineLowIntersection[0].intersectionPoint,
			frontLineUpperIntersection[0].intersectionPoint,
	};

	// the parts of the outline are 
	// backFinFinal
	// frontLineFinal

	float vSteps = 35;
	float hSteps = 45;
#pragma region Create Outline
	// Calculate lower line 
	std::vector<DirectX::XMFLOAT2> lowerLineBase;
	auto lBegPt = frontLineFinal[0];
	auto lEndPt = backFinFinal[0];
	float hLowerStepValue = (lEndPt.x - lBegPt.x) / hSteps;

	// Calculate upper line
	std::vector<DirectX::XMFLOAT2> upperLineBase;
	auto uBegPt = frontLineFinal[frontLineFinal.size() - 1];
	auto uEndPt = backFinFinal[backFinFinal.size() - 1];
	float hUpperStepValue = (uEndPt.x - uBegPt.x) / hSteps;

	for (size_t step = 1; step < hSteps; step++)
	{
		lowerLineBase.push_back(
			DirectX::XMFLOAT2(
				lBegPt.x + hLowerStepValue * step,
				lBegPt.y
			)
		);
		upperLineBase.push_back(
			DirectX::XMFLOAT2(
				uBegPt.x + hUpperStepValue * step,
				uBegPt.y
			)
		);
	}


	auto hairOutlineCut1 = IntersectCurves(upperLineBase, intersectionParamHairOrdered1);
	auto hairOutlineCut2 = IntersectCurves(upperLineBase, intersectionParamHairOrdered2);

	// line: beg to hair1
	auto segBegToHair1 = ExtractSegmentFromOutline(upperLineBase, 0, hairOutlineCut1[1].qLineIndex);
	// hair: hair1
	auto segHair1 = ExtractSegmentFromOutline(intersectionParamHairOrdered1, hairOutlineCut1[1].pLineIndex, hairOutlineCut1[3].pLineIndex);
	// line: hair1 to hair2
	auto segHair1ToHair2 = ExtractSegmentFromOutline(upperLineBase, hairOutlineCut1[3].qLineIndex, hairOutlineCut2[0].qLineIndex);
	// hair: hair2
	auto segHair2 = ExtractSegmentFromOutline(intersectionParamHairOrdered2, hairOutlineCut2[0].pLineIndex, hairOutlineCut2[1].pLineIndex);
	// line: hair2 to end
	auto segHair2ToEnd = ExtractSegmentFromOutline(upperLineBase, hairOutlineCut2[1].qLineIndex, upperLineBase.size());


	std::vector<DirectX::XMFLOAT2> topOutlineFinal;
	topOutlineFinal.insert(topOutlineFinal.end(), segBegToHair1.begin(), segBegToHair1.end());
	std::reverse(segHair1.begin(), segHair1.end());
	topOutlineFinal.insert(topOutlineFinal.end(), segHair1.begin(), segHair1.end());
	topOutlineFinal.insert(topOutlineFinal.end(), segHair1ToHair2.begin(), segHair1ToHair2.end());
	std::reverse(segHair2.begin(), segHair2.end());
	topOutlineFinal.insert(topOutlineFinal.end(), segHair2.begin(), segHair2.end());
	topOutlineFinal.insert(topOutlineFinal.end(), segHair2ToEnd.begin(), segHair2ToEnd.end());

	std::vector<DirectX::XMFLOAT2> outlineFinal;
	outlineFinal.insert(outlineFinal.end(), lowerLineBase.begin(), lowerLineBase.end());
	outlineFinal.insert(outlineFinal.end(), backFinFinal.begin(), backFinFinal.end());

	std::reverse(topOutlineFinal.begin(), topOutlineFinal.end());
	outlineFinal.insert(outlineFinal.end(), topOutlineFinal.begin(), topOutlineFinal.end());

	std::reverse(frontLineFinal.begin(), frontLineFinal.end());
	outlineFinal.insert(outlineFinal.end(), frontLineFinal.begin(), frontLineFinal.end());
#pragma endregion

	float vStepValue = (upperParamBaseLineY - lowerParamBaseLineY) / vSteps;

	std::vector<DirectX::XMFLOAT2> pathPoints;
	// TODO move outline creation and std::reverse to the end?
	pathPoints.insert(pathPoints.end(), lowerLineBase.begin(), lowerLineBase.end());

	bool reversed = true;
	auto lastIntersectionPoint = backFinLowerEnd[0];

	for (size_t step = 1; step < vSteps; step++)
	{
		float currentY = lowerParamBaseLineY + step * vStepValue;
		// generate points in the middle
		std::vector<DirectX::XMFLOAT2> scanLine;

		for (size_t hStep = 0; hStep <= hSteps; hStep++)
		{
			float beg = -0.5f;
			float end = 1.5f;

			float hStepWidth = (end - beg) / hSteps;

			scanLine.push_back(
				DirectX::XMFLOAT2(
					beg + hStepWidth * hStep,
					currentY
				)
			);
		}

		// Intersect with front
		auto frontInt = IntersectCurves(scanLine, frontLineFinal);
		// Intersect with hair1
		auto hair1Int = IntersectCurves(scanLine, segHair1);
		// Intersect with hair2
		auto hair2Int = IntersectCurves(scanLine, segHair2);
		// Intersect with eye
		auto eyeInt = IntersectCurves(scanLine, intersectionParamEye);
		// Intersect with sideFin
		auto sideFinInt = IntersectCurves(scanLine, intersectionParamSideFin);
		// Intersect with back
		auto backFinInt = IntersectCurves(scanLine, backFinFinal);
		// Intersect with side spikes
		auto sideSpikeInt = IntersectCurves(scanLine, intersectionSideSpikes);

		// TODO split line with all these intersections
		auto wholeParameterLine = ExtractSegmentFromOutline(scanLine, frontInt[0].qLineIndex, backFinInt[0].qLineIndex);
		std::vector<LineIntersectionData> intersections;

		intersections.push_back(frontInt[0]);

		if (hair1Int.size())
		{
			intersections.push_back(hair1Int[0]);
			intersections.push_back(hair1Int[1]);
		}

		if (hair2Int.size())
		{
			intersections.push_back(hair2Int[0]);
			intersections.push_back(hair2Int[1]);
		}

		if (eyeInt.size())
		{
			intersections.push_back(eyeInt[0]);
			intersections.push_back(eyeInt[1]);
		}

		if (sideFinInt.size())
		{
			for (const auto& inter : sideFinInt)
			{
				intersections.push_back(inter);
			}
		}

		if (sideSpikeInt.size())
		{
			for (const auto& inter : sideSpikeInt)
			{
				intersections.push_back(inter);
			}
		}

		intersections.push_back(backFinInt[0]);
		auto mutlipleSegments = ExtractMultipleSegments(intersections, scanLine);

		LineIntersectionData startIntersection;
		std::vector<DirectX::XMFLOAT2> outlineSegment;
		if (reversed)
		{
			startIntersection = backFinInt[0];
			// Update frame segment
			outlineSegment = ExtractSegmentFromOutline(backFinFinal, lastIntersectionPoint.pLineIndex, startIntersection.pLineIndex);
			std::reverse(mutlipleSegments.begin(), mutlipleSegments.end());
			lastIntersectionPoint = frontInt[0];
		}
		else
		{
			startIntersection = frontInt[0];
			// Update frame segment
			//outlineSegment = ExtractSegmentFromOutline(frontLineFinal, lastIntersectionPoint.pLineIndex, startIntersection.pLineIndex);
			lastIntersectionPoint = backFinInt[0];
		}

		//pathPoints.insert(pathPoints.end(), outlineSegment.begin(), outlineSegment.end());
		pathPoints.push_back(startIntersection.intersectionPoint);
		pathPoints.insert(pathPoints.end(), mutlipleSegments.begin(), mutlipleSegments.end());
		pathPoints.push_back(lastIntersectionPoint.intersectionPoint);

		int x = 2;
		reversed = !reversed;
	}

	// reverse if reverse flag is present?

	pathPoints.insert(pathPoints.end(), topOutlineFinal.begin(), topOutlineFinal.end());

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareHair(
	const std::vector<DirectX::XMFLOAT2>& intersectionHair1,
	const std::vector<DirectX::XMFLOAT2>& intersectionHair2)
{
	float botCutoffParam = 1.0f / 6.0f;
	float topCutoffParam = botCutoffParam + 0.5f;

	std::vector<DirectX::XMFLOAT2> topParamLine
	{
		DirectX::XMFLOAT2(-0.5f, topCutoffParam),
		DirectX::XMFLOAT2(1.5f, topCutoffParam)
	};

	std::vector<DirectX::XMFLOAT2> botParamLine
	{
		DirectX::XMFLOAT2(-0.5f, botCutoffParam),
		DirectX::XMFLOAT2(1.5f, botCutoffParam)
	};

	auto hair1IntersectionTop = IntersectCurves(topParamLine, intersectionHair1);
	auto hair1IntersectionBot = IntersectCurves(botParamLine, intersectionHair1);

	auto hair2IntersectionTop = IntersectCurves(topParamLine, intersectionHair2);
	auto hair2IntersectionBot = IntersectCurves(botParamLine, intersectionHair2);

	float vSteps = 15;
	float hSteps = 15;

	float vStepWidth = (topCutoffParam - botCutoffParam) / vSteps;

	std::vector<DirectX::XMFLOAT2> pathPoints;
	// Todo add bottom line to pathpoints


	bool reversed = true;
	LineIntersectionData lastIntersection = hair2IntersectionBot[0];
	for (size_t vStep = 1; vStep < vSteps; vStep++)
	{
		float currentStepValue = botCutoffParam + vStep * vStepWidth;
		std::vector<DirectX::XMFLOAT2> scanline
		{
			DirectX::XMFLOAT2(-0.5f, currentStepValue),
			DirectX::XMFLOAT2(1.5f, currentStepValue)
		};

		auto scanlineHairIntersection1 = IntersectCurves(scanline, intersectionHair1);
		auto scanlineHairIntersection2 = IntersectCurves(scanline, intersectionHair2);

		// Subdivide the curve into
		LineIntersectionData beg;
		LineIntersectionData end;
		std::vector<DirectX::XMFLOAT2>& outlineline = scanline;
		if (reversed)
		{
			beg = scanlineHairIntersection2[0];
			end = scanlineHairIntersection1[0];
			outlineline = intersectionHair2;
		}
		else
		{
			beg = scanlineHairIntersection1[0];
			end = scanlineHairIntersection2[0];
			outlineline = intersectionHair1;
		}
		
		// Add part of the outline
		if (lastIntersection.pLineIndex != beg.pLineIndex)
		{
			auto outlineSegment = ExtractSegmentFromOutline(outlineline, lastIntersection.pLineIndex, beg.pLineIndex); // maybe +1?
			// TODO maybe reverse this segmenmt

			pathPoints.insert(pathPoints.end(), outlineSegment.begin(), outlineSegment.end());
		}		
		auto hStepWidth = (end.intersectionPoint.x - beg.intersectionPoint.x) / hSteps;
		
		for (size_t hStep = 0; hStep <= hSteps; hStep++)
		{
			pathPoints.push_back(
				DirectX::XMFLOAT2(
					beg.intersectionPoint.x + hStepWidth * hStep,
					currentStepValue
				)
			);
		}

		reversed = !reversed;
		lastIntersection = end;
	}

	// Todo add upper line to pathpoints
	// Todo add outline to pathpoints
	

	return pathPoints;
}
