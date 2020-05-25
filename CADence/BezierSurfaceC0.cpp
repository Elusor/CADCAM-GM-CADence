#include "BezierSurfaceC0.h"

BezierSurfaceC0::BezierSurfaceC0(std::vector<std::shared_ptr<Node>> patches)
{
	// mark all patches as "in use"
	for (int i = 0; i < patches.size(); i++)
	{
		patches[i]->m_object->RefUse();
	}

	m_divisions = 4;
	m_patches = patches;
}

BezierSurfaceC0::~BezierSurfaceC0()
{
	// unmark all patches and remove them
	for (int i = 0; i < m_patches.size(); i++)
	{
		m_patches[i]->m_object->RefRelease();
	}
}

void BezierSurfaceC0::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	for (int i = 0; i < m_patches.size(); i++)
	{
		m_patches[i]->m_object->RenderObject(renderState);
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

	std::string dimDrag = "Grid density" + GetIdentifier();
	bool divsChanged = ImGui::DragInt(dimDrag.c_str(), &m_divisions, 1.0f, 2, 64);

	if (m_divisions < 2)
		m_divisions = 2;
	if (m_divisions > 64)
		m_divisions = 64;

	if(divsChanged)
	{
		SetDivisions();
	}

	patchChanged |= divsChanged;

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

void BezierSurfaceC0::SetDivisions()
{
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto patch = (BezierPatch*)(m_patches[i]->m_object.get());
		patch->SetDivisions(m_divisions);
	}
}
