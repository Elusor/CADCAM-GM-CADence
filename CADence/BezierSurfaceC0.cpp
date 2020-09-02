#pragma once 
#include "BezierSurfaceC0.h"
#include "imgui.h"
#include "mathUtils.h"
#include "Scene.h"
#include "Transform.h"

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

std::weak_ptr<Node> BezierSurfaceC0::GetPatch(int w, int h)
{	
	auto points = m_patches;
	return points[m_patchH * (w) + h];

}

BezierPatch* BezierSurfaceC0::GetPatchAtParameter(float& u, float& v)
{

	// u - width
	// v - height 
	
	// determine the W and H of a given patch and get the point from this patch	
	assert(ParamsInsideBounds(u,v));
	GetWrappedParams(u, v);

	int w = (int)(u * (float)m_patchW);
	int h = (int)(v * (float)m_patchH);

	if (w == m_patchW)
		w--;
	if (h == m_patchH)
		h--;

	float uStep = 1.f / (float)m_patchW;
	float vStep = 1.f / (float)m_patchH;

	float uPatchBoundary = (float)w * uStep;
	float vPatchBoundary = (float)h * vStep;

	// Scale translated parameters to [0;1]
	float newU = (u - uPatchBoundary) * m_patchW; 
	float newV = (v - vPatchBoundary) * m_patchH;

	auto node = GetPatch(w, h).lock();

	u = newU;
	v = newV;

	return (BezierPatch*)(node->m_object).get();
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

ParameterPair BezierSurfaceC0::GetMaxParameterValues()
{
	ParameterPair maxParams;

	//maxParams.u = (float) m_patchW;
	//maxParams.v = (float) m_patchH;

	maxParams.u = (float)1.0f;
	maxParams.v = (float)1.0f;

	return maxParams;
}

DirectX::XMFLOAT3 BezierSurfaceC0::GetPoint(float u, float v)
{
	// Gets the patch at given parameters and sclae u,v to appropriate size
	float uRef = u;
	float vRef = v;
	auto patch = GetPatchAtParameter(uRef, vRef);
	return patch->GetPoint(uRef, vRef);
}

DirectX::XMFLOAT3 BezierSurfaceC0::GetTangent(float u, float v, TangentDir tangentDir)
{
	// Gets the patch at given parameters and sclae u,v to appropriate size
	float uRef = u;
	float vRef = v;
	auto patch = GetPatchAtParameter(uRef, vRef);

	DirectX::XMFLOAT3 tan = patch->GetTangent(uRef, vRef, tangentDir);
	DirectX::XMFLOAT3 scaledTan;
	if (tangentDir == TangentDir::AlongU)
	{
		scaledTan = tan * m_patchH;
	}
	else 
	{
		scaledTan = tan * m_patchW;
	}

	return scaledTan;
}

DirectX::XMFLOAT3 BezierSurfaceC0::GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir)
{
	// Gets the patch at given parameters and sclae u,v to appropriate size
	float uRef = u;
	float vRef = v;
	auto patch = GetPatchAtParameter(uRef, vRef);
	return patch->GetSecondDarivativeSameDirection(uRef, vRef, tangentDir);
}

DirectX::XMFLOAT3 BezierSurfaceC0::GetSecondDarivativeMixed(float u, float v)
{
	// Gets the patch at given parameters and sclae u,v to appropriate size
	float uRef = u;
	float vRef = v;
	auto patch = GetPatchAtParameter(uRef, vRef);
	return patch->GetSecondDarivativeMixed(uRef, vRef);
}

bool BezierSurfaceC0::ParamsInsideBounds(float u, float v)
{
	bool res = false;
	bool UinRange = (u >= 0 && u <= 1);
	bool VinRange = (v >= 0 && v <= 1);

	if (m_wrapDir == SurfaceWrapDirection::None)
	{ // None
		// No wrap check both params
		res = (UinRange && VinRange);
	}
	else {
		// TODO: Check if v corresponds to height and u to width
		if (m_wrapDir == SurfaceWrapDirection::Height)
		{ //Height
			// Height wrapped so check width
			res = UinRange;
		}
		else 
		{ //Width
			// Width wrapped so check height
			res = VinRange;
		}
	}

	return res;
}

