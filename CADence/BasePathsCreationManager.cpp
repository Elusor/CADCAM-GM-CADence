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
	for (auto pointPos : pathPoints)
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

	// Add safe points to the 
	SavePathToFile(pathPoints);
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
	auto bodyCurveLower = m_intersectionFinder->FindIntersectionWithCursor(&bodyOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(0, -1.78F, 0));
	// For 1e-1 step tail results in one intersection
	auto tailCurveUpper = m_intersectionFinder->FindIntersectionWithCursor(&tailOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(-0.95F, 5.55F, 0)); //better starting point
	auto tailCurveLower = m_intersectionFinder->FindIntersectionWithCursor(&tailOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(4, -5.25, 0));
	auto hairCurve = m_intersectionFinder->FindIntersectionWithCursor(&hairOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(-0.95F, 5.55F, 0));

	// TODO when getting the line index be careful to check the directionality of the intersected curve to cut the right line out of the curve
	// https://cdn.discordapp.com/attachments/643546986901012480/921134941960568852/unknown.png

	// Body top with hair

	auto endOfTheLineOffset = 1;
	auto endIteratorOffset = 1;

#pragma region Top and hair
	auto bodyTopHairPoints = IntersectCurves(bodyCurveUpper.surfPParams, hairCurve.surfPParams);

	auto bodyTopSection1 = std::vector<DirectX::XMFLOAT2>(
		bodyCurveUpper.surfPParams.begin(),
		bodyCurveUpper.surfPParams.begin() + bodyTopHairPoints[0].qLineIndex + endIteratorOffset
		);

	auto bodyTopSection2 = std::vector<DirectX::XMFLOAT2>(
		bodyCurveUpper.surfPParams.begin() + bodyTopHairPoints[1].qLineIndex + endOfTheLineOffset,
		bodyCurveUpper.surfPParams.end()
		);

	auto hairSection = std::vector<DirectX::XMFLOAT2>(
		hairCurve.surfPParams.begin() + bodyTopHairPoints[0].pLineIndex + endOfTheLineOffset,
		hairCurve.surfPParams.begin() + bodyTopHairPoints[1].pLineIndex + endIteratorOffset
		);
#pragma endregion

	// Body top section with tail upper
#pragma region Top and Tail
	auto bodyTopTailPoints = IntersectCurves(bodyTopSection1, tailCurveUpper.surfPParams);

	auto bodyTopSection1Tail = std::vector<DirectX::XMFLOAT2>(
		bodyTopSection1.begin() + bodyTopTailPoints[0].qLineIndex + endOfTheLineOffset,
		bodyTopSection1.end()
		);

	auto tailUpperSection = std::vector<DirectX::XMFLOAT2>(
		tailCurveUpper.surfPParams.begin(),
		tailCurveUpper.surfPParams.begin() + bodyTopTailPoints[0].pLineIndex + endIteratorOffset
		);
#pragma endregion

	// Body bottom with tail lower
#pragma region Bottom and Tail
	auto bodyBottomTailPoints = IntersectCurves(bodyCurveLower.surfPParams, tailUpperSection);

	auto lowerBodySection = std::vector<DirectX::XMFLOAT2>(
		bodyCurveLower.surfPParams.begin(),
		bodyCurveLower.surfPParams.begin() + bodyBottomTailPoints[0].qLineIndex + endIteratorOffset
		);

	auto tailEndSection = std::vector<DirectX::XMFLOAT2>(
		tailUpperSection.begin() + bodyBottomTailPoints[0].pLineIndex + endOfTheLineOffset,
		tailUpperSection.end()
		);
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
	std::vector<DirectX::XMFLOAT2> endParams;
	endParams.reserve(bodyTopSection1Tail.size() + bodyTopSection2.size() + lowerBodySection.size() + tailEndSection.size() + hairSection.size() + 4);
		
	// Forehead to hair
	std::reverse(bodyTopSection2.begin(), bodyTopSection2.end());
	endParams.insert(endParams.end(), bodyTopSection2.begin(), bodyTopSection2.end());
	// Connector
	endParams.insert(endParams.end(), bodyTopHairPoints[1].intersectionPoint);
	// Hair
	std::reverse(hairSection.begin(), hairSection.end());
	endParams.insert(endParams.end(), hairSection.begin(), hairSection.end());
	// Connector
	endParams.insert(endParams.end(), bodyTopHairPoints[0].intersectionPoint);
	// Hair to Tail
	std::reverse(bodyTopSection1Tail.begin(), bodyTopSection1Tail.end());
	endParams.insert(endParams.end(), bodyTopSection1Tail.begin(), bodyTopSection1Tail.end());
	// Connector
	endParams.insert(endParams.end(), bodyTopTailPoints[0].intersectionPoint);
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
		DirectX::XMFLOAT3 beg = DirectX::XMFLOAT3(currentX, 7.5, 0);
		DirectX::XMFLOAT3 end = DirectX::XMFLOAT3(currentX, -7.5, 0);
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

