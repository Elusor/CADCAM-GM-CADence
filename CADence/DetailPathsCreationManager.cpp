#include "DetailPathsCreationManager.h"
#include "ParametricOffsetSurface.h"
#include "IParametricSurface.h"
#include "Scene.h"
#include "PathUtils.h"

#include <fstream>
#include <sstream>
#include <iomanip>


DetailPathsCreationManager::DetailPathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene)
{
	m_intersectionFinder = intersectionFinder;
	m_scene = scene;
}

void DetailPathsCreationManager::CreateDetailPaths(PathModel* model)
{
	// TODO offset should be 0.45 not 0.09 lol
	float offset = -0.4F; // 0.08 + 0.01 eps

	auto modelObjects = model->GetModelObjects();
	auto bodyObject = dynamic_cast<IParametricSurface*>(modelObjects[0].lock().get()->m_object.get());
	auto eyeObject = dynamic_cast<IParametricSurface*>(modelObjects[1].lock().get()->m_object.get());
	auto hairObject = dynamic_cast<IParametricSurface*>(modelObjects[2].lock().get()->m_object.get());
	auto backFinObject = dynamic_cast<IParametricSurface*>(modelObjects[3].lock().get()->m_object.get());
	auto sideFinObject = dynamic_cast<IParametricSurface*>(modelObjects[4].lock().get()->m_object.get());
	auto sideSpikes = dynamic_cast<IParametricSurface*>(modelObjects[5].lock().get()->m_object.get());
	auto base = dynamic_cast<IParametricSurface*>(modelObjects[6].lock().get()->m_object.get());

	// Create offset surfaces for each object in the model
	auto bodyOffsetObject = ParametricOffsetSurface(bodyObject, offset);
	auto eyeOffsetObject = ParametricOffsetSurface(eyeObject, offset);
	auto hairOffsetObject = ParametricOffsetSurface(hairObject, offset);
	auto backFinOffsetObject = ParametricOffsetSurface(backFinObject, offset);
	auto sideFinOffsetObject = ParametricOffsetSurface(sideFinObject, offset);
	auto sideSpikesOffsetObject = ParametricOffsetSurface(sideSpikes, offset);
	auto baseOffsetObject = ParametricOffsetSurface(base, offset);

#pragma region Calculate intersections
	
	// Base intersections
	auto baseXhairIntersectionBot = m_intersectionFinder->FindIntersectionWithCursor(
		&baseOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(0.4, 4.1, -0.4)
	);
	//VisualizeCurve(&baseOffsetObject, baseXhairIntersectionBot.surfQParams);

	auto baseXhairIntersectionTop = m_intersectionFinder->FindIntersectionWithCursor(
		&baseOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(0.0f, 6.0f, -0.4)
	);

	//VisualizeCurve(&baseOffsetObject, baseXhairIntersectionTop.surfQParams);

	auto baseXbodyIntersectionTop = m_intersectionFinder->FindIntersectionWithCursor(
		&baseOffsetObject,
		&bodyOffsetObject,
		DirectX::XMFLOAT3(0.4, 2.7, -0.4)
	);
	//VisualizeCurve(&baseOffsetObject, baseXbodyIntersectionTop.surfQParams);

	auto baseXbodyIntersectionBot = m_intersectionFinder->FindIntersectionWithCursor(
		&baseOffsetObject,
		&bodyOffsetObject,
		DirectX::XMFLOAT3(0, -2.2, -0.4)
	);

	//VisualizeCurve(&baseOffsetObject, baseXbodyIntersectionBot.surfQParams);

	
	auto baseXtailIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&baseOffsetObject,
		&backFinOffsetObject,
		DirectX::XMFLOAT3(3.8, 2.7, -0.4)
	);
	//VisualizeCurve(&baseOffsetObject, baseXtailIntersection.surfQParams);

	//Intersect the models that should be intersected
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
	////VisualizeCurve(&bodyOffsetObject, bodyXeyeIntersection.surfQParams);

	
	auto backFinXhairIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&backFinOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(4.2f, 3.8f, 0.59f)
	);
	//VisualizeCurve(&backFinOffsetObject, backFinXhairIntersection.surfQParams);

	
	auto bodyXhairIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(-3.1f, 4.5f, -0.65f)
	);
	//VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection.surfQParams);
	
	auto leftUpper = DirectX::XMFLOAT3(-3.25f, 1.66f, -1.62f);
	auto leftLower = DirectX::XMFLOAT3(-3.95f, -0.34f, -2.0f);
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

	std::vector<DirectX::XMFLOAT2> spikePParams;
	std::vector<DirectX::XMFLOAT2> spikeQParams;
	auto spikePParamUp = bodyXsideSpikeLU.surfPParams;
	auto spikePParamBot = bodyXsideSpikeLL.surfPParams;

	auto spikeQParamUp = bodyXsideSpikeLU.surfQParams;
	auto spikeQParamBot = bodyXsideSpikeLL.surfQParams;

	spikePParams.insert(spikePParams.end(), spikePParamUp.begin(), spikePParamUp.end());
	spikePParams.insert(spikePParams.end(), spikePParamBot.begin(), spikePParamBot.end());
	spikePParams.push_back(spikePParams[0]);

	spikeQParams.insert(spikeQParams.end(), spikeQParamUp.begin(), spikeQParamUp.end());
	spikeQParams.insert(spikeQParams.end(), spikeQParamBot.begin(), spikeQParamBot.end());
	spikeQParams.push_back(spikeQParams[0]);

	auto spikeParamsPNormalized = NormalizeParameters(spikePParams, sideSpikes);
	auto c0PathsNorm = PrepareC0(spikeParamsPNormalized);
	auto spikeParamsDenormalized = DenormalizeParameters(c0PathsNorm, sideSpikes);
	auto c0WorldPoints = VisualizeCurve(&sideSpikesOffsetObject, spikeParamsDenormalized);

