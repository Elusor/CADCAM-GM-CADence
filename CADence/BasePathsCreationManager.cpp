#include "BasePathsCreationManager.h"
#include "ParametricOffsetSurface.h"
#include "Scene.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "PathUtils.h"

BasePathsCreationManager::BasePathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene, float baseHeight)
{
	m_intersectionFinder = intersectionFinder;
	m_blockSafeHeight = baseHeight;
	m_scene = scene;
}

void BasePathsCreationManager::CreateBasePaths(PathModel* model)
{
	m_blockBaseHeight = 1.5F;

	auto base = dynamic_cast<IParametricSurface*>(model->GetModelObjects()[6].lock().get()->m_object.get());

	// 1. Create proper intersections of the model with the non offset surfaces
	auto paramLits = CalculateOffsetSurfaceIntersections(model);

	std::vector<DirectX::XMFLOAT3> worldPoints;
	for (auto param : paramLits)
	{
		auto pt = base->GetPoint(param);
		worldPoints.push_back(pt);
		pt.z = 0.0F;
	}

	auto pathPoints = IntersectPointsWithVerticalMillLines(worldPoints);

	std::vector<ObjectRef> points;
	for (auto pointPos : worldPoints)
	{
		Transform ptTransform;
		ptTransform.SetPosition(pointPos);
		ptTransform.Translate(0, 0, -5.F);
		auto point = m_factory.CreatePoint(ptTransform);
		point->SetIsVirtual(true);
		m_scene->AttachObject(point);
		points.push_back(point);
	}

	auto curve = m_factory.CreateInterpolBezierCurveC2(points);
	m_scene->AttachObject(curve);

	DirectX::XMFLOAT3 startPtZero = DirectX::XMFLOAT3(0, 0, 5.0f);
	DirectX::XMFLOAT3 startPt = DirectX::XMFLOAT3(-8.2f, 8.5f, 5.0f);
	DirectX::XMFLOAT3 safePt = DirectX::XMFLOAT3(-8.2f, 8.5f, m_blockBaseHeight);

	std::vector<DirectX::XMFLOAT3> generalPrePoints{ startPtZero, startPt, safePt };
	std::vector<DirectX::XMFLOAT3> generalPostPoints{ safePt, startPt, startPtZero};
	// Add safe points to the 
	SavePathToFile(pathPoints, generalPrePoints, generalPostPoints, "2_base_general.f10");

	// Generate the outline pts
	DirectX::XMFLOAT3 outlinePrePoint = safePt;
	outlinePrePoint.y = worldPoints[0].y;
	DirectX::XMFLOAT3 outlinePostPoint = safePt;
	outlinePostPoint.y = (worldPoints.end() - 1)->y;

	std::vector<DirectX::XMFLOAT3> outlinePrePoints{ startPtZero, startPt, safePt , outlinePrePoint};
	std::vector<DirectX::XMFLOAT3> outlinePostPoints{ outlinePostPoint, safePt, startPt, startPtZero };

	SavePathToFile(worldPoints, outlinePrePoints, outlinePostPoints, "2_base_outline.f10");
}

