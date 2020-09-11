#pragma once 
#include "BezierSurfaceC0.h"
#include "imgui.h"
#include "mathUtils.h"
#include "Scene.h"
#include "Transform.h"
#include "MeshObject.h"
#include "BezierPatch.h"

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

std::vector<std::weak_ptr<Node>> BezierSurfaceC0::GetPatches(int& height, int& width)
{
	height = m_pointHeight;
	width = m_pointWidth;
	auto objList = GetReferences().GetRefObjects();
	std::vector<std::weak_ptr<Node>> points;
	for (int i = 0; i < objList.size(); i++)
	{
		if (auto objRef = objList[i].lock())
		{
			auto pt = dynamic_cast<Point*>(objRef->m_object.get());
			if (pt != nullptr)
			{
				points.push_back(objRef);
			}
		}
	}

	return points;
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
	auto wrappedParams = GetWrappedParams(u, v);

	float patchW;
	float patchH;
	modff(wrappedParams.u, &patchW);
	modff(wrappedParams.v, &patchH);

	if (patchW == m_patchW)
		patchW--;
	if (patchH == m_patchH)
		patchH--;

	float uParam = wrappedParams.u - patchW;
	float vParam = wrappedParams.v - patchH;

	auto node = GetPatch(patchW, patchH).lock();

	u = uParam;
	v = vParam;

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
		m_recalculateMesh = true;
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
	patchChanged |= CreateTrimSwitchGui(GetIdentifier());;

	ImGui::Spacing();

	ImGui::End();

	if (patchChanged)
	{
		SetModified(true);
	}

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

	if (m_intersectionData.intersectionCurve.expired() == false)
	{
		if (m_recalculateMesh)
		{
			auto trimmedSpace = GetTrimmedMesh((m_divisionsU)*m_patchW + 1, m_divisionsV * m_patchH + 1);
			UpdateTrimmedChildrenMeshes(trimmedSpace);
		}
	}
	else 
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
	m_recalculateMesh = true;
}

ParameterPair BezierSurfaceC0::GetMaxParameterValues()
{
	ParameterPair maxParams;

	maxParams.u = (float) m_patchW;
	maxParams.v = (float) m_patchH;

	return maxParams;
}

ParameterPair BezierSurfaceC0::GetNormalizedParams(float u, float v)
{	
	auto params = GetWrappedParams(u, v);

	float newU = params.u / m_patchW;
	float newV = params.v / m_patchH;

	return ParameterPair(newU, newV);
}

