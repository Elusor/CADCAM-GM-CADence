#include "PathCreationManager.h"
#include "MillingHullRenderPass.h"
#include <system_error>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace DirectX::SimpleMath;

PathCreationManager::PathCreationManager(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	m_millRadius = 0.16f;
	m_modelDepth = 5.0f;
	m_offset = 0.0f;
	m_zNear = 1.0f;
	m_zFar = m_modelDepth + m_zNear;
	m_scene = scene;
	m_resolution = 150;
	m_blockSide = 15.f;
	m_blockBaseHeight = 1.6f;
	m_radius = 0.8;

	m_millingHullPass = std::make_unique<MillingHullRenderPass>(
		renderState, 
		m_blockSide,
		m_zNear, m_zFar,
		m_resolution);

	m_model = std::make_unique<PathModel>();
}

void PathCreationManager::RenderGui(std::unique_ptr<RenderState>& renderState)
{
	if (ImGui::DragFloat("Hull offsetm##PathCreationManager", &m_offset, 0.05))
	{
		m_millingHullPass->SetOffset(m_offset);
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
				data.push_back(m_modelDepth - (LinearizeDepth(val)- m_zNear));
			}
		}

		SavePathToFile(data);

	}
	catch (const std::exception& e)
	{
		data.clear();
	}

	context->Unmap(mappedTexture, 0);
}

float PathCreationManager::LinearizeDepth(float uNormDepth)
{
	float d = uNormDepth;
	float n = m_zNear;
	float f = m_zFar;

	float linDepth = 2.f*(n * f) / (f + n - d * (f - n));
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
	// 

	// Left upper corner pos
	float safeHeight = m_blockBaseHeight + m_modelDepth * 1.5f;
	
	Vector3 safePos = Vector3(0.0f, 0.0f, safeHeight);

	Vector3 basePos = Vector3(-halfSide, halfSide, m_blockBaseHeight);
	Vector3 luCorner = Vector3(-halfSide, halfSide, safeHeight);
	Vector3 rdCorner = Vector3(halfSide, -halfSide, safeHeight);
	Vector3 luCornerWork = Vector3(0.0f, 0.0f, heights[0]) + basePos;
	Vector3 rdCornerWork = Vector3(m_blockSide, -m_blockSide, heights[heights.size() - 1]) + basePos;

	int yStride = (m_radius / 2.f) / texH;
	if (yStride < 1) yStride = 1;
	path.push_back(safePos);
	path.push_back(luCorner);
	path.push_back(luCornerWork);	

	float lastHeight = heights[0];
	for (int yIdx = 0; yIdx < m_resolution; yIdx++)
	{
		for (int xIdx = 0; xIdx < m_resolution; xIdx++)
		{

			UINT heightIdx = yIdx * m_resolution;
			UINT widthIdx = yIdx % 2 == 0 ? xIdx : m_resolution - 1 - xIdx;
			UINT idx = heightIdx + widthIdx;
			float height = heights[idx];
			
			// Make sure to add add the beggining and end of every row
			// Also add if previous height was different
			if ((height != lastHeight) ||
				(widthIdx == 0 || widthIdx == m_resolution - 1))
			{
				Vector3 pos = Vector3(
					texW * (float)widthIdx + texWHalf,
					-texH * (float)yIdx - texHHalf,
					height);
				path.push_back(pos + basePos);
			}

			lastHeight = height;
		}
	}

	path.push_back(rdCornerWork);
	path.push_back(rdCorner);
	path.push_back(safePos);
	return path;
}

bool PathCreationManager::SavePathToFile(std::vector<float>& heights)
{
	//Select file 
	std::ofstream myfile;
	myfile.open("example.k16");
	
	//Reset instruction counter
	m_instructionCounter = 1;
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