std::vector<DirectX::XMFLOAT2> BasePathsCreationManager::CalculateOffsetSurfaceIntersections(PathModel* model)
{
	auto modelObjects = model->GetModelObjects();

	float offset = -0.5F;

	auto bodyRef = modelObjects[0];
	auto hairRef = modelObjects[2];
	auto tailRef = modelObjects[3];
	auto baseRef = modelObjects[6];

	auto bodyParametricObject = dynamic_cast<IParametricSurface*>(bodyRef.lock().get()->m_object.get());
	auto hairParametricObject = dynamic_cast<IParametricSurface*>(hairRef.lock().get()->m_object.get());
	auto tailParametricObject = dynamic_cast<IParametricSurface*>(tailRef.lock().get()->m_object.get());
	auto baseParametricObject = dynamic_cast<IParametricSurface*>(baseRef.lock().get()->m_object.get());

	ParametricOffsetSurface bodyOffsetSurface(bodyParametricObject, offset);
	ParametricOffsetSurface hairOffsetSurface(hairParametricObject, offset);
	ParametricOffsetSurface tailOffsetSurface(tailParametricObject, offset);

	auto bodyCurveUpper = m_intersectionFinder->FindIntersectionWithCursor(&bodyOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(0, 2.6F, 0));
	//VisualizeCurve(baseParametricObject, bodyCurveUpper.surfPParams);
	auto bodyCurveLower = m_intersectionFinder->FindIntersectionWithCursor(&bodyOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(0, -1.78F, 0));
	//VisualizeCurve(baseParametricObject, bodyCurveLower.surfPParams);
	// For 1e-1 step tail results in one intersection
	auto tailCurveUpper = m_intersectionFinder->FindIntersectionWithCursor(&tailOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(-0.95F, 5.55F, 0)); //better starting point
	//VisualizeCurve(baseParametricObject, tailCurveUpper.surfPParams);
	auto tailCurveLower = m_intersectionFinder->FindIntersectionWithCursor(&tailOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(4, -5.25, 0));
	//VisualizeCurve(baseParametricObject, tailCurveLower.surfPParams);
	auto hairCurve = m_intersectionFinder->FindIntersectionWithCursor(&hairOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(-0.95F, 5.55F, 0));
	//VisualizeCurve(baseParametricObject, hairCurve.surfPParams);

	// TODO when getting the line index be careful to check the directionality of the intersected curve to cut the right line out of the curve
	// https://cdn.discordapp.com/attachments/643546986901012480/921134941960568852/unknown.png

	// Body top with hair

	auto endOfTheLineOffset = 1;
	auto endIteratorOffset = 1;
	std::vector<DirectX::XMFLOAT2> endParams;

#pragma region Top and hair
	auto bodyTopHairPoints = IntersectCurves(bodyCurveUpper.surfPParams, hairCurve.surfPParams);

	auto bodyTopSection2 = std::vector<DirectX::XMFLOAT2>(
		bodyCurveUpper.surfPParams.begin() + bodyTopHairPoints[0].qLineIndex + endOfTheLineOffset,
		bodyCurveUpper.surfPParams.end()
		);
	
#pragma endregion

	// Body top section with tail upper


	// Body bottom with tail lower
#pragma region Bottom and Tail
	auto bodyBottomTailPoints = IntersectCurves(bodyCurveLower.surfPParams, tailCurveUpper.surfPParams);
	auto tailHairPoints = IntersectCurves(hairCurve.surfPParams, tailCurveUpper.surfPParams);

	auto hairSection = std::vector<DirectX::XMFLOAT2>(
		hairCurve.surfPParams.begin() + tailHairPoints[0].qLineIndex + endOfTheLineOffset,
		hairCurve.surfPParams.begin() + bodyTopHairPoints[0].pLineIndex + endIteratorOffset		
		);

	auto lowerBodySection = std::vector<DirectX::XMFLOAT2>(
		bodyCurveLower.surfPParams.begin(),
		bodyCurveLower.surfPParams.begin() + bodyBottomTailPoints[0].qLineIndex + endIteratorOffset
		);

	auto tailEndSection = ExtractSegmentFromOutline(tailCurveUpper.surfPParams, bodyBottomTailPoints[0].pLineIndex, tailHairPoints[0].pLineIndex + 1);		
#pragma endregion

	// Add split top segments
	//VisualizeCurve(baseParametricObject, bodyTopSection1Tail);
	//VisualizeCurve(baseParametricObject, bodyTopSection2);
	//VisualizeCurve(baseParametricObject, lowerBodySection);	
	//VisualizeCurve(baseParametricObject, tailEndSection);
	//VisualizeCurve(baseParametricObject, tailEndSection);
	//VisualizeCurve(baseParametricObject, hairSection);

	// points that should be added
	// VisualizeCurve(baseParametricObject, std::vector<DirectX::XMFLOAT2>(1, bodyTopHairPoints[0].intersectionPoint), -6.5F);
	// VisualizeCurve(baseParametricObject, std::vector<DirectX::XMFLOAT2>(1, bodyTopHairPoints[1].intersectionPoint), -6.5F);	
	// VisualizeCurve(baseParametricObject, std::vector<DirectX::XMFLOAT2>(1, bodyTopTailPoints[0].intersectionPoint), -6.5F);
	// VisualizeCurve(baseParametricObject, std::vector<DirectX::XMFLOAT2>(1, bodyBottomTailPoints[0].intersectionPoint), -6.5F);
	
	// TODO:
	// Add a safety point at mouth
	endParams.reserve(bodyTopSection2.size() + lowerBodySection.size() + tailEndSection.size() + hairSection.size() + 4);
		
	// Forehead to hair
	std::reverse(bodyTopSection2.begin(), bodyTopSection2.end());
	endParams.insert(endParams.end(), bodyTopSection2.begin(), bodyTopSection2.end());
	// Connector
	endParams.insert(endParams.end(), bodyTopHairPoints[0].intersectionPoint);
	// Hair
	std::reverse(hairSection.begin(), hairSection.end());
	endParams.insert(endParams.end(), hairSection.begin(), hairSection.end());
	// Connector
	endParams.insert(endParams.end(), tailHairPoints[0].intersectionPoint);
	// Tail
	std::reverse(tailEndSection.begin(), tailEndSection.end());
	endParams.insert(endParams.end(), tailEndSection.begin(), tailEndSection.end());
	// Connector
	endParams.insert(endParams.end(), bodyBottomTailPoints[0].intersectionPoint);
	// Tail to jaw
	std::reverse(lowerBodySection.begin(), lowerBodySection.end());
	endParams.insert(endParams.end(), lowerBodySection.begin(), lowerBodySection.end());
	
	// Add the first point to close the loop
	endParams.push_back(endParams[0]);

	/*std::vector<ObjectRef> points;
	for (auto param : endParams)
	{
		auto pos = baseParametricObject->GetPoint(param.x, param.y);
		Transform ptTransform;
		ptTransform.SetPosition(pos);
		ptTransform.Translate(0, 0, -5.0F);
		auto point = m_factory.CreatePoint(ptTransform);
		point->SetIsVirtual(true);
		m_scene->AttachObject(point);
		points.push_back(point);
	}
	
	auto curve = m_factory.CreateInterpolBezierCurveC2(points);
	m_scene->AttachObject(curve);*/
	//VisualizeCurve(baseParametricObject, endParams);
	return endParams;
}