#pragma endregion
#pragma region body

	auto normalizedBackFin = NormalizeParameters(bodyXbackFinIntersection.surfPParams, backFinObject);
	auto backFinPathPointsParams = PrepareBackFin(normalizedBackFin);
	auto denormalizedBackFinPathPointParams = DenormalizeParameters(backFinPathPointsParams, backFinObject);
	auto backFinEndPath = VisualizeCurve(&backFinOffsetObject, denormalizedBackFinPathPointParams);

	auto normalizedSideFin = NormalizeParameters(bodyXsideFinIntersection.surfPParams, sideFinObject);
	auto sideFinPathPointsParams = PrepareSideFin(normalizedSideFin);
	auto denormalizedSideFinPathPointParams = DenormalizeParameters(sideFinPathPointsParams, sideFinObject);
	auto sideFinEndPath = VisualizeCurve(&sideFinOffsetObject, denormalizedSideFinPathPointParams);

	auto normalizedEye = NormalizeParameters(bodyXeyeIntersection.surfPParams, eyeObject);
	auto eyePathPointsParams = PrepareEye(normalizedEye);
	auto denomarlizedEyePathPointParams = DenormalizeParameters(eyePathPointsParams, eyeObject);
	auto eyeEndPath = VisualizeCurve(&eyeOffsetObject, denomarlizedEyePathPointParams);

	auto normalizeBodySideFin = NormalizeParameters(bodyXsideFinIntersection.surfQParams, bodyObject);
	auto normalizeBodyBackFin = NormalizeParameters(bodyXbackFinIntersection.surfQParams, bodyObject);
	auto normalizeBodyEye = NormalizeParameters(bodyXeyeIntersection.surfQParams, bodyObject);
	auto normalizeBodyHair = NormalizeParameters(backFinXhairIntersection.surfQParams, bodyObject);
	auto normalizeBodyHair2 = NormalizeParameters(bodyXhairIntersection.surfQParams, bodyObject);

	auto normalizeBodySpikes = NormalizeParameters(spikeQParams, bodyObject);

	auto normalizedBodyCutoffTop = NormalizeParameters(baseXbodyIntersectionTop.surfPParams, bodyObject);
	auto normalizedBodyCutoffBot = NormalizeParameters(baseXbodyIntersectionBot.surfPParams, bodyObject);

	auto bodyPathPointsParams = PrepareBody(
		normalizedBodyCutoffTop,
		normalizedBodyCutoffBot,
		normalizeBodySideFin,
		normalizeBodyBackFin,
		normalizeBodyEye,
		std::vector<DirectX::XMFLOAT2>(),
		normalizeBodyHair2,
		normalizeBodySpikes
	);
	auto denormalizedBodyPathPointParams = DenormalizeParameters(bodyPathPointsParams, bodyObject);
	auto bodyEndPath = VisualizeCurve(&bodyOffsetObject, denormalizedBodyPathPointParams);
#pragma endregion


	auto normalizeHair = NormalizeParameters(bodyXhairIntersection.surfPParams, hairObject);
	auto normalizeHair2 = NormalizeParameters(backFinXhairIntersection.surfPParams, hairObject);
	auto normalizedHairCutoffTop = NormalizeParameters(baseXhairIntersectionTop.surfPParams, hairObject);
	auto normalizedHairCutoffBot = NormalizeParameters(baseXhairIntersectionBot.surfPParams, hairObject);
	auto hairPathPointParams = PrepareHair(
		normalizedHairCutoffTop,
		normalizedHairCutoffBot,
		normalizeHair,
		normalizeHair2
	);
	auto denormalizedHairPathPointParams = DenormalizeParameters(hairPathPointParams, hairObject);
	auto hairEndPath = VisualizeCurve(&hairOffsetObject, denormalizedHairPathPointParams);

#pragma region hole

	auto intBodyXBack = IntersectCurves(baseXbodyIntersectionTop.surfQParams, baseXtailIntersection.surfQParams);
	auto intBodyXHair = IntersectCurves(baseXbodyIntersectionTop.surfQParams, baseXhairIntersectionBot.surfQParams);
	auto intHairXBack = IntersectCurves(baseXhairIntersectionBot.surfQParams, baseXtailIntersection.surfQParams);

	auto backSegment = ExtractSegmentFromOutline(baseXtailIntersection.surfQParams, intBodyXBack[0].pLineIndex, intHairXBack[0].pLineIndex + 1);
	auto bodySegment = ExtractSegmentFromOutline(baseXbodyIntersectionTop.surfQParams, intBodyXHair[0].qLineIndex, intBodyXBack[0].qLineIndex + 1);
	auto hairSegment = ExtractSegmentFromOutline(baseXhairIntersectionBot.surfQParams, intHairXBack[0].qLineIndex, intBodyXHair[0].pLineIndex + 1);

	backSegment.push_back(intHairXBack[0].intersectionPoint);
	bodySegment.push_back(intBodyXBack[0].intersectionPoint);
	hairSegment.push_back(intBodyXHair[0].intersectionPoint);

	std::vector<DirectX::XMFLOAT2> holepath = PrepareHole(
		hairSegment,
		bodySegment,
		backSegment);
	// add outline here?
	auto holePathWorld = VisualizeCurve(&baseOffsetObject, holepath);
