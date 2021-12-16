#include "BasePathsCreationManager.h"
#include "ParametricOffsetSurface.h"
#include "Scene.h"

BasePathsCreationManager::BasePathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene, float baseHeight)
{
	m_intersectionFinder = intersectionFinder;
	m_blockSafeHeight = baseHeight;
	m_scene = scene;
}

void BasePathsCreationManager::CreateBasePaths(PathModel* model)
{
	// 1. Create proper intersections of the model with the non offset surfaces
	CalculateOffsetSurfaceIntersections(model);
	// 2. Merge intersection lines
	MergeIntersections();
	// 3. Save paths to file
	SavePathsToFile();
}

void BasePathsCreationManager::CalculateOffsetSurfaceIntersections(PathModel* model)
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


	// Body top with hair
#pragma region Top and hair
	auto bodyTopHairPoints = IntersectCurves(bodyCurveUpper.surfPParams, hairCurve.surfPParams);

	auto bodyTopSection1 = std::vector<DirectX::XMFLOAT2>(
		bodyCurveUpper.surfPParams.begin(),
		bodyCurveUpper.surfPParams.begin() + bodyTopHairPoints[0].qLineIndex
		);

	auto bodyTopSection2 = std::vector<DirectX::XMFLOAT2>(
		bodyCurveUpper.surfPParams.begin() + bodyTopHairPoints[1].qLineIndex,
		bodyCurveUpper.surfPParams.end()
		);

	auto hairSection = std::vector<DirectX::XMFLOAT2>(
		hairCurve.surfPParams.begin() + bodyTopHairPoints[0].pLineIndex,
		hairCurve.surfPParams.begin() + bodyTopHairPoints[1].pLineIndex
		);
#pragma endregion

	// Body top section with tail upper
#pragma region Top and Tail
	auto bodyTopTailPoints = IntersectCurves(bodyTopSection1, tailCurveUpper.surfPParams);

	auto bodyTopSection1Tail = std::vector<DirectX::XMFLOAT2>(
		bodyTopSection1.begin() + bodyTopTailPoints[0].qLineIndex,
		bodyTopSection1.end()
		);

	auto tailUpperSection = std::vector<DirectX::XMFLOAT2>(
		tailCurveUpper.surfPParams.begin(),
		tailCurveUpper.surfPParams.begin() + bodyTopTailPoints[0].pLineIndex
		);
#pragma endregion

	// Body bottom with tail lower
#pragma region Bottom and Tail
	auto bodyBottomTailPoints = IntersectCurves(bodyCurveLower.surfPParams, tailUpperSection);

	auto lowerBodySection = std::vector<DirectX::XMFLOAT2>(
		bodyCurveLower.surfPParams.begin(),
		bodyCurveLower.surfPParams.begin() + bodyBottomTailPoints[0].qLineIndex
		);

	auto tailEndSection = std::vector<DirectX::XMFLOAT2>(
		tailUpperSection.begin() + bodyBottomTailPoints[0].pLineIndex,
		tailUpperSection.end()
		);
#pragma endregion

	// Add split top segments
	VisualizeCurve(baseParametricObject, bodyTopSection1Tail);
	VisualizeCurve(baseParametricObject, bodyTopSection2);

	VisualizeCurve(baseParametricObject, lowerBodySection);
	
	VisualizeCurve(baseParametricObject, tailEndSection);
	//VisualizeCurve(baseParametricObject, tailEndSection);

	VisualizeCurve(baseParametricObject, hairSection);
}

void BasePathsCreationManager::MergeIntersections()
{
}

void BasePathsCreationManager::SavePathsToFile()
{
}

void BasePathsCreationManager::VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params)
{
	for (auto pt : params)
	{
		auto pos = surface->GetPoint(pt.x, pt.y);
		Transform ptTransform;
		ptTransform.SetPosition(pos);
		ptTransform.Translate(0, 0, -5.F);
		m_scene->AttachObject(m_factory.CreatePoint(ptTransform));
	}
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
		res.second = beg1;
	}

	return res;
}