std::vector<DirectX::XMFLOAT3> BasePathsCreationManager::IntersectPointsWithVerticalMillLines(const std::vector<DirectX::XMFLOAT3>& outlinePoints)
{
	// Mill data
	float radiusInCm = 0.5;
	float millEps = 0.02f;

	// Base data
	float initialXOffset = -7.5f;
	float baseMaxXOffset = 7.5f;
	float baseWidth = baseMaxXOffset - initialXOffset;

	// Process Data
	float stepWidth = 2 * radiusInCm - millEps;
	int steps = baseWidth / stepWidth;
	bool intersected = false;

	// 4 vectors with pairs of points from top to bottom
	// before the model
	std::vector<pointPair> nonIntersectingBefore;
	// top of the body
	std::vector<LineIntersectionData> intersectionsTop;
	std::vector<pointPair> intersectionPairsTop;
	// bottom of the body
	std::vector<LineIntersectionData> intersectionsBot;
	std::vector<pointPair> intersectionPairsBot;
	// after the model
	std::vector<pointPair> nonIntersectingAfter;

	std::vector<DirectX::XMFLOAT3> points;
	for (size_t stepId = 0; stepId <= steps; stepId++)
	{
		float currentX = initialXOffset + stepId * stepWidth;
		DirectX::XMFLOAT3 beg = DirectX::XMFLOAT3(currentX, 8.5, 0);
		DirectX::XMFLOAT3 end = DirectX::XMFLOAT3(currentX, -8.5, 0);
		std::vector<DirectX::XMFLOAT3> verticalLine{ beg,end };
		
		auto res = IntersectCurves(verticalLine, outlinePoints);
		if (res.size())
		{
			intersected = true;
			auto intersectionWorldPointTop = DirectX::XMFLOAT3(res[0].intersectionPoint.x, res[0].intersectionPoint.y, 0.0F);
			intersectionPairsTop.push_back({ beg, intersectionWorldPointTop });
			intersectionsTop.push_back(res[0]);

			auto intersectionWorldPointBottom = DirectX::XMFLOAT3(res[1].intersectionPoint.x, res[1].intersectionPoint.y, 0.0F);
			intersectionPairsBot.push_back({ intersectionWorldPointBottom, end});
			intersectionsBot.push_back(res[1]);
		}
		else
		{
			if (intersected)
			{
				nonIntersectingAfter.push_back({beg, end});
			}
			else
			{
				nonIntersectingBefore.push_back({beg, end});
			}
		}
	}

	std::vector<DirectX::XMFLOAT3> endPathPoints;

	// If the amount is odd then the last line will be from top to bottom
	bool lastWholeLineTopToBottom = nonIntersectingBefore.size() % 2 == 1;
	bool reversed = false;
	for (auto line : nonIntersectingBefore)
	{
		if (reversed)
		{
			endPathPoints.push_back(line.second);
			endPathPoints.push_back(line.first);
		}
		else
		{
			endPathPoints.push_back(line.first);
			endPathPoints.push_back(line.second);
		}
		
		reversed = !reversed;
	}
	// TODO WATCH out for situation where the pair count is not even
	if (lastWholeLineTopToBottom)
	{
		// Add bottom points
		auto ptsBot = AddBottomPointsLeftToRight(
			intersectionPairsBot, 
			intersectionsBot, 
			outlinePoints
		);

		endPathPoints.insert(endPathPoints.end(), ptsBot.begin(), ptsBot.end());

		AddAfterPoints(nonIntersectingAfter, lastWholeLineTopToBottom, endPathPoints);

		auto ptsTop = AddUpperPointsLeftToRight(
			intersectionPairsTop,
			intersectionsTop,
			outlinePoints
		);

		std::reverse(ptsTop.begin(), ptsTop.end());
		endPathPoints.insert(endPathPoints.end(), ptsTop.begin(), ptsTop.end());
	}
	else
	{
		// Add top points
	}
	
	return endPathPoints;
}