#pragma endregion

#pragma region Visualize intersections
	/*VisualizeCurve(&bodyOffsetObject, bodyXbackFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXsideFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXeyeIntersection);
	VisualizeCurve(&bodyOffsetObject, backFinXhairIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection);
	*/
	//VisualizeCurve(&bodyOffsetObject, spikePParams);
#pragma endregion

	float safeHeight = 6.5f;
	std::vector<DirectX::XMFLOAT3> wholePath;
	wholePath.push_back({ 0.0f, 0.0f, -safeHeight });
	wholePath.push_back({ 0.0f, 0.0f, -safeHeight }); //dummy pt 
	wholePath.insert(wholePath.end(), backFinEndPath.begin(), backFinEndPath.end());
	wholePath.push_back({ -1.0f, -1.0f, -1.0f });
	wholePath.insert(wholePath.end(), bodyEndPath.begin(), bodyEndPath.end());
	wholePath.push_back({ -1.0f, -1.0f, -1.0f });
	wholePath.insert(wholePath.end(), eyeEndPath.begin(), eyeEndPath.end());
	wholePath.push_back({ -1.0f, -1.0f, -1.0f });
	wholePath.insert(wholePath.end(), sideFinEndPath.begin(), sideFinEndPath.end());
	wholePath.push_back({ -1.0f, -1.0f, -1.0f });
	wholePath.insert(wholePath.end(), hairEndPath.begin(), hairEndPath.end());
	wholePath.push_back({ -1.0f, -1.0f, -1.0f });
	wholePath.insert(wholePath.end(), holePathWorld.begin(), holePathWorld.end());
	wholePath.push_back({ -1.0f, -1.0f, -1.0f });
	wholePath.insert(wholePath.end(), c0WorldPoints.begin(), c0WorldPoints.end());
	wholePath.push_back({ 0.0f, 0.0f, -safeHeight });
	wholePath.push_back({ 0.0f, 0.0f, -safeHeight });

	auto endPath = RemoveSpecialPoints(wholePath);
	endPath[1] = endPath[2];
	endPath[1].z = -safeHeight;

	endPath[endPath.size() - 2] = endPath[endPath.size() - 3];
	endPath[endPath.size() - 2].z = -safeHeight;

	// Export path
	SavePathToFile(endPath, "3_detailed.k08");
}

std::vector<DirectX::XMFLOAT3> DetailPathsCreationManager::VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params)
{
	std::vector<DirectX::XMFLOAT3> endPoints;
	float safeHeight = 5.0f;

	std::vector<ObjectRef> points;
	for (size_t index = 0; index < params.size(); index++)
	{
		auto param = params[index];
		if(param.x == -1.0f && param.y == -1.0f)
		{
			auto param1 = params[index - 1];
			auto param2 = params[index + 1];

			auto pos1 = surface->GetPoint(param1.x, param1.y);
			auto pos2 = surface->GetPoint(param2.x, param2.y);
			Transform ptTransform;
			pos1.z = -safeHeight;
			ptTransform.SetPosition(pos1);			
			auto point = m_factory.CreatePoint(ptTransform);
			point->SetIsVirtual(true);
			m_scene->AttachObject(point);
			points.push_back(point);
			endPoints.push_back(pos1);

			pos2.z = -safeHeight;
			ptTransform.SetPosition(pos2);
			point = m_factory.CreatePoint(ptTransform);
			point->SetIsVirtual(true);
			m_scene->AttachObject(point);
			points.push_back(point);
			endPoints.push_back(pos2);
		}
		else
		{
			auto pos = surface->GetPoint(param.x, param.y);
			Transform ptTransform;
			ptTransform.SetPosition(pos);
			ptTransform.Translate(0, 0, 0);
			auto point = m_factory.CreatePoint(ptTransform);
			point->SetIsVirtual(true);
			m_scene->AttachObject(point);
			points.push_back(point);
			endPoints.push_back(pos);
		}
	}

	m_scene->AttachObject(m_factory.CreateInterpolBezierCurveC2(points));

	return endPoints;
}

bool DetailPathsCreationManager::SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions, std::string name)
{
	//Select file 
	std::ofstream myfile;
	myfile.open(name);

	//Reset instruction counter
	m_instructionCounter = 3;
	m_blockBaseHeight = 1.5f;
	float millRadius = 0.4f;
	if (myfile.is_open())
	{

		//// Use milimeters
		//myfile << "%G71\n";
		//// Opening sequence
		//PushInstructionToFile(myfile, "G40G90");
		//// Rotation speed and direction
		//PushInstructionToFile(myfile, "S10000M03");
		//// Mill movement speed
		//PushInstructionToFile(myfile, "F15000");
	
		for (int i = 0; i < positions.size(); i++)
		{
			SimpleMath::Vector3 pos = positions[i];
			pos.z = abs(pos.z - m_blockBaseHeight + millRadius) < m_blockBaseHeight? m_blockBaseHeight : abs(pos.z - m_blockBaseHeight + millRadius);
			auto mmPt = ConvertToMilimeters(pos);
			PushInstructionToFile(myfile, PrepareMoveInstruction(mmPt), i == positions.size() - 1);
		}

		//// Disable the rotation
		//PushInstructionToFile(myfile, "M05");
		//// End the program
		//PushInstructionToFile(myfile, "M30");

		myfile.close();
		return true;
	}
	else
	{
		return false;
	}
}

