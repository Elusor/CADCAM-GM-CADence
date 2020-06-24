#pragma once 
#include "BezierSurfaceC0.h"
#include "imgui.h"

BezierSurfaceC0::BezierSurfaceC0(std::vector<std::shared_ptr<Node>> patches, int wCount, int hCount, SurfaceWrapDirection wrapDir)
{
	m_wrapDir = wrapDir;
	// mark all patches as "in use"
	for (int i = 0; i < patches.size(); i++)
	{
		patches[i]->m_object->RefUse();
	}

	m_patchW = wCount;
	m_patchH = hCount;

	m_divisionsU = m_divisionsV =  4;
	m_patches = patches;
}

BezierSurfaceC0::~BezierSurfaceC0()
{
	// unmark all patches and remove them
	for (int i = 0; i < m_patches.size(); i++)
	{
		m_patches[i]->m_object->GetReferences().UnlinkAll();
	}
}

void BezierSurfaceC0::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	for (int i = 0; i < m_patches.size(); i++)
	{
		m_patches[i]->m_object->RenderObject(renderState);
	}
}

void BezierSurfaceC0::SetIsSelected(bool isSelected)
{
	MeshObject::SetIsSelected(isSelected);
	SetMeshColor();
}

void BezierSurfaceC0::SetDivisions(int divsU, int divsV)
{
	m_divisionsU = divsU;
	m_divisionsV = divsV;
	SetDivisions();
}

int BezierSurfaceC0::GetUDivisions()
{
	return m_divisionsU;
}

int BezierSurfaceC0::GetVDivisions()
{
	return m_divisionsV;
}

void BezierSurfaceC0::SetDisplayPolygon(bool displayPolygon)
{
	m_displayPatchesPolygon = displayPolygon;
	SetDisplayPolygon();
}

bool BezierSurfaceC0::GetDisplayPolygon()
{
	return m_displayPatchesPolygon;
}

SurfaceWrapDirection BezierSurfaceC0::GetWrapDirection()
{
	return m_wrapDir;
}

std::vector<std::weak_ptr<Node>> BezierSurfaceC0::GetDisplayChildren()
{
	std::vector<std::weak_ptr<Node>> patches;

	for (int i = 0; i < m_patches.size(); i++)
	{
		patches.push_back(m_patches[i]);
		m_patches[i]->SetIsVirtual(false);
	}
	return patches;
}

std::vector<std::weak_ptr<Node>> BezierSurfaceC0::GetPoints(int& height, int& width)
{
	height = m_pointHeight;
	width = m_pointWidth;
	return GetReferences().GetRefObjects();
}

void BezierSurfaceC0::SetPoints(std::shared_ptr<Node>** points, int width, int height)
{
	m_pointWidth = width;
	m_pointHeight = height;

	for (int w = 0; w < width; w++)
	{
		//m_points.push_back(std::vector<std::weak_ptr<Node>>());
		for (int h = 0; h < height; h++)
		{
			GetReferences().LinkRef(points[w][h]);
			//m_points[w].push_back();
		}
	}
}

void BezierSurfaceC0::SetPoints(std::vector<std::vector<std::weak_ptr<Node>>> points, int width, int height)
{
	m_pointWidth = width;
	m_pointHeight = height;
	for (int w = 0; w < width; w++)
	{
		for (int h = 0; h < height; h++)
		{
			GetReferences().LinkRef(points[w][h]);
		}
	}
}

bool BezierSurfaceC0::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	bool patchChanged = false;

	std::string label = "Display Bezier polygons" + GetIdentifier();
	patchChanged |= ImGui::Checkbox(label.c_str(), &m_displayPatchesPolygon);
	ImGui::Spacing();

	// change colors for mesh
	float mcolor[3] = {
		m_meshDesc.m_adjustableColor.x,
		m_meshDesc.m_adjustableColor.y,
		m_meshDesc.m_adjustableColor.z,
	};

	std::string mText = "Mesh color";
	ImGui::Text(mText.c_str());
	bool mColChanged = ImGui::ColorEdit3((mText + "##" + mText + GetIdentifier()).c_str(), (float*)&mcolor);
	patchChanged |= mColChanged;

	m_meshDesc.m_adjustableColor.x = mcolor[0];
	m_meshDesc.m_adjustableColor.y = mcolor[1];
	m_meshDesc.m_adjustableColor.z = mcolor[2];

	if (mColChanged)
	{
		SetMeshColor();
	}

	// change colors for polygon
	float pcolor[3] = {
		m_polygonsColor.x,
		m_polygonsColor.y,
		m_polygonsColor.z,
	};

	if (patchChanged)
	{
		SetDisplayPolygon();
	}

	std::string ptext = "Bezier color";
	ImGui::Text(ptext.c_str());
	bool colChanged = ImGui::ColorEdit3((ptext + "##" + ptext + GetIdentifier()).c_str(), (float*)&pcolor);
	patchChanged |= colChanged;
	m_polygonsColor.x = pcolor[0];
	m_polygonsColor.y = pcolor[1];
	m_polygonsColor.z = pcolor[2];

	if (colChanged)
	{	
		SetPolygonColor();		
	}

	std::string dimDrag = "Grid density U" + GetIdentifier();
	bool divsChanged = ImGui::DragInt(dimDrag.c_str(), &m_divisionsU, 1.0f, 2, 64);

	std::string dimDrag2 = "Grid density V" + GetIdentifier();
	bool divsChanged2 = ImGui::DragInt(dimDrag2.c_str(), &m_divisionsV, 1.0f, 2, 64);

	if (m_divisionsU < 2)
		m_divisionsU = 2;
	if (m_divisionsU > 64)
		m_divisionsU = 64;

	if (m_divisionsV < 2)
		m_divisionsV = 2;
	if (m_divisionsV > 64)
		m_divisionsV = 64;

	if(divsChanged || divsChanged2)
	{
		SetDivisions();
	}

	patchChanged |= divsChanged || divsChanged2;

	ImGui::Spacing();

	ImGui::End();
	return patchChanged;
}

void BezierSurfaceC0::UpdateObject()
{
	for (int i = 0; i < m_patches.size(); i++)
	{		
		auto patch = (BezierPatch*)(m_patches[i]->m_object.get());
		if (patch->GetIsModified())
		{
			patch->UpdateObject();
		}		
	}
}

bool BezierSurfaceC0::GetIsModified()
{
	bool modified = m_modified;
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto patch = (BezierPatch*)(m_patches[i]->m_object.get());
		modified |= patch->GetIsModified();
	}

	return modified;
}

void BezierSurfaceC0::SetPolygonColor()
{
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto patch = (BezierPatch*)(m_patches[i]->m_object.get());
		patch->SetPolygonColor(m_polygonsColor);
	}
}

void BezierSurfaceC0::SetDisplayPolygon()
{
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto patch = (BezierPatch*)(m_patches[i]->m_object.get());
		patch->SetPolygonVisible(m_displayPatchesPolygon);
	}
}

void BezierSurfaceC0::SetMeshColor()
{
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto patch = (BezierPatch*)(m_patches[i]->m_object.get());
		patch->SetPatchColor(m_meshDesc.m_defaultColor);
	}
}

void BezierSurfaceC0::SetDivisions()
{
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto patch = (BezierPatch*)(m_patches[i]->m_object.get());
		patch->SetDivisions(m_divisionsU,m_divisionsV);
	}
}