void BasePathsCreationManager::VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params, float distance)
{
	for (auto pt : params)
	{
		auto pos = surface->GetPoint(pt.x, pt.y);
		Transform ptTransform;
		ptTransform.SetPosition(pos);
		ptTransform.Translate(0, 0, distance);
		auto point = m_factory.CreatePoint(ptTransform);
		m_scene->AttachObject(point);
	}
}

std::vector<DirectX::XMFLOAT3> BasePathsCreationManager::AddBottomPointsLeftToRight(
	const std::vector<pointPair>& bottomPairs, 
	const std::vector<LineIntersectionData>& intersectionPoints, 
	const std::vector<DirectX::XMFLOAT3>& outlinePoints)
{
	std::vector<DirectX::XMFLOAT3> result;
	bool evenCount = bottomPairs.size() % 2 == 0;
	assert(evenCount);

	for (size_t pairIdx = 0; pairIdx < bottomPairs.size(); pairIdx++)
	{
		bool reversed = pairIdx % 2 == 0;

		if (reversed)
		{
			// bot
			result.push_back(bottomPairs[pairIdx].second);
			// top
			result.push_back(bottomPairs[pairIdx].first);

			// Add the section of outline
			auto sectionBeg = intersectionPoints[pairIdx].pLineIndex;
			auto sectionEnd = intersectionPoints[pairIdx + 1].pLineIndex;

			auto segment = ExtractSegmentFromOutline(outlinePoints, sectionBeg, sectionEnd);
			std::reverse(segment.begin(), segment.end());
			result.insert(result.end(), segment.begin(), segment.end());
		}
		else
		{
			// top
			result.push_back(bottomPairs[pairIdx].first);
			// bot
			result.push_back(bottomPairs[pairIdx].second);
		}
	}

	return result;
}

