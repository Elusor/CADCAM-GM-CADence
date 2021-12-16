#include "PathCreationManager.h"
#include "MillingHullRenderPass.h"
#include <system_error>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "IntersectionFinder.h"
using namespace DirectX::SimpleMath;

PathCreationManager::PathCreationManager(std::unique_ptr<RenderState>& renderState, Scene* scene, IntersectionFinder* intersectionFinder)
{
	m_millRadius = 0.8f;
	m_millRadiusEps = 0.1f;
	m_passWidth = 1.f;
	
	m_blockMaxHeight = 5.0f;
	m_blockSide = 15.f;
	m_blockBaseHeight = 1.5f;
	m_blockSafetyEps = 0.1f;
	m_modelUpperSafetyEps = 0.1f;
	m_modelDepth = 
		m_blockMaxHeight - 
		(m_blockBaseHeight + m_blockSafetyEps) - // To prevent milling into the base
		m_modelUpperSafetyEps; // To prevent milling at the exact top of the block		

	m_offset = 0.0f;
	m_zNear = 1.0f;
	m_zFar = 4.5f;
	m_scene = scene;
	m_resolution = 250;

	m_radius = 0.8;

	m_millingHullPass = std::make_unique<MillingHullRenderPass>(
		renderState, 
		m_blockSide,
		m_zNear, m_zFar,
		m_resolution);

	m_model = std::make_unique<PathModel>();

	m_basePathsCreationManager = std::make_unique<BasePathsCreationManager>(intersectionFinder, m_scene, m_blockBaseHeight);

	m_millingHullPass->SetOffset(m_millRadius + m_millRadiusEps);
}

void PathCreationManager::RenderGui(std::unique_ptr<RenderState>& renderState)
{
	if (ImGui::Checkbox("Manual offset settings", &m_manualOffset))
	{
		if (m_manualOffset)
		{
			if (m_offset <= m_millRadius) m_offset = m_millRadius;
			m_millingHullPass->SetOffset(m_offset);
		}
		else
		{
			m_millingHullPass->SetOffset(m_millRadius + m_millRadiusEps);
		}

	}

	if (m_manualOffset)
	{
		if (ImGui::DragFloat("Hull offset##PathCreationManager", &m_offset, 0.05))
		{
			if (m_offset <= m_millRadius) m_offset = m_millRadius;

			m_millingHullPass->SetOffset(m_offset);
		}
	}
	else
	{
		if (ImGui::DragFloat("Mill radius (in cm)##PathCreationManager", &m_millRadius, 0.1))
		{
			if (m_millRadius <= 0) m_millRadius = 0.01;

			m_millingHullPass->SetOffset(m_millRadius + m_millRadiusEps);
		}

		if (ImGui::DragFloat("Mill radius eps (in cm)##PathCreationManager", &m_millRadiusEps, 0.05))
		{
			if (m_millRadiusEps <= 0) m_millRadiusEps = 0.01;

			m_millingHullPass->SetOffset(m_millRadius + m_millRadiusEps);
		}
	}

	if (ImGui::DragFloat("Mill path width (in cm)##PathCreationManager", &m_passWidth, 0.1))
	{
		if (m_passWidth < m_millRadius / 10.f) m_passWidth = m_millRadius / 10.f;
	}

	if (ImGui::Button("Create milling model"))
	{
		CreateMillingModel();
	}

	if (m_model->GetModelObjects().size() > 0)
	{
		if (ImGui::Button("Create General Path"))
		{
			ExecuteRenderPass(renderState);
		}

		if (ImGui::Button("Create Base Path"))
		{
			m_basePathsCreationManager->CreateBasePaths(m_model.get());
		}
		
	}

	ImGui::Text("Current milling model elements:");
	for (auto name : m_model->GetObjectNames())
	{
		ImGui::Text(name.c_str());
	}
}

void PathCreationManager::ExecuteRenderPass(std::unique_ptr<RenderState>& renderState)
{
	// Todo render scene without grid, points etc. only Model and the block
	m_millingHullPass->Execute(renderState, m_model.get());
	ParseDepthTexture(renderState);
}

void PathCreationManager::CreateMillingModel()
{	
	m_model->SetModelObjects(m_scene->m_selectedNodes);
}