DirectX::XMFLOAT2 BezierSurfaceC0::GetParameterSpaceDistance(ParameterPair point1, ParameterPair point2)
{
	auto wrappedP1 = GetWrappedParams(point1.u, point1.v);
	auto wrappedP2 = GetWrappedParams(point2.u, point2.v);

	auto maxParams = GetMaxParameterValues();

	float u1 = wrappedP1.u;
	float v1 = wrappedP1.v;
	float u2 = wrappedP2.u;
	float v2 = wrappedP2.v;

	float distU = abs(u1 - u2);
	float distV = abs(v1 - v2);

	if (m_wrapDir == SurfaceWrapDirection::Width)
		distU = min(distU, maxParams.u - distU);
	if (m_wrapDir == SurfaceWrapDirection::Height)
		distV = min(distV, maxParams.v - distV);

	XMFLOAT2 distUV = { distU, distV };
	return distUV;
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
		//TODO remove?
		scaledTan = tan * m_patchH;
	}
	else 
	{
		//TODO remove?
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

bool BezierSurfaceC0::IsWrappedInDirection(SurfaceWrapDirection wrapDir)
{
	return m_wrapDir == wrapDir;
}

bool BezierSurfaceC0::ParamsInsideBounds(float u, float v)
{
	bool res = false;

	auto maxParams = GetMaxParameterValues();
	bool UinRange = (u >= 0 && u <= maxParams.u);
	bool VinRange = (v >= 0 && v <= maxParams.v);

	if (m_wrapDir == SurfaceWrapDirection::None)
	{ // None
		// No wrap check both params
		res = (UinRange && VinRange);
	}
	else {
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

ParameterPair BezierSurfaceC0::GetWrappedParams(float u, float v)
{		
	auto maxParams = GetMaxParameterValues();

	float resU = u, resV = v;

	if (m_wrapDir == SurfaceWrapDirection::Height)
	{ 
		if (v != maxParams.v)
		{
			float wrappedV = fmod(v, maxParams.v);
			resV = wrappedV >= 0.0f ? wrappedV : maxParams.v + wrappedV;
		}		
	}

	if (m_wrapDir == SurfaceWrapDirection::Width)
	{ 
		if (u != maxParams.u)
		{
			float wrappedU = fmod(u, maxParams.u);
			resU = wrappedU >= 0.0f ? wrappedU : maxParams.u + wrappedU;
		}
	}

	return ParameterPair(resU, resV);
}

float BezierSurfaceC0::GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep)
{

	float res = defStep;
	DirectX::XMFLOAT2 params = DirectX::XMFLOAT2(u, v);
	DirectX::XMFLOAT2 movedParams = params + dir * defStep;

	auto bounds = GetMaxParameterValues();

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
				float step1 = (bounds.u - u) / dir.x;

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
				float step1 = (bounds.v - v) / dir.y;

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
				float step1 = (bounds.v - v) / dir.y;

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
				float step1 = (bounds.u - u) / dir.x;

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

void BezierSurfaceC0::SetIntersectionData(IntersectionData data)
{
	TrimmableSurface::SetIntersectionData(data);
	m_recalculateMesh = true;
}

void BezierSurfaceC0::RenderObjectSpecificContextOptions(Scene& scene)
{
	if (ImGui::MenuItem("Select all points"))
	{
		scene.ClearSelection();

		for (int i = 0; i < m_patches.size(); i++)
		{
			auto patch = dynamic_cast<BezierPatch*>(m_patches[i]->m_object.get());

			if (patch != nullptr)
			{
				auto points = patch->GetPoints();
				scene.Select(points, false);
			}			
		}
	}
}

void BezierSurfaceC0::UpdateTrimmedChildrenMeshes(TrimmedSpace trimmedMesh)
{
	std::vector<VertexParameterColor> vertices;
	for (auto params : trimmedMesh.vertices)
	{
		params.x *= m_patchW;
		params.y *= m_patchH;
		vertices.push_back({
			params,
			m_meshDesc.m_adjustableColor
			});
	}

	for (int u = 0; u < m_patchW; u++)
	{
		for (int v = 0; v < m_patchH; v++)
		{
			auto patchMesh = dynamic_cast<BezierPatch*>(GetPatch(u, v).lock()->m_object.get());
		 	patchMesh->m_meshDesc.vertices = ScaleVerticesForPatch(vertices,u,v);
			patchMesh->m_meshDesc.indices = DetermineIndicesForPatch(vertices, trimmedMesh.indices, u, v);;
		}
	}	
}

std::vector<unsigned short> BezierSurfaceC0::DetermineIndicesForPatch(std::vector<VertexParameterColor> vertices, std::vector<unsigned short> indices, int patchW, int patchH)
{
	std::vector<unsigned short> res;
	int patchCountW = m_patchW;
	int patchCountH = m_patchH;
	auto VertexInsidePatch = [patchCountW, patchCountH](VertexParameterColor vertex, int patchWIdx, int patchHIdx)
	{
		float eps = 0.0000;
		auto params = vertex.parameters;
		float scaledU = params.x;
		float scaledV = params.y;		

		float lowerUBound = (float)patchWIdx;
		float upperUBound = (float)(patchWIdx + 1);

		float lowerVBound = (float)patchHIdx;
		float upperVBound = (float)(patchHIdx + 1);

		bool uInBounds = scaledU >= (lowerUBound-eps) && scaledU <= (upperUBound + eps);
		bool vInBounds = scaledV >= (lowerVBound-eps) && scaledV <= (upperVBound + eps);

		return uInBounds && vInBounds;
	};

	auto ValidIndexes = [VertexInsidePatch, vertices, patchW, patchH, patchCountW, patchCountH]
	(unsigned short idxBeg, unsigned short idxEnd) 
	{
		auto vertex1 = vertices[idxBeg];
		auto vertex2 = vertices[idxEnd];

		bool v1InsideBounds = VertexInsidePatch(vertex1, patchW, patchH);
		bool v2InsideBounds = VertexInsidePatch(vertex2, patchW, patchH);
		return v1InsideBounds && v2InsideBounds;
	};

	for (int i = 0; i < indices.size() - 1; i += 2)
	{
		unsigned short idxBeg = indices[i];
		unsigned short idxEnd = indices[i+1];

		if (ValidIndexes(idxBeg, idxEnd))
		{			
			res.push_back(idxBeg);
			res.push_back(idxEnd);
		}
	}
	return res;
}

std::vector<VertexParameterColor> BezierSurfaceC0::ScaleVerticesForPatch(std::vector<VertexParameterColor> vertices, int patchW, int patchH)
{
	std::vector<VertexParameterColor> res;
	int patchCountW = m_patchW;
	int patchCountH = m_patchH;
	auto VertexInsidePatch = [patchCountW, patchCountH](VertexParameterColor vertex, int patchWIdx, int patchHIdx)
	{
		float eps = 0.0000;
		auto params = vertex.parameters;
		float scaledU = params.x;
		float scaledV = params.y;

		float lowerUBound = (float)patchWIdx;
		float upperUBound = (float)(patchWIdx + 1);

		float lowerVBound = (float)patchHIdx;
		float upperVBound = (float)(patchHIdx + 1);

		bool uInBounds = scaledU >= (lowerUBound - eps) && scaledU <= (upperUBound + eps);
		bool vInBounds = scaledV >= (lowerVBound - eps) && scaledV <= (upperVBound + eps);

		return uInBounds && vInBounds;
	};

	for (int i = 0; i < vertices.size(); i++)
	{
		auto vertex = vertices[i];
		if (VertexInsidePatch(vertex, patchW, patchH))
		{
			float u = vertex.parameters.x;
			float v = vertex.parameters.y;
			float modu = u;
			float modv = v;

			modu -= patchW;
			modv -= patchH;

			vertex.parameters.x = modu;
			vertex.parameters.y = modv;
		}	
		res.push_back(vertex);
	}

	return res;
}