std::vector<DirectX::XMFLOAT3> BasePathsCreationManager::AddUpperPointsLeftToRight(const std::vector<pointPair>& upperPairs, const std::vector<LineIntersectionData>& intersectionPoints, const std::vector<DirectX::XMFLOAT3>& outlinePoints)
{
	std::vector<DirectX::XMFLOAT3> result;
	bool evenCount = upperPairs.size() % 2 == 0;
	assert(evenCount);

	for (size_t pairIdx = 0; pairIdx < upperPairs.size(); pairIdx++)
	{
		bool reversed = pairIdx % 2 == 1;

		if (reversed)
		{
			// bot
			result.push_back(upperPairs[pairIdx].second);
			// top
			result.push_back(upperPairs[pairIdx].first);
		}
		else
		{
			// top
			result.push_back(upperPairs[pairIdx].first);
			// bot
			result.push_back(upperPairs[pairIdx].second);

			// Add the section of outline
			auto sectionBeg = intersectionPoints[pairIdx].pLineIndex;
			auto sectionEnd = intersectionPoints[pairIdx + 1].pLineIndex;

			auto segment = ExtractSegmentFromOutline(outlinePoints, sectionBeg, sectionEnd);
			//std::reverse(segment.begin(), segment.end());
			result.insert(result.end(), segment.begin(), segment.end());
		}
	}

	return result;
}

void BasePathsCreationManager::AddAfterPoints(
	const std::vector<pointPair>& afterPairs, 
	const bool startFromBottom, 
	std::vector<DirectX::XMFLOAT3>& endPoints)
{
	bool reversed = startFromBottom;
	for (const auto& line : afterPairs)
	{
		if (reversed)
		{
			endPoints.push_back(line.second);
			endPoints.push_back(line.first);
		}
		else
		{
			endPoints.push_back(line.first);
			endPoints.push_back(line.second);
		}

		reversed = !reversed;
	}
}

bool BasePathsCreationManager::SavePathToFile(
	const std::vector<DirectX::XMFLOAT3>& positions, 
	const std::vector<DirectX::XMFLOAT3>& prePoints, 
	const std::vector<DirectX::XMFLOAT3>& postPoints, 
	std::string name)
{
	//Select file 
	std::ofstream myfile;
	myfile.open(name);

	//Reset instruction counter
	m_instructionCounter = 3;
	m_blockBaseHeight = 1.5f;
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

		//Move to a safe location
		for (auto pt : prePoints)
		{
			PushInstructionToFile(myfile, PrepareMoveInstruction(ConvertToMilimeters(pt)));
		}

		for (int i = 0; i < positions.size(); i++)
		{
			SimpleMath::Vector3 pos = positions[i];
			pos.z = m_blockBaseHeight;
			auto mmPt = ConvertToMilimeters(pos);
			PushInstructionToFile(myfile, PrepareMoveInstruction(mmPt));
		}

		//Move to a safe location
		for(size_t idx = 0; idx < postPoints.size(); idx++)
		{
			PushInstructionToFile(myfile, PrepareMoveInstruction(ConvertToMilimeters(postPoints[idx])), idx == postPoints.size()-1);
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

void BasePathsCreationManager::PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr)
{
	file << "N" << std::to_string(m_instructionCounter) << instructionText;

	if (lastInstr == false)
	{
		file << "\n";
	}

	m_instructionCounter++;
}