void PathCreationManager::ParseDepthTexture(std::unique_ptr<RenderState>& renderState)
{		
	auto& context = renderState->m_device.context();
	auto& device = renderState->m_device.m_device;

	// Map depth texture
	mini::dx_ptr<ID3D11Texture2D> stagingTexture;
	ID3D11Texture2D* dsTex = m_millingHullPass->GetDepthTex();	
	ID3D11Texture2D* mappedTexture = dsTex;
	D3D11_MAPPED_SUBRESOURCE mapInfo;
	D3D11_TEXTURE2D_DESC desc;
	// Get Depth tex desc
	dsTex->GetDesc(&desc);

	auto hr = context->Map(dsTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo);
	// If mapping fails - create a stagin texture which can be copied to cpu
	if (FAILED(hr))
	{
	

		// Prepare the stagin tex
		
		D3D11_TEXTURE2D_DESC stagDesc;
		stagDesc = desc;
		stagDesc.Usage = D3D11_USAGE_STAGING;
		stagDesc.BindFlags = 0;
		stagDesc.MiscFlags = 0;
		stagDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;

		stagingTexture = renderState->m_device.CreateTexture(stagDesc);
		context->CopyResource(stagingTexture.get(), dsTex);

		// Map staged texture 
		hr = context->Map(
			stagingTexture.get(),
			0,
			D3D11_MAP_READ,
			0,
			&mapInfo);

		if (FAILED(hr))
		{
			std::wstring err = L"Failed to map staging texture";
		}
		else
		{
			mappedTexture = stagingTexture.get();
		}
	}
	else
	{
		mappedTexture = dsTex;
	}

	// Copy data to local collection
	UINT dataSize = m_resolution * m_resolution;
	UINT rowSize = m_resolution;
	std::vector<float> data;

	// MapSubresource includes an infoabout internal padding
	// RowPitch is a value of bytes thats why having a pointer stride of char is useful
	// RowPitch can include automatic device padding and user should not depend on this value
	char* rawData = reinterpret_cast<char*>(mapInfo.pData);

	data.reserve(dataSize);

	try
	{
		float maxVal = 0.0f;
		float minVal = 2.f * m_modelDepth;

		for (int h = 0; h < desc.Height; h++)
		{
			for (int w = 0; w < desc.Width; w++)
			{
				// Device dependant offset - jump that many bytes to read the next line
				UINT deviceRowOffset = h * mapInfo.RowPitch;				
				UINT deviceColOffset = w * sizeof(float);

				// Move using the pointer arithmetics
				char* deviceDependantDataPointer = rawData + deviceRowOffset + deviceColOffset;
				
				// Get the actual value at data position
				float* valElem = reinterpret_cast<float*>(deviceDependantDataPointer);
				float val = *valElem;
				// Remember to render with an offset block surface!
				float newVal = m_modelDepth * (1.f - NormalizedLinearDepth(LinearizeDepth(val))) - m_millRadius;
				if (newVal > maxVal)
				{
					maxVal = newVal;
				}
				if (newVal < minVal)
				{
					minVal = newVal;
				}

				data.push_back(newVal);
			}
		}

		// TODO: Scale data to [0,m_modelDepth]
		maxVal -= minVal;
		for (int idx = 0; idx < data.size(); idx++)
		{
			float val = data[idx] - minVal;
			assert(val / maxVal <= 1.0f);
			float scaledVal = val / maxVal * m_modelDepth;
			data[idx] = scaledVal;
		}

		SavePathToFile(data);

	}
	catch (const std::exception& e)
	{
		data.clear();
	}

	context->Unmap(mappedTexture, 0);
}

float PathCreationManager::NormalizedLinearDepth(float linearDepth)
{
	float normDepth = (linearDepth - m_zNear) / (m_zFar - m_zNear);
	assert(normDepth <= 1.0f);
	return normDepth;
}

float PathCreationManager::LinearizeDepth(float uNormDepth)
{
	float d = uNormDepth;
	float n = m_zNear;
	float f = m_zFar;

	//float linDepth = 2.f*(n * f) / (f + n - d * (f - n));
	float linDepth = 2.f * (d - n) / (f - n) - 1.f;
	return linDepth;
}

//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
Vector3 ConvertToMilimeters(Vector3 point)
{
	Vector3 res;
	res = Vector3(point.x * 10.f, point.y * 10.f, point.z * 10.f);
	return res;
}

// All points are represented in centimeters
std::vector<DirectX::SimpleMath::Vector3> PathCreationManager::GeneratePath(std::vector<float>& heights)
{
	std::vector<DirectX::SimpleMath::Vector3> path;

	float texW = m_blockSide / (float)m_resolution;
	float texH = m_blockSide / (float)m_resolution;

	float texWHalf = texW / 2.f;
	float texHHalf = texH / 2.f;

	float halfSide = m_blockSide / 2.f;
	

	// Left upper corner pos
	float safeHeight = m_blockBaseHeight + m_modelDepth * 1.5f;
	Vector3 safePos = Vector3(0.0f, 0.0f, safeHeight);
	float baseSafeHeight = m_blockBaseHeight + m_blockSafetyEps;
	
	Vector3 basePos = Vector3(-halfSide, halfSide, baseSafeHeight);
	Vector3 luCorner = Vector3(-halfSide, halfSide, safeHeight);
	Vector3 rdCorner = Vector3(halfSide, -halfSide, safeHeight);
	Vector3 luCornerWork = Vector3(0.0f, 0.0f, heights[0]) + basePos;
	Vector3 rdCornerWork = Vector3(m_blockSide, -m_blockSide, heights[heights.size() - 1]) + basePos;

	float upperPassMinHeight = (m_modelDepth / 2.f);	
	
	UINT yStride = m_resolution * (m_passWidth / m_blockSide);
	if (yStride < 1) yStride = 1;

	// TODO add first row

	// Add next rows
	path.push_back(safePos);
	path.push_back(luCorner);
	AddHeightPointsWithMinVal(path, heights, basePos, yStride, upperPassMinHeight);		

	path.push_back(luCorner);
	path.push_back(luCornerWork);
	AddHeightPointsWithMinVal(path, heights, basePos, yStride, 0.0f);	
	
	return path;
}