std::vector<LineIntersectionData> BasePathsCreationManager::IntersectCurves(
	const std::vector<DirectX::XMFLOAT3>& params1,
	const std::vector<DirectX::XMFLOAT3>& params2)
{
	std::vector<LineIntersectionData> result;

	for (size_t line1Idx = 0; line1Idx < params1.size() - 1; line1Idx++)
	{
		auto beg1 = params1[line1Idx];
		auto end1 = params1[line1Idx + 1];

		for (size_t line2Idx = 0; line2Idx < params2.size() - 1; line2Idx++)
		{
			auto beg2 = params2[line2Idx];
			auto end2 = params2[line2Idx + 1];

			auto res = GetIntersectionPoint(
				DirectX::XMFLOAT2(beg1.x, beg1.y),
				DirectX::XMFLOAT2(end1.x, end1.y),
				DirectX::XMFLOAT2(beg2.x, beg2.y),
				DirectX::XMFLOAT2(end2.x, end2.y));

			if (res.first)
			{
				LineIntersectionData intersectionPoint;
				intersectionPoint.qLineIndex = line1Idx;
				intersectionPoint.pLineIndex = line2Idx;
				intersectionPoint.intersectionPoint = res.second;
				result.push_back(intersectionPoint);
			}
		}
	}

	return result;
}

std::vector<LineIntersectionData> BasePathsCreationManager::IntersectCurves(const std::vector<DirectX::XMFLOAT2>& params1, const std::vector<DirectX::XMFLOAT2>& params2)
{
	std::vector<LineIntersectionData> result;

	for (size_t line1Idx = 0; line1Idx < params1.size() - 1; line1Idx++)
	{
		auto beg1 = params1[line1Idx];
		auto end1 = params1[line1Idx + 1];

		for (size_t line2Idx = 0; line2Idx < params2.size() - 1; line2Idx++)
		{
			auto beg2 = params2[line2Idx];
			auto end2 = params2[line2Idx + 1];

			auto res = GetIntersectionPoint(beg1, end1, beg2, end2);
			if (res.first)
			{
				LineIntersectionData intersectionPoint;
				intersectionPoint.qLineIndex = line1Idx;
				intersectionPoint.pLineIndex = line2Idx;
				intersectionPoint.intersectionPoint = res.second;
				result.push_back(intersectionPoint);
			}
		}
	}

	return result;
}

