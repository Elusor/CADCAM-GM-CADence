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
	auto pathPointsParams = PrepareBackFin(normalizedBackFin);
	auto denormalizedPathPointParams = DenormalizeParameters(pathPointsParams, backFinObject);
	VisualizeCurve(&backFinOffsetObject, denormalizedPathPointParams);

	// Trim the "single intersection objects"

	// Merge the trimmed ares of the base parts

	// Merge the trimmed areas of the main body


#pragma region Visualize intersections
	/*VisualizeCurve(&bodyOffsetObject, bodyXbackFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXsideFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXeyeIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection2);

	VisualizeCurve(&bodyOffsetObject, bodyXsideSpikeLU);
	VisualizeCurve(&bodyOffsetObject, bodyXsideSpikeLL);*/
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
		result.push_back(DirectX::XMFLOAT2(pt.x * maxParams.u, pt.y * maxParams.v));
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
	return std::vector<DirectX::XMFLOAT2>();
}

std::vector<DirectX::XMFLOAT2> DetailPathsCreationManager::PrepareEye(const std::vector<DirectX::XMFLOAT2>& intersectionParams)
{
	return std::vector<DirectX::XMFLOAT2>();
}
