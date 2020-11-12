#include "adaptiveRenderingCalculator.h"
#include "renderState.h"
#include "Node.h"
#include "Object.h"

unsigned int AdaptiveRenderingCalculator::CalculateAdaptiveSamplesCount(std::vector<std::weak_ptr<Node>> points, std::unique_ptr<RenderState>& renderState)
{
	auto camera = renderState->m_camera;
	auto size = camera->GetViewportSize();	
	int width = size.cx;
	int height = size.cy;

	float minX = FLT_MAX;
	float minY = FLT_MAX;
	float maxX = FLT_MIN;
	float maxY = FLT_MIN;

	// get rectangle from all points' viewports coordinates
#pragma region calculate positions
	for (int i = 0; i < points.size(); i++)
	{
		if (auto point = points[i].lock())
		{
			XMMATRIX VP = renderState->currentCamera->GetViewProjectionMatrix();

			std::string name = point->m_object->m_name;
			DirectX::XMFLOAT4 screenPosFl;
			DirectX::XMVECTOR pointPos = DirectX::XMLoadFloat3(&(point->m_object->GetPosition()));
			DirectX::XMVECTOR screenPos = DirectX::XMVector3Transform(pointPos, VP);

			XMStoreFloat4(&screenPosFl, screenPos);
			float x = screenPosFl.x;
			float y = screenPosFl.y;
			float z = screenPosFl.z;
			float w = screenPosFl.w;

			float xFrust = x / w;
			float yFrust = y / w;

			float wf = (float)width;
			float hf = (float)height;

			float wfHalf = wf / 2.0f;
			float hfHalf = hf / 2.0f;

			float screenW = wfHalf * (xFrust + 1.0f);
			float screenH = hfHalf * (1.0f - yFrust);

			if (screenW < minX)
			{
				minX = screenW;
			}

			if (screenW > maxX)
			{
				maxX = screenW;
			}

			if (screenH < minY)
			{
				minY = screenW;
			}

			if (screenH > maxY)
			{
				maxY = screenW;
			}
		}				
	}

	float hullRectWidth = maxX - minX;
	float hullRectHeight = maxY - minY;

	int segmentLength = 8;
	int samples = sqrtf(hullRectWidth * hullRectWidth + hullRectHeight * hullRectHeight) / (float)(segmentLength);
	samples *= 2;
#pragma endregion
	int minSamples = points.size() * 10;
	int maxSamples = points.size() * 150;
	if (samples < minSamples) samples = minSamples;
	if (samples > maxSamples) samples = maxSamples;
	return samples;
}