bool PathCreationManager::SavePathToFile(std::vector<float>& heights)
{
	//Select file 
	std::ofstream myfile;
	myfile.open("example.k16");
	
	//Reset instruction counter
	m_instructionCounter = 3;
	if (myfile.is_open())
	{
		auto points = GeneratePath(heights);

		Vector3 safePt = Vector3(0.0f, 0.0f, m_blockBaseHeight + m_modelDepth * 2.f);
		Vector3 safePtMM = ConvertToMilimeters(safePt);

		//// Use milimeters
		//myfile << "%G71\n";
		//// Opening sequence
		//PushInstructionToFile(myfile, "G40G90");
		//// Rotation speed and direction
		//PushInstructionToFile(myfile, "S10000M03");
		//// Mill movement speed
		//PushInstructionToFile(myfile, "F15000");

		//Move to a safe location
		PushInstructionToFile(myfile, PrepareMoveInstruction(safePtMM));

		for (int i = 0; i < points.size(); i++)
		{		
			auto mmPt = ConvertToMilimeters(points[i]);
			PushInstructionToFile(myfile, PrepareMoveInstruction(mmPt));
		}

		//Move to a safe location
		PushInstructionToFile(myfile, PrepareMoveInstruction(safePtMM), true);

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

void PathCreationManager::AddHeightPointsWithMinVal(
	std::vector<DirectX::SimpleMath::Vector3>& path,
	std::vector<float>& heights,
	Vector3 basePos, UINT yStride, float minVal)
{
	float safeHeight = m_blockBaseHeight + m_modelDepth * 1.5f;
	Vector3 safePos = Vector3(0.0f, 0.0f, safeHeight);

	float texW = m_blockSide / (float)m_resolution;
	float texH = m_blockSide / (float)m_resolution;
	auto texWHalf = texW / 2.0f;
	auto texHHalf = texH / 2.0f;

	Vector3 lastVtx = safePos;
	float lastHeight = heights[0] > minVal ? heights[0] : minVal;
	for (int yIdx = 0; yIdx * yStride < m_resolution; yIdx++)
	{
		for (int xIdx = 0; xIdx < m_resolution; xIdx++)
		{
			UINT heightIdx = (yIdx * yStride);
			UINT widthIdx = yIdx % 2 == 0 ? xIdx : m_resolution - 1 - xIdx;
			UINT idx = heightIdx * m_resolution + widthIdx;
			float height = heights[idx] > minVal ? heights[idx] : minVal;

			if (xIdx > 0 && xIdx < m_resolution - 1)
			{
				auto nextIdx = yIdx % 2 == 0 ? idx + 1 : idx - 1;
				float nextH = heights[nextIdx] > minVal ? heights[nextIdx] : minVal;

				if (nextH != height)
				{
					Vector3 pos = Vector3(
						texW * (float)widthIdx + texWHalf,
						-texH * (float)heightIdx - texHHalf,
						height);
					path.push_back(pos + basePos);
					lastVtx = pos + basePos;
				}

			}

			// Make sure to add add the beggining and end of every row
			// Also add if previous height was different
			if ((height != lastHeight) ||
				(widthIdx == 0 || widthIdx == m_resolution - 1))
			{
				Vector3 pos = Vector3(
					texW * (float)widthIdx + texWHalf,
					-texH * (float)heightIdx - texHHalf,
					height);
				path.push_back(pos + basePos);
				lastVtx = pos + basePos;
			}

			lastHeight = height;
		}
	}
	
	auto lastSafe = lastVtx;
	lastSafe.z = safeHeight;

	path.push_back(lastSafe);
	path.push_back(safePos);
}

void PathCreationManager::PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr)
{	
	file << "N" << std::to_string(m_instructionCounter) << instructionText;

	if (lastInstr == false)
	{
		file << "\n";
	}

	m_instructionCounter++;
}

std::string PathCreationManager::PrepareMoveInstruction(DirectX::SimpleMath::Vector3 pos)
{
	std::stringstream stream;
	stream  << "G01";

	// Prepare the number formatting
	stream << std::fixed << std::showpoint << std::setprecision(3);
	stream << "X" << pos.x;
	stream << "Y" << pos.y;
	stream << "Z" << pos.z;
	
	return stream.str();
}