void DetailPathsCreationManager::PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr)
{
	file << "N" << std::to_string(m_instructionCounter) << instructionText;

	if (lastInstr == false)
	{
		file << "\n";
	}

	m_instructionCounter++;
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
		if (!(pt.x == -1.0f && pt.y == -1.0f))
		{
			auto params = surface->GetWrappedParams(pt.x * maxParams.u, pt.y * maxParams.v);
			result.push_back(params.GetVector());
		}
		else
		{
			result.push_back(pt);
		}
		
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

	std::vector<DirectX::XMFLOAT2> pathPoints;
	pathPoints.insert(pathPoints.end(), frontLine.begin(), frontLine.end());

	float stepsVertical = 50;
	float stepsHorizontal = 30;

	bool reversed = true;
	LineIntersectionData lastIntersection = endIntersection[0];

	float begH = max(startIntersection[0].intersectionPoint.x, endIntersection[0].intersectionPoint.x);
	float endH = maxParam;
	float stepHWidth = (endH - begH) / stepsHorizontal;

	for (size_t hStep = 1; hStep < stepsHorizontal; hStep++)
	{
		std::vector<DirectX::XMFLOAT2> scanline;
		float curx = begH + hStep * stepHWidth;
		float begV = 0.0f;
		float endV = 1.0f;
		float stepVWidth = (endV - begV) / stepsVertical;
		for (size_t vStep = 0; vStep <= stepsVertical; vStep++)
		{
			scanline.push_back({
				curx,
				begV + stepVWidth * vStep
				});
		}

		auto startIntersection = IntersectCurves(scanline, lineStart);
		auto endIntersection = IntersectCurves(scanline, lineEnd);

		LineIntersectionData beg;
		LineIntersectionData end;
		if (reversed)
		{
			beg = endIntersection[0];
			end = startIntersection[0];
		}
		else
		{
			beg = startIntersection[0];
			end = endIntersection[0];
		}

		// TODO add the extraction from the outline

		auto segment = ExtractSegmentFromOutline(scanline, beg.qLineIndex, end.qLineIndex + 1);
		if (reversed)
		{
			std::reverse(segment.begin(), segment.end());
		}


		pathPoints.push_back(beg.intersectionPoint);
		pathPoints.insert(pathPoints.end(), segment.begin(), segment.end());
		pathPoints.push_back(end.intersectionPoint);

		lastIntersection = end;
		reversed = !reversed;
	}

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareSideFin(const std::vector<DirectX::XMFLOAT2>& intersectionParams)
{
	float minHParam = 0.0f;
	float maxHParam = 1.0f;

	size_t steps = 40;
	size_t vSteps = 31;
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
	//std::reverse(segmentUp.begin(), segmentUp.end());
	pathPoints.insert(pathPoints.end(), segmentUp.begin(), segmentUp.end());

	// Outline 
	std::vector<DirectX::XMFLOAT2> outlinePoints;
	auto seg1 = ExtractSegmentFromOutline(intersectionParams, 0, allIntersections[allIntersections.size() - 1].pLineIndex);
	auto seg2 = ExtractSegmentFromOutline(intersectionParams, allIntersections[allIntersections.size() - 2].pLineIndex, intersectionParams.size());
	std::reverse(seg1.begin(), seg1.end());
	std::reverse(seg2.begin(), seg2.end());

	outlinePoints.insert(outlinePoints.end(), seg1.begin(), seg1.end());
	outlinePoints.insert(outlinePoints.end(), seg2.begin(), seg2.end());
	// allIntersections[allIntersections.size() - 1].pLineIndex,
	// allIntersections[allIntersections.size() - 2].pLineIndex

	//pathPoints.insert(pathPoints.end(), outlinePoints.begin(), outlinePoints.end());
	//pathPoints.insert(intersectionParams);
	//pathPoints.push_back(allIntersections[allIntersections.size()-1].intersectionPoint);

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareEye(const std::vector<DirectX::XMFLOAT2>& intersectionParams)
{
	auto fistHardcode1Index = 26;
	auto secondHardcode1Index = 59;

	auto segment1 = std::vector<DirectX::XMFLOAT2>(intersectionParams.begin(), intersectionParams.begin() + fistHardcode1Index);
	auto segment2 = std::vector<DirectX::XMFLOAT2>(intersectionParams.begin() + fistHardcode1Index, intersectionParams.begin() + secondHardcode1Index);
	auto segment3 = std::vector<DirectX::XMFLOAT2>(intersectionParams.begin() + secondHardcode1Index, intersectionParams.end());

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

	size_t steps = 35;
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

	/*std::swap(
		allIntersections[allIntersections.size() - 1],
		allIntersections[allIntersections.size() - 2]
	);*/

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
		if (!index)// || index == (allIntersections.size() / 2) - 1)
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
		if (segmentId != segments.size() - 1)
		{
			mergedPoints.push_back({ -1.0f, -1.0f });
		}
	}

	return mergedPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareBody(
	const std::vector<DirectX::XMFLOAT2>& cutoffTop,
	const std::vector<DirectX::XMFLOAT2>& cutoffBot,
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
	size_t paramBackFinZeroOffset = 103;
	std::vector<DirectX::XMFLOAT2> intersectionParamBackFinOrdered(
		intersectionParamBackFin.begin() + paramBackFinZeroOffset,
		intersectionParamBackFin.end()
	);
	intersectionParamBackFinOrdered.insert(
		intersectionParamBackFinOrdered.end(),
		intersectionParamBackFin.begin(),
		intersectionParamBackFin.begin() + paramBackFinZeroOffset
	);
	
	size_t hairParamAboveCutoffOffset = 104;
	std::vector<DirectX::XMFLOAT2> intersectionParamHairOrdered(
		intersectionHair2.begin() + hairParamAboveCutoffOffset,
		intersectionHair2.end()
	);
	intersectionParamHairOrdered.insert(
		intersectionParamHairOrdered.end(),
		intersectionHair2.begin(),
		intersectionHair2.begin() + hairParamAboveCutoffOffset
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
		backFinUpperEnd[0].pLineIndex + 1);
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

	float vSteps = 100;
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


	auto hairOutlineCut1 = IntersectCurves(upperLineBase, intersectionParamHairOrdered);

	// line: beg to hair1
	auto segBegToHair1 = ExtractSegmentFromOutline(upperLineBase, 0, hairOutlineCut1[0].qLineIndex);
	// hair: hair1
	auto segHair1 = ExtractSegmentFromOutline(intersectionParamHairOrdered, hairOutlineCut1[0].pLineIndex, hairOutlineCut1[1].pLineIndex);	
	// line: hair2 to end
	auto segHair2ToEnd = ExtractSegmentFromOutline(upperLineBase, hairOutlineCut1[1].qLineIndex, upperLineBase.size());


	std::vector<DirectX::XMFLOAT2> topOutlineFinal;
	topOutlineFinal.insert(topOutlineFinal.end(), segBegToHair1.begin(), segBegToHair1.end());
	std::reverse(segHair1.begin(), segHair1.end());
	topOutlineFinal.insert(topOutlineFinal.end(), segHair1.begin(), segHair1.end());	
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
			//outlineSegment = ExtractSegmentFromOutline(backFinFinal, lastIntersectionPoint.pLineIndex, startIntersection.pLineIndex);
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
	std::reverse(topOutlineFinal.begin(), topOutlineFinal.end());
	pathPoints.insert(pathPoints.end(), topOutlineFinal.begin(), topOutlineFinal.end());

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareHair(
	const std::vector<DirectX::XMFLOAT2>& cutoffTop,
	const std::vector<DirectX::XMFLOAT2>& cutoffBot,
	const std::vector<DirectX::XMFLOAT2>& intersectionHair1,
	const std::vector<DirectX::XMFLOAT2>& intersectionHair2)
{
	std::vector<DirectX::XMFLOAT2> reorderedIntersectionHair1(intersectionHair1.begin() + 106, intersectionHair1.end());
	reorderedIntersectionHair1.insert(reorderedIntersectionHair1.end(), intersectionHair1.begin(), intersectionHair1.begin() + 106);
	std::vector<DirectX::XMFLOAT2> reorderedIntersectionHair2(intersectionHair2.begin() + 9, intersectionHair2.end());
	reorderedIntersectionHair2.insert(reorderedIntersectionHair2.end(), intersectionHair2.begin(), intersectionHair2.begin() + 9);
	std::reverse(reorderedIntersectionHair2.begin(), reorderedIntersectionHair2.end());

	

	float vSteps = 32;
	float hSteps = 40;

	std::vector<DirectX::XMFLOAT2> topParamLine = cutoffTop;

	std::vector<DirectX::XMFLOAT2> botParamLine = cutoffBot;
	std::reverse(botParamLine.begin(), botParamLine.end());

	float minY = 1.0f;
	float maxY = 0.0f;
	for (auto pt : topParamLine)
	{
		minY = minY < pt.y ? minY : pt.y;
		maxY = maxY > pt.y ? maxY : pt.y;
	}

	for (auto pt : botParamLine)
	{
		minY = minY < pt.y ? minY : pt.y;
		maxY = maxY > pt.y ? maxY : pt.y;
	}

	float topCutoffParam = maxY;
	float botCutoffParam = minY;

	float begParamLine = -0.5f;
	float endParamLine = 1.5f;

	float paramLineSteps = hSteps * 2;
	float paramLineStep = (endParamLine - begParamLine) / paramLineSteps;
	/*for (size_t stepV = 0; stepV <= paramLineSteps; stepV++)
	{
		topParamLine.push_back({
			begParamLine + paramLineStep * stepV,
			topCutoffParam
			});
		botParamLine.push_back({
			begParamLine + paramLineStep * stepV,
			botCutoffParam
			});
	}*/

	auto hair1IntersectionTop = IntersectCurves(topParamLine, reorderedIntersectionHair1);
	auto hair1IntersectionBot = IntersectCurves(botParamLine, reorderedIntersectionHair1);

	auto hair2IntersectionTop = IntersectCurves(topParamLine, reorderedIntersectionHair2);
	auto hair2IntersectionBot = IntersectCurves(botParamLine, reorderedIntersectionHair2);


	float vStepWidth = (topCutoffParam - botCutoffParam) / vSteps;

	std::vector<DirectX::XMFLOAT2> pathPoints;

	std::vector<DirectX::XMFLOAT2> topOutlinePart;
	std::vector<DirectX::XMFLOAT2> botOutlinePart;
	
	auto topOutlineSegment = ExtractSegmentFromOutline(topParamLine, hair1IntersectionTop[0].qLineIndex, hair2IntersectionTop[0].qLineIndex + 1);
	auto botOutlineSegment = ExtractSegmentFromOutline(botParamLine, hair1IntersectionBot[0].qLineIndex, hair2IntersectionBot[0].qLineIndex + 1);

	botOutlinePart.push_back(hair1IntersectionBot[0].intersectionPoint);
	botOutlinePart.insert(botOutlinePart.end(), botOutlineSegment.begin(), botOutlineSegment.end());
	botOutlinePart.push_back(hair2IntersectionBot[0].intersectionPoint);

	topOutlinePart.push_back(hair1IntersectionTop[0].intersectionPoint);
	topOutlinePart.insert(topOutlinePart.end(), topOutlineSegment.begin(), topOutlineSegment.end());
	topOutlinePart.push_back(hair2IntersectionTop[0].intersectionPoint);
	
	
	// Todo add bottom line to pathpoints
	pathPoints.insert(pathPoints.end(), botOutlinePart.begin(), botOutlinePart.end());

	bool reversed = true;
	LineIntersectionData lastIntersection = hair2IntersectionBot[0];
	for (size_t vStep = 1; vStep < vSteps; vStep++)
	{
		float currentStepValue = botCutoffParam + vStep * vStepWidth;
		std::vector<DirectX::XMFLOAT2> scanline;
		float scanLineBeg = 0.0f;
		float scanLineEnd = 1.0f;
		float scanLineStep = (scanLineEnd - scanLineBeg) / hSteps;
		for (size_t stepH = 0; stepH <= hSteps; stepH++)
		{
			scanline.push_back({
				scanLineBeg + scanLineStep * stepH,
				currentStepValue
				});
		}
		auto scanlineHairIntersection1 = IntersectCurves(scanline, reorderedIntersectionHair1);
		auto scanlineHairIntersection2 = IntersectCurves(scanline, reorderedIntersectionHair2);

		auto scanLineBotIntersection = IntersectCurves(scanline, botOutlinePart);
		auto scanLineTopIntersection = IntersectCurves(scanline, topOutlinePart);

		if(scanLineBotIntersection.size())
		{
			int x = scanLineBotIntersection.size();
		}
		if (scanLineTopIntersection.size())
		{
			int x = scanLineTopIntersection.size();
		}

		// Subdivide the curve into
		LineIntersectionData beg;
		LineIntersectionData end;
		std::vector<DirectX::XMFLOAT2> outlineline;
		if (reversed)
		{
			beg = scanlineHairIntersection2[0];
			end = scanlineHairIntersection1[0];
			outlineline = reorderedIntersectionHair2;
		}
		else
		{
			beg = scanlineHairIntersection1[0];
			end = scanlineHairIntersection2[0];
			outlineline = reorderedIntersectionHair1;
		}

		// Add part of the outline
		if (lastIntersection.pLineIndex != beg.pLineIndex)
		{
			auto outlineSegment = ExtractSegmentFromOutline(outlineline, lastIntersection.pLineIndex, beg.pLineIndex); // maybe +1?
			// TODO maybe reverse this segmenmt

			pathPoints.insert(pathPoints.end(), outlineSegment.begin(), outlineSegment.end());
		}		
		auto hStepWidth = (end.intersectionPoint.x - beg.intersectionPoint.x) / hSteps;
		
		auto scanLineSegment = ExtractSegmentFromOutline(scanline, beg.qLineIndex, end.qLineIndex + 1);
		if (reversed)
		{
			std::reverse(scanLineSegment.begin(), scanLineSegment.end());
		}
		pathPoints.push_back(beg.intersectionPoint);
		pathPoints.insert(pathPoints.end(), scanLineSegment.begin(), scanLineSegment.end());
		pathPoints.push_back(end.intersectionPoint);
		/*
		for (size_t hStep = 0; hStep <= hSteps; hStep++)
		{
			pathPoints.push_back(
				DirectX::XMFLOAT2(
					beg.intersectionPoint.x + hStepWidth * hStep,
					currentStepValue
				)
			);
		}*/

		reversed = !reversed;
		lastIntersection = end;
	}

	// Todo add upper line to pathpoints
	// Todo add outline to pathpoints
	auto segment = ExtractSegmentFromOutline(reorderedIntersectionHair1, lastIntersection.pLineIndex, hair1IntersectionTop[0].pLineIndex + 1);
	pathPoints.insert(pathPoints.end(), segment.begin(), segment.end());
	// Todo add bottom line to pathpoints
	pathPoints.insert(pathPoints.end(), topOutlinePart.begin(), topOutlinePart.end());

	std::vector<DirectX::XMFLOAT2> outlinePoints;
	auto backSegment = ExtractSegmentFromOutline(reorderedIntersectionHair2, hair2IntersectionBot[0].pLineIndex, hair2IntersectionTop[0].pLineIndex + 1);
	auto frontSegment = ExtractSegmentFromOutline(reorderedIntersectionHair1, hair1IntersectionBot[0].pLineIndex, hair1IntersectionTop[0].pLineIndex + 1);

	std::reverse(backSegment.begin(), backSegment.end());

	outlinePoints.insert(outlinePoints.end(), backSegment.begin(), backSegment.end());
	outlinePoints.push_back({ -1.0f, -1.0f });
	outlinePoints.insert(outlinePoints.end(), frontSegment.begin(), frontSegment.end());

	// Add outline to the path
	pathPoints.insert(pathPoints.end(), outlinePoints.begin(), outlinePoints.end());

	return pathPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareHole(
	const std::vector<DirectX::XMFLOAT2>& intersectionHair, 
	const std::vector<DirectX::XMFLOAT2>& intersectionBody,
	const std::vector<DirectX::XMFLOAT2>& intersectionBackFin)
{
	auto topLine = intersectionHair;
	std::reverse(topLine.begin(), topLine.end());
	auto firstPt = topLine[0];

	std::vector<DirectX::XMFLOAT2> botLine{ firstPt };
	botLine.insert(botLine.end(), intersectionBody.begin(), intersectionBody.end());
	botLine.insert(botLine.end(), intersectionBackFin.begin(), intersectionBackFin.end());	
	auto lastPt = botLine[botLine.size() - 1];
	topLine.push_back(lastPt);

	float begParamX = firstPt.x;
	float endParamX = lastPt.x;

	float vSteps = 40;
	float vStepWidth = (endParamX - begParamX) / vSteps;

	LineIntersectionData lastIntersection;
	lastIntersection.pLineIndex = 0;
	lastIntersection.qLineIndex = 0;
	lastIntersection.intersectionPoint = firstPt;
	bool reversed = false;
	std::vector<DirectX::XMFLOAT2> endPoints;
	endPoints.push_back(firstPt);
	for (size_t vStepIdx = 1; vStepIdx < vSteps; vStepIdx++)
	{
		float curXParam = begParamX + vStepWidth * vStepIdx;
		std::vector<DirectX::XMFLOAT2> scanline
		{
			{curXParam, 0.0f},
			{curXParam, 1.0f}
		};

		auto topIntersection = IntersectCurves(scanline, topLine);
		auto botIntersection = IntersectCurves(scanline, botLine);
		LineIntersectionData beg;
		LineIntersectionData end;
		std::vector<DirectX::XMFLOAT2> segment;

		if (reversed)
		{
			beg = topIntersection[0];
			end = botIntersection[0];

			segment = ExtractSegmentFromOutline(topLine, lastIntersection.pLineIndex, beg.pLineIndex);
		}
		else
		{
			beg = botIntersection[0];
			end = topIntersection[0];

			segment = ExtractSegmentFromOutline(botLine, lastIntersection.pLineIndex, beg.pLineIndex);
		}

		endPoints.insert(endPoints.end(), segment.begin(), segment.end());
		endPoints.push_back(beg.intersectionPoint);
		endPoints.push_back(end.intersectionPoint);

		lastIntersection = end;
		reversed = !reversed;		
	}



	endPoints.push_back(lastPt);


	return endPoints;
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareC0(const std::vector<DirectX::XMFLOAT2>& intersectionCurve)
{
	float magicParameter = 0.75f;
	float magicEps = 0.000001f;

	float topHeight = magicParameter + magicEps;
	float botHeight = magicParameter - magicEps;

	float hSteps = 60;
	float endParam = 1;
	float begParam = 0; 
	float horizontalStepWidth = (endParam - begParam) / hSteps;

	std::vector<DirectX::XMFLOAT2> scanlineTop;
	std::vector<DirectX::XMFLOAT2> scanlineBot;
	for (size_t step = 0; step <= hSteps; step++)
	{
		float curX = begParam + horizontalStepWidth * step;
		scanlineTop.push_back({ curX, topHeight });
		scanlineBot.push_back({ curX, botHeight });
	}
	
	
	auto botCurveInt = IntersectCurves(scanlineBot, intersectionCurve);
	auto botCurve = ExtractSegmentFromOutline(intersectionCurve, botCurveInt[0].pLineIndex, botCurveInt[1].pLineIndex + 1);
	std::reverse(botCurve.begin(), botCurve.end());
	auto botLine = ExtractSegmentFromOutline(scanlineBot, botCurveInt[0].qLineIndex, botCurveInt[1].qLineIndex + 1);
	botCurve.insert(botCurve.begin(), botCurveInt[0].intersectionPoint);
	botLine.insert(botLine.begin(), botCurveInt[0].intersectionPoint);
	botCurve.push_back(botCurveInt[1].intersectionPoint);
	botLine.push_back(botCurveInt[1].intersectionPoint);

	auto topCurveInt = IntersectCurves(scanlineTop, intersectionCurve);
	auto topCurve = ExtractSegmentFromOutline(intersectionCurve, topCurveInt[0].pLineIndex, intersectionCurve.size());
	topCurve.insert(topCurve.end(), intersectionCurve.begin(), intersectionCurve.begin() + topCurveInt[1].pLineIndex + 1);
	auto topLine = ExtractSegmentFromOutline(scanlineTop, topCurveInt[0].qLineIndex, topCurveInt[1].qLineIndex + 1);
	topCurve.insert(topCurve.begin(), topCurveInt[0].intersectionPoint);
	topLine.insert(topLine.begin(), topCurveInt[0].intersectionPoint);
	topCurve.push_back(topCurveInt[1].intersectionPoint);
	topLine.push_back(topCurveInt[1].intersectionPoint);

	float eps = 0.02f;
	float scanningHSteps = 60;

	std::vector<DirectX::XMFLOAT2> path;
	std::vector<DirectX::XMFLOAT2> curve = botCurve;
	std::vector<DirectX::XMFLOAT2> line = botLine;
	float searchBegX = line[0].x;
	float searchEndX = line[line.size() - 1].x;
	float searchStep = (searchEndX - searchBegX) / scanningHSteps;
	bool reversed = false;
	LineIntersectionData lastIntersection = botCurveInt[0];
	// Do search for a given x
	for (size_t xStep = 1; xStep < scanningHSteps; xStep++)
	{
		float currentX = searchBegX + xStep * searchStep;
		if (currentX <0.50f - eps || currentX > 0.50f + eps)
		{
			

			std::vector<DirectX::XMFLOAT2> verticalScanline;
			float maxY = 1.0f;
			float minY = 0.5f;
			float verticalSteps = 60;
			float stepY = (maxY - minY) / verticalSteps;

			for (size_t vertStep = 0; vertStep <= verticalSteps; vertStep++)
			{
				float currentY = minY + stepY * vertStep;
				verticalScanline.push_back({ currentX, currentY });
			}

			auto lineInt = IntersectCurves(verticalScanline, line);
			auto curveInt = IntersectCurves(verticalScanline, curve);

			LineIntersectionData beg;
			LineIntersectionData end;
			if (reversed)
			{
				beg = lineInt[0];
				end = curveInt[0];
			}
			else
			{
				beg = curveInt[0];
				end = lineInt[0];
			}


			std::vector<DirectX::XMFLOAT2> segment;
			if (beg.qLineIndex != end.qLineIndex)
			{
				segment = ExtractSegmentFromOutline(verticalScanline, beg.qLineIndex, end.qLineIndex);
				if (reversed)
				{
					std::reverse(segment.begin(), segment.end());
				}
			}

			path.push_back(lastIntersection.intersectionPoint);
			path.push_back(beg.intersectionPoint);
			path.insert(path.end(), segment.begin(), segment.end());
			path.push_back(end.intersectionPoint);

			lastIntersection = end;
			reversed = !reversed;
		}
	}
	path.push_back(line[line.size() - 1]);
	path.push_back({ -1.0f, -1.0f });

	curve = topCurve;
	line = topLine;
	searchBegX = line[0].x;
	searchEndX = line[line.size() - 1].x;
	searchStep = (searchEndX - searchBegX) / scanningHSteps;
	reversed = true;
	lastIntersection = topCurveInt[0];
	// Do search for a given x
	for (size_t xStep = 1; xStep < scanningHSteps; xStep++)
	{
		float currentX = searchBegX + xStep * searchStep;

		if (currentX < 0.50f - eps || currentX > 0.50f + eps)
		{
			std::vector<DirectX::XMFLOAT2> verticalScanline;
			float maxY = 1.0f;
			float minY = 0.5f;
			float verticalSteps = 60;
			float stepY = (maxY - minY) / verticalSteps;

			for (size_t vertStep = 0; vertStep <= verticalSteps; vertStep++)
			{
				float currentY = minY + stepY * vertStep;
				verticalScanline.push_back({ currentX, currentY });
			}

			auto lineInt = IntersectCurves(verticalScanline, line);
			auto curveInt = IntersectCurves(verticalScanline, curve);

			LineIntersectionData beg;
			LineIntersectionData end;
			if (reversed)
			{
				beg = curveInt[0];
				end = lineInt[0];
			}
			else
			{
				beg = lineInt[0];
				end = curveInt[0];
			}


			std::vector<DirectX::XMFLOAT2> segment;
			if (beg.qLineIndex != end.qLineIndex)
			{
				segment = ExtractSegmentFromOutline(verticalScanline, beg.qLineIndex, end.qLineIndex);
				if (reversed)
				{
					std::reverse(segment.begin(), segment.end());
				}
			}

			path.push_back(lastIntersection.intersectionPoint);
			path.push_back(beg.intersectionPoint);
			path.insert(path.end(), segment.begin(), segment.end());
			path.push_back(end.intersectionPoint);

			lastIntersection = end;
			reversed = !reversed;
		}
	}
	path.push_back(line[line.size() - 1]);

	//auto topCurve = ExtractSegmentFromOutline(intersectionCurve, topCurveInt[2].pLineIndex, topCurveInt[3].pLineIndex);
	//auto topLine2 = ExtractSegmentFromOutline(scanlineTop, topCurveInt[2].qLineIndex, topCurveInt[3].qLineIndex);

	//auto botCurveInt = IntersectCurves(scanlineBot, intersectionCurve);
	//auto botCurve1 = ExtractSegmentFromOutline(intersectionCurve, botCurveInt[0].pLineIndex, botCurveInt[1].pLineIndex);
	//auto botCurve2 = ExtractSegmentFromOutline(intersectionCurve, botCurveInt[2].pLineIndex, botCurveInt[3].pLineIndex);


	return path;
}

std::vector<DirectX::XMFLOAT3> DetailPathsCreationManager::RemoveSpecialPoints(const std::vector<DirectX::XMFLOAT3>& endPathPoints)
{
	float safeHeight = 5;
	std::vector<DirectX::XMFLOAT3> res;
	for (size_t ptIdx = 0; ptIdx < endPathPoints.size(); ptIdx++)
	{
		auto pt = endPathPoints[ptIdx];
		if (pt.x == -1.0f && pt.y == -1.0f && pt.z == -1.0f)
		{
			auto prev = endPathPoints[ptIdx - 1];
			prev.z = -safeHeight;
			auto next = endPathPoints[ptIdx + 1];
			next.z = -safeHeight;

			res.push_back(prev);
			res.push_back(next);
		}
		else
		{
			res.push_back(pt);
		}
	}

	return res;
}