std::pair<bool, DirectX::XMFLOAT2> BasePathsCreationManager::GetIntersectionPoint(
	const DirectX::SimpleMath::Vector2& beg1,
	const DirectX::SimpleMath::Vector2& end1,
	const DirectX::SimpleMath::Vector2& beg2,
	const DirectX::SimpleMath::Vector2& end2)
{
	auto res = std::pair<bool, DirectX::XMFLOAT2>();
	res.first = false;

	auto segment1Vector = end1 - beg1;
	auto segment2Vector = end2 - beg2;

	auto segment1ToBeg2 = beg2 - beg1;
	auto segment1ToEnd2 = end2 - beg1;

	auto segment2ToBeg1 = beg1 - beg2;
	auto segment2ToEnd1 = end1 - beg2;

	auto cross1_beg2 = Cross(segment1Vector, segment1ToBeg2);
	auto cross1_end2 = Cross(segment1Vector, segment1ToEnd2);
	
	auto cross2_beg1 = Cross(segment2Vector, segment2ToBeg1);
	auto cross2_end1 = Cross(segment2Vector, segment2ToEnd1);

	auto twoOnDifferentSidesOfOne = Dot(cross1_beg2, cross1_end2) <= 0;
	auto oneOnDifferentSidesOfTwo = Dot(cross2_beg1, cross2_end1) <= 0;

	res.first = oneOnDifferentSidesOfTwo && twoOnDifferentSidesOfOne;

	if (res.first)
	{
		const float s1_x = end1.x - beg1.x;
		const float s1_y = end1.y - beg1.y;

		const float s2_x = end2.x - beg2.x;
		const float s2_y = end2.y - beg2.y;
		
		// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect		
		const float t =
			(s2_x * (beg1.y - beg2.y) - s2_y * (beg1.x - beg2.x)) /
			(-s2_x * s1_y + s1_x * s2_y);

		assert(t >= 0.0f && t <= 1.0f);
		res.second = beg1 + t * (end1 - beg1);
	}

	return res;
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

std::vector<DirectX::XMFLOAT3> BasePathsCreationManager::ExtractSegmentFromOutline(const std::vector<DirectX::XMFLOAT3>& outline, int line1, int line2)
{
	std::vector<DirectX::XMFLOAT3> points;
	auto begLine = line1 < line2 ? line1 : line2;
	auto endLine = line1 < line2 ? line2 : line1;

	size_t endOfLineOffset = 1;	

	// No end of line offset at the end
	points.insert(points.end(), outline.begin() + begLine + endOfLineOffset, outline.begin() + endLine);

	return points;
}

bool BasePathsCreationManager::SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions)
{
	//Select file 
	std::ofstream myfile;
	myfile.open("example.f10");

	//Reset instruction counter
	m_instructionCounter = 3;
	m_blockBaseHeight = 1.5f;
	if (myfile.is_open())
	{

		DirectX::XMFLOAT3 startPtZero = DirectX::XMFLOAT3(0, 0, 5.0f);
		DirectX::XMFLOAT3 startPtZeroMM = ConvertToMilimeters(startPtZero);
		DirectX::XMFLOAT3 startPt = DirectX::XMFLOAT3(-8.2f, 7.5f, 5.0f);
		DirectX::XMFLOAT3 startPtMM = ConvertToMilimeters(startPt);

		DirectX::XMFLOAT3 safePt = DirectX::XMFLOAT3(-8.2f, 7.5f, m_blockBaseHeight);
		DirectX::XMFLOAT3 safePtMM = ConvertToMilimeters(safePt);

		//// Use milimeters
		//myfile << "%G71\n";
		//// Opening sequence
		//PushInstructionToFile(myfile, "G40G90");
		//// Rotation speed and direction
		//PushInstructionToFile(myfile, "S10000M03");
		//// Mill movement speed
		//PushInstructionToFile(myfile, "F15000");

		//Move to a safe location
		PushInstructionToFile(myfile, PrepareMoveInstruction(startPtZeroMM));
		PushInstructionToFile(myfile, PrepareMoveInstruction(startPtMM));
		PushInstructionToFile(myfile, PrepareMoveInstruction(safePtMM));

		for (int i = 0; i < positions.size(); i++)
		{
			SimpleMath::Vector3 pos = positions[i];
			pos.z = m_blockBaseHeight;
			auto mmPt = ConvertToMilimeters(pos);
			PushInstructionToFile(myfile, PrepareMoveInstruction(mmPt));
		}

		//Move to a safe location
		PushInstructionToFile(myfile, PrepareMoveInstruction(safePtMM));
		PushInstructionToFile(myfile, PrepareMoveInstruction(startPtMM));
		PushInstructionToFile(myfile, PrepareMoveInstruction(startPtZeroMM), true);

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