void BezierSurfaceC0::GetWrappedParams(float& u, float& v)
{		

	auto maxParams = GetMaxParameterValues();

	// TODO: Check if v corresponds to height and u to width
	if (m_wrapDir == SurfaceWrapDirection::Height)
	{ 
		float vIntPart;
		float newV = modff(v, &vIntPart);
		if (newV < 0.0f)
		{
			newV = 1.f + newV;
		}
		v = newV;
	}

	if (m_wrapDir == SurfaceWrapDirection::Width)
	{ 
		float uIntPart;
		float newU = modff(u, &uIntPart);
		if (newU < 0.0f)
		{
			newU = 1.f + newU;
		}
		u = newU;
	}
}

float BezierSurfaceC0::GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep)
{

	float res = defStep;
	DirectX::XMFLOAT2 params = DirectX::XMFLOAT2(u, v);
	DirectX::XMFLOAT2 movedParams = params + dir * defStep;

	if (ParamsInsideBounds(movedParams.x, movedParams.y) == false)
	{	
		// parameters are outside bounds
		switch (m_wrapDir)
		{
		case SurfaceWrapDirection::None:
			// check for nearest intersection with u and v edges

			// Check u edges
			if (dir.x != 0.0f)
			{
				float step0 = (0.f - u) / dir.x;
				float step1 = (1.f - u) / dir.x;

				if (step0 * defStep >= 0 && abs(step0) < abs(res)) //has the same sign
				{
					res = step0;
				}

				if (step1 * defStep >= 0 && abs(step1) < abs(res)) //has the same sign
				{
					res = step1;
				}
			}

			// Check v edges
			if (dir.y != 0.0f)
			{
				float step0 = (0.f - v) / dir.y;
				float step1 = (1.f - v) / dir.y;

				if (step0 * defStep >= 0 && abs(step0) < abs(res)) //has the same sign
				{
					res = step0;
				}

				if (step1 * defStep >= 0 && abs(step1) < abs(res)) //has the same sign
				{
					res = step1;
				}
			}

			break;

		case SurfaceWrapDirection::Width:
			// check for nearest intersection with v edges
			if (dir.y != 0.0f)
			{
				float step0 = (0.f - v) / dir.y;
				float step1 = (1.f - v) / dir.y;

				if (step0 * defStep>= 0 && abs(step0) < abs(res)) //has the same sign
				{
					res = step0;
				}

				if (step1 * defStep >= 0 && abs(step1) < abs(res)) //has the same sign
				{
					res = step1;
				}
			}
			break;

		case SurfaceWrapDirection::Height:
			// check for nearest intersection with u edges
			// Check u edges
			if (dir.x != 0.0f)
			{
				float step0 = (0.f - u) / dir.x;
				float step1 = (1.f - u) / dir.x;

				if (step0 * defStep >= 0 && abs(step0) < abs(res)) //has the same sign
				{
					res = step0;
				}

				if (step1 * defStep >= 0 && abs(step1) < abs(res)) //has the same sign
				{
					res = step1;
				}
			}
			break;
		}


	}

	return res;
}

void BezierSurfaceC0::RenderObjectSpecificContextOptions(Scene& scene)
{
	if (ImGui::Button("Place debug points"))
	{
		for (int u = 0; u <= 10; u++)
		{
			for (int v = 0; v <= 10; v++)
			{
				float paramV = 1.f / 10.f * (float)v;
				float paramU = 1.f / 10.f * (float)u;

				auto pos = this->GetPoint(paramU, paramV);
				Transform t;
				t.SetPosition(pos);
				auto pt = scene.m_objectFactory->CreatePoint(t);
				scene.AttachObject(pt);
			}
		}
	}
}