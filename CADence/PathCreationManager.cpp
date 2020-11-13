#include "PathCreationManager.h"
#include "MillingHullRenderPass.h"
#include <system_error>

PathCreationManager::PathCreationManager(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	m_offset = 0.0f;
	m_zNear = 1.0f;
	m_zFar = 6.0f;
	m_scene = scene;
	m_resolution = 1000;
	float cameraSizeLength = 15.f;

	m_millingHullPass = std::make_unique<MillingHullRenderPass>(
		renderState, 
		cameraSizeLength, 
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
				data.push_back(LinearizeDepth(val));
			}
		}
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
