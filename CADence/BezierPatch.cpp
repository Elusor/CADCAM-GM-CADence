#include "BezierPatch.h"
#include "imgui.h"
#include "bezierCalculator.h"
#include "mathUtils.h"
#include "Scene.h"
#include "Transform.h"

BezierPatch::BezierPatch()
{
	m_displayPolygon = false;	
}

BezierPatch::~BezierPatch()
{
}

void BezierPatch::Initialize(
	std::vector<std::weak_ptr<Node>> first, 
	std::vector<std::weak_ptr<Node>> second, 
	std::vector<std::weak_ptr<Node>> third, 
	std::vector<std::weak_ptr<Node>> fourth)
{
	m_displayPolygon = false;
	
	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(first[i]);		
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(second[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(third[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(fourth[i]);
	}


	auto refs = GetReferences().GetAllRef();
	for (int i = 0; i < 16; i++)
	{
		assert(!refs[i].m_refered.expired());
	}
	UpdateObject();
}

void BezierPatch::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	if (m_meshDesc.indices.size() > 0)
	{
		RenderPatch(renderState);
	}
	if (m_displayPolygon)
	{
		RenderPolygon(renderState);
	}
}

void BezierPatch::RenderPatch(std::unique_ptr<RenderState>& renderState)
{
	auto context = renderState->m_device.context().get();

	auto desc = m_meshDesc;
	context->IASetPrimitiveTopology(desc.m_primitiveTopology);
	auto inputLayout = renderState->GetLayout(desc.GetVertexDataTypeIdx());
	renderState->m_device.context()->IASetInputLayout(inputLayout);

	auto prevPreset = renderState->GetCurrentShaderPreset();
	ShaderPreset preset;
	preset.vertexShader = renderState->m_paramVS.get();
	preset.pixelShader = renderState->m_pixelShader.get();
	preset.geometryShader = renderState->m_patchParamGeometryShader.get();
	preset.hullShader = nullptr;
	preset.domainShader = nullptr;
	renderState->SetShaderPreset(preset);

	//Set constant buffer
	XMMATRIX m = m_transform.GetModelMatrix();
	XMMATRIX VP = renderState->m_camera->GetViewProjectionMatrix();
	auto positions = GetPatchPointPositions();
	XMFLOAT4X4 row1 = GetRowAsFloat4x4(positions.row0);	
	XMFLOAT4X4 row2 = GetRowAsFloat4x4(positions.row1);
	XMFLOAT4X4 row3 = GetRowAsFloat4x4(positions.row2); 
	XMFLOAT4X4 row4 = GetRowAsFloat4x4(positions.row3);
	XMFLOAT4X4 rows[4] = { row1, row2, row3, row4 };

	auto CPbuffer1 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData.get(), row1);
	auto CPbuffer2 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData1.get(), row2);
	auto CPbuffer3 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData2.get(), row3);
	auto CPbuffer4 = renderState->SetConstantBuffer<XMFLOAT4X4>(renderState->m_cbPatchData3.get(), row4);
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), m);
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), VP);

	ID3D11Buffer* cbs1[] = { Mbuffer }; //, VPbuffer
	ID3D11Buffer* cbs2[] = { VPbuffer }; //, VPbuffer
	ID3D11Buffer* cbsControlPoint[] = { CPbuffer1,CPbuffer2,CPbuffer3,CPbuffer4 }; //, VPbuffer

	context->GSSetConstantBuffers(0, 1, cbs1);
	context->GSSetConstantBuffers(1, 1, cbs2);
	context->GSSetConstantBuffers(2, 4, cbsControlPoint);

	// Update Vertex and index buffers
	renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(desc.vertices));
	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(desc.indices));
	ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexParameterColor) };
	UINT offsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	context->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(desc.indices.size(), 0, 0);

	renderState->SetShaderPreset(prevPreset);

}

void BezierPatch::RenderPolygon(std::unique_ptr<RenderState>& renderState)
{
	RenderMesh(renderState, m_PolygonDesc);
}

bool BezierPatch::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());	
	bool patchChanged = false;

	std::string label = "Display Bezier polygon" + GetIdentifier();
	patchChanged |= ImGui::Checkbox(label.c_str(), &m_displayPolygon);
	ImGui::Spacing();

	// change colors for polygon
	float pcolor[3] = {
		m_PolygonDesc.m_defaultColor.x,
		m_PolygonDesc.m_defaultColor.y,
		m_PolygonDesc.m_defaultColor.z,
	};

	std::string ptext = "Bezier color";
	ImGui::Text(ptext.c_str());
	patchChanged |= ImGui::ColorEdit3((ptext + "##" + ptext + GetIdentifier()).c_str(), (float*)&pcolor);

	m_PolygonDesc.m_defaultColor.x = pcolor[0];
	m_PolygonDesc.m_defaultColor.y = pcolor[1];
	m_PolygonDesc.m_defaultColor.z = pcolor[2];
	ImGui::Spacing();

	std::string dimDrag = "Grid density" + GetIdentifier();
	patchChanged |= ImGui::DragInt(dimDrag.c_str(), &m_uSize, 1.0f, 2, 64);

	if (m_uSize < 2)
		m_uSize = 2;
	if (m_uSize > 64)
		m_uSize = 64;
	
	m_vSize = m_uSize;

	patchChanged |= CreateTrimSwitchGui(GetIdentifier());;

	ImGui::End();
	return patchChanged;
}

void BezierPatch::UpdateObject()
{	
	m_VDesc.m_primitiveTopology = m_UDesc.m_primitiveTopology =  D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
	m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	// pass an (u,v) line to the shader
	// one of those is the constant parameter - the other one is 
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indicesU;
	std::vector<unsigned short> indicesV; 

// TODO : move to a namespace and make it static
#pragma region mesh generation

	int densityX = m_uSize;
	int densityY = m_vSize;
	m_meshDesc.vertices.clear();
	m_meshDesc.indices.clear();
	for (int u = 0; u <= densityX; u++)
	{
		for (int v = 0; v <= densityY; v++)
		{
			float uParam = (float)u / densityX;
			float vParam = (float)v / densityY;

			VertexParameterColor vertex = {
				{uParam, vParam},
				m_meshDesc.m_defaultColor
			};
			m_meshDesc.vertices.push_back(vertex);
		}
	}

	auto calcIdx = [densityY](int u, int v) {return u * (densityY +1) + v; };
	for (int x = 0; x < densityX; x++)
	{
		for (int y = 0; y < densityY; y++)
		{
			//Add indices representing edges
			m_meshDesc.indices.push_back(calcIdx(x,y));
			m_meshDesc.indices.push_back(calcIdx(x+1, y));

			m_meshDesc.indices.push_back(calcIdx(x, y));
			m_meshDesc.indices.push_back(calcIdx(x, y+1));
		}
	}

	for (int x = 0; x < densityX; x++)
	{
		m_meshDesc.indices.push_back(calcIdx(x, densityY));
		m_meshDesc.indices.push_back(calcIdx(x + 1, densityY));
	}

	for (int y = 0; y < densityY; y++)
	{
		m_meshDesc.indices.push_back(calcIdx(densityX, y));
		m_meshDesc.indices.push_back(calcIdx(densityX, y+1));
	}
#pragma endregion

	if (m_intersectionData.intersectionCurve.expired() == false)
	{
		auto space = GetTrimmedMesh(densityX + 1, densityY + 1);
		auto curColor = m_meshDesc.vertices[0].color;
		m_meshDesc.vertices.clear();
		for (auto params : space.vertices)
		{
			m_meshDesc.vertices.push_back({
				params,
				curColor });

		}
		m_meshDesc.indices = space.indices;
	}


	m_PolygonDesc.vertices.clear();
	m_PolygonDesc.indices.clear();
	m_PolygonDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	for (int i = 0; i < 16; i++) {
		m_PolygonDesc.vertices.push_back(VertexPositionColor{
			GetReferences().GetAllRef()[i].m_refered.lock()->m_object->GetPosition(),
			m_meshDesc.m_defaultColor });
	}

	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.indices.push_back(4*i);
		m_PolygonDesc.indices.push_back(4*i + 1);
		m_PolygonDesc.indices.push_back(4*i + 1);
		m_PolygonDesc.indices.push_back(4*i + 2);
		m_PolygonDesc.indices.push_back(4*i + 2);
		m_PolygonDesc.indices.push_back(4*i + 3);
	}

	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.indices.push_back(i);
		m_PolygonDesc.indices.push_back(i + 4); 
		m_PolygonDesc.indices.push_back(i + 4);
		m_PolygonDesc.indices.push_back(i + 8);
		m_PolygonDesc.indices.push_back(i + 8);
		m_PolygonDesc.indices.push_back(i + 12);
	}

	m_UDesc.vertices = m_VDesc.vertices = vertices;
	m_UDesc.indices = indicesU;
	m_VDesc.indices = indicesV;
	//m_meshDesc.indices = indices;
}

bool BezierPatch::GetIsModified()
{
	bool modified = m_modified;

	auto points = GetReferences().GetAllRef();
	for (int i = 0; i < 16; i++)
	{
		modified |= points[i].m_refered.lock()->m_object->GetIsModified();
	}

	return modified;
}

void BezierPatch::SetDivisions(int divsU, int divsV)
{
	m_uSize = divsU;
	m_vSize = divsV;
}

void BezierPatch::SetPolygonVisible(bool state)
{
	m_displayPolygon = state;
}

void BezierPatch::SetPolygonColor(DirectX::XMFLOAT3 col)
{
	m_PolygonDesc.m_adjustableColor = col;
	SetModified(true);
}

void BezierPatch::SetPatchColor(DirectX::XMFLOAT3 col)
{
	m_meshDesc.m_adjustableColor = col;
	SetModified(true);
}

BezierPatchPointPos BezierPatch::GetPatchPointPositions()
{
	BezierPatchPointPos patch;

	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 p00, p01, p02, p03;
	p00 = refs[0].m_refered.lock()->m_object->GetPosition();
	p01 = refs[1].m_refered.lock()->m_object->GetPosition();
	p02 = refs[2].m_refered.lock()->m_object->GetPosition();
	p03 = refs[3].m_refered.lock()->m_object->GetPosition();	
	patch.row0 = BezierCoeffs{ p00,p01,p02,p03 };

	DirectX::XMFLOAT3 p10, p11, p12, p13;
	p10 = refs[4].m_refered.lock()->m_object->GetPosition();
	p11 = refs[5].m_refered.lock()->m_object->GetPosition();
	p12 = refs[6].m_refered.lock()->m_object->GetPosition();
	p13 = refs[7].m_refered.lock()->m_object->GetPosition();
	patch.row1 = BezierCoeffs{ p10,p11,p12,p13 };

	DirectX::XMFLOAT3 p20, p21, p22, p23;
	p20 = refs[8].m_refered.lock()->m_object->GetPosition();
	p21 = refs[9].m_refered.lock()->m_object->GetPosition();
	p22 = refs[10].m_refered.lock()->m_object->GetPosition();
	p23 = refs[11].m_refered.lock()->m_object->GetPosition();
	patch.row2 = BezierCoeffs{ p20,p21,p22,p23 };

	DirectX::XMFLOAT3 p30, p31, p32, p33;
	p30 = refs[12].m_refered.lock()->m_object->GetPosition();
	p31 = refs[13].m_refered.lock()->m_object->GetPosition();
	p32 = refs[14].m_refered.lock()->m_object->GetPosition();
	p33 = refs[15].m_refered.lock()->m_object->GetPosition();	
	patch.row3 = BezierCoeffs{ p30,p31,p32,p33 };

	return patch;
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPoints()
{
	return GetReferences().GetRefObjects();
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPoints(BoundaryDirection direction)
{
	std::vector<std::weak_ptr<Node>> points;
	auto refs = GetReferences().GetAllRef();
	switch (direction) {
	case BoundaryDirection::Top:
		points.push_back(refs[0].m_refered);
		points.push_back(refs[1].m_refered);
		points.push_back(refs[2].m_refered);
		points.push_back(refs[3].m_refered);
		break;
	case BoundaryDirection::Bottom:
		points.push_back(refs[12].m_refered);
		points.push_back(refs[13].m_refered);
		points.push_back(refs[14].m_refered);
		points.push_back(refs[15].m_refered);
		break;
	case BoundaryDirection::Left:
		points.push_back(refs[0].m_refered);
		points.push_back(refs[4].m_refered);
		points.push_back(refs[8].m_refered);
		points.push_back(refs[12].m_refered);
		break;
	case BoundaryDirection::Right:
		points.push_back(refs[3].m_refered);
		points.push_back(refs[7].m_refered);
		points.push_back(refs[11].m_refered);
		points.push_back(refs[15].m_refered);
		break;
	}

	return points;
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPreBoundaryPoints(BoundaryDirection direction)
{
	std::vector<std::weak_ptr<Node>> points;
	auto refs = GetReferences().GetAllRef();
	switch (direction) {
	case BoundaryDirection::Top:
		points.push_back(refs[4].m_refered);
		points.push_back(refs[5].m_refered);
		points.push_back(refs[6].m_refered);
		points.push_back(refs[7].m_refered);
		break;
	case BoundaryDirection::Bottom:
		points.push_back(refs[8].m_refered);
		points.push_back(refs[9].m_refered);
		points.push_back(refs[10].m_refered);
		points.push_back(refs[1].m_refered);
		break;
	case BoundaryDirection::Left:
		points.push_back(refs[1].m_refered);
		points.push_back(refs[5].m_refered);
		points.push_back(refs[9].m_refered);
		points.push_back(refs[13].m_refered);
		break;
	case BoundaryDirection::Right:
		points.push_back(refs[2].m_refered);
		points.push_back(refs[6].m_refered);
		points.push_back(refs[10].m_refered);
		points.push_back(refs[14].m_refered);
		break;
	}

	return points;
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPoints(RowPlace row)
{
	std::vector<std::weak_ptr<Node>> points;
	auto refs = GetReferences().GetAllRef();
	switch (row) {
	case RowPlace::First:
		points.push_back(refs[0].m_refered);
		points.push_back(refs[1].m_refered);
		points.push_back(refs[2].m_refered);
		points.push_back(refs[3].m_refered);
		break;
	case RowPlace::Second:
		points.push_back(refs[4].m_refered);
		points.push_back(refs[5].m_refered);
		points.push_back(refs[6].m_refered);
		points.push_back(refs[7].m_refered);		
		break;
	case RowPlace::Third:
		points.push_back(refs[8].m_refered);
		points.push_back(refs[9].m_refered);
		points.push_back(refs[10].m_refered);
		points.push_back(refs[11].m_refered);
		break;
	case RowPlace::Fourth:
		points.push_back(refs[12].m_refered);
		points.push_back(refs[13].m_refered);
		points.push_back(refs[14].m_refered);
		points.push_back(refs[15].m_refered);
		break;
	}

	return points;
}

float GetCoord(std::weak_ptr<Node> point, Coord coord)
{
	if (auto pt = point.lock())
	{
		auto pos = pt->m_object->GetPosition();
		switch (coord) {
		case Coord::Xpos:
			return pos.x;
		case Coord::Ypos:
return pos.y;
		case Coord::Zpos:
			return pos.z;
		}
	}

	return NAN;
}


XMMATRIX BezierPatch::GetCoordinates(Coord coord)
{

	auto refs = GetReferences().GetAllRef();
	XMFLOAT4X4 mat{
		GetCoord(refs[0].m_refered, coord), GetCoord(refs[1].m_refered, coord), GetCoord(refs[2].m_refered, coord), GetCoord(refs[3].m_refered, coord),
		GetCoord(refs[4].m_refered, coord), GetCoord(refs[5].m_refered, coord), GetCoord(refs[6].m_refered, coord), GetCoord(refs[7].m_refered, coord),
		GetCoord(refs[8].m_refered, coord), GetCoord(refs[9].m_refered, coord), GetCoord(refs[10].m_refered, coord), GetCoord(refs[11].m_refered, coord),
		GetCoord(refs[12].m_refered, coord), GetCoord(refs[13].m_refered, coord), GetCoord(refs[14].m_refered, coord), GetCoord(refs[15].m_refered, coord)
	};
	
	return XMLoadFloat4x4(&mat);
}

XMFLOAT4X4 BezierPatch::GetRowAsFloat4x4(BezierCoeffs row)
{
	XMFLOAT4X4 rowMat;

	rowMat._11 = row.b0.x;
	rowMat._21 = row.b0.y;
	rowMat._31 = row.b0.z;
	rowMat._41 = 1.0f;

	rowMat._12 = row.b1.x;
	rowMat._22 = row.b1.y;
	rowMat._32 = row.b1.z;
	rowMat._42 = 1.0f;

	rowMat._13 = row.b2.x;
	rowMat._23 = row.b2.y;
	rowMat._33 = row.b2.z;
	rowMat._43 = 1.0f;

	rowMat._14 = row.b3.x;
	rowMat._24 = row.b3.y;
	rowMat._34 = row.b3.z;
	rowMat._44 = 1.0f;

	return rowMat;
}

void BezierPatch::RenderObjectSpecificContextOptions(Scene& scene)
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

ParameterPair BezierPatch::GetMaxParameterValues()
{
	ParameterPair res;
	res.u = 1.0f;
	res.v = 1.0f;
	return res;
}

ParameterPair BezierPatch::GetNormalizedParams(float u, float v)
{
	// Intentional - no need to scale
	return ParameterPair(u,v);
}

DirectX::XMFLOAT2 BezierPatch::GetParameterSpaceDistance(ParameterPair point1, ParameterPair point2)
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

	XMFLOAT2 distUV = { distU, distV };
	return distUV;
}

DirectX::XMFLOAT3 BezierPatch::GetPoint(float u, float v)
{
	auto refs = GetReferences().GetAllRef();

	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	auto points = GetPatchPointPositions();
	
	aux0 = BezierCalculator::CalculateBezier4(points.row0, u);
	aux1 = BezierCalculator::CalculateBezier4(points.row1, u);	
	aux2 = BezierCalculator::CalculateBezier4(points.row2, u);	
	aux3 = BezierCalculator::CalculateBezier4(points.row3, u);
	auto res = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
	return res;
}

DirectX::XMFLOAT3 BezierPatch::GetTangent(float u, float v, TangentDir tangentDir)
{
	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	auto points = GetPatchPointPositions();

	DirectX::XMFLOAT3 result;

	if (tangentDir == TangentDir::AlongU)
	{
		// Along U
		auto row0Der = BezierCalculator::GetDerivativeCoefficients(points.row0);
		auto row1Der = BezierCalculator::GetDerivativeCoefficients(points.row1);
		auto row2Der = BezierCalculator::GetDerivativeCoefficients(points.row2);
		auto row3Der = BezierCalculator::GetDerivativeCoefficients(points.row3);
		
		aux0 = BezierCalculator::CalculateBezier3(row0Der, u);
		aux1 = BezierCalculator::CalculateBezier3(row1Der, u);
		aux2 = BezierCalculator::CalculateBezier3(row2Der, u);
		aux3 = BezierCalculator::CalculateBezier3(row3Der, u);
		
		result = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
	}
	else {
		// Along V
		aux0 = BezierCalculator::CalculateBezier4(points.row0, u);
		aux1 = BezierCalculator::CalculateBezier4(points.row1, u);
		aux2 = BezierCalculator::CalculateBezier4(points.row2, u);
		aux3 = BezierCalculator::CalculateBezier4(points.row3, u);
		
		// Calculate diff along V
		auto derPoints = BezierCalculator::GetDerivativeCoefficients(aux0, aux1, aux2, aux3);
		result = BezierCalculator::CalculateBezier3(derPoints,v);
	}

	return result;
}

DirectX::XMFLOAT3 BezierPatch::GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir)
{
	DirectX::XMFLOAT3 result;
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;
	auto points = GetPatchPointPositions();

	switch (tangentDir)
	{
	case TangentDir::AlongU:

		// Get 4 second derivatives in u direction
		auto coeffs0 = BezierCalculator::GetSecondDerivativeCoefficients(points.row0);
		auto coeffs1 = BezierCalculator::GetSecondDerivativeCoefficients(points.row1);
		auto coeffs2 = BezierCalculator::GetSecondDerivativeCoefficients(points.row2);
		auto coeffs3 = BezierCalculator::GetSecondDerivativeCoefficients(points.row3);

		aux0 = BezierCalculator::CalculateBezier2(coeffs0.b210, coeffs0.b321, u);
		aux1 = BezierCalculator::CalculateBezier2(coeffs1.b210, coeffs1.b321, u);
		aux2 = BezierCalculator::CalculateBezier2(coeffs2.b210, coeffs2.b321, u);
		aux3 = BezierCalculator::CalculateBezier2(coeffs3.b210, coeffs3.b321, u);

		// Calculate value from 4 vectors
		result = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
		break;
	case TangentDir::AlongV:
		// calculate 4 points in u direction
		aux0 = BezierCalculator::CalculateBezier4(points.row0, u);
		aux1 = BezierCalculator::CalculateBezier4(points.row1, u);
		aux2 = BezierCalculator::CalculateBezier4(points.row2, u);
		aux3 = BezierCalculator::CalculateBezier4(points.row3, u);
		// calculate second derivative in the v direction based on 4 points
		auto coeffs = BezierCalculator::GetSecondDerivativeCoefficients(aux0, aux1, aux2, aux3);
		result = BezierCalculator::CalculateBezier2(coeffs.b210, coeffs.b321, v);
		break;
	}

	return result;
}

DirectX::XMFLOAT3 BezierPatch::GetSecondDarivativeMixed(float u, float v)
{
	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	auto points = GetPatchPointPositions();

	// Differentiate in  the u direction
	auto uDer0 = BezierCalculator::GetDerivativeCoefficients(points.row0);
	auto uDer1 = BezierCalculator::GetDerivativeCoefficients(points.row1);
	auto uDer2 = BezierCalculator::GetDerivativeCoefficients(points.row2);
	auto uDer3 = BezierCalculator::GetDerivativeCoefficients(points.row3);

	// Differentiate in  the v direction
	auto uvDer0 = BezierCalculator::GetDerivativeCoefficients(uDer0.b10, uDer1.b10, uDer2.b10, uDer3.b10);
	auto uvDer1 = BezierCalculator::GetDerivativeCoefficients(uDer0.b21, uDer1.b21, uDer2.b21, uDer3.b21);
	auto uvDer2 = BezierCalculator::GetDerivativeCoefficients(uDer0.b32, uDer1.b32, uDer2.b32, uDer3.b32);

	// Calculate one side of the Bezier tensor 
	auto finalCoeff0 = BezierCalculator::CalculateBezier3(uvDer0, v);
	auto finalCoeff1 = BezierCalculator::CalculateBezier3(uvDer1, v);
	auto finalCoeff2 = BezierCalculator::CalculateBezier3(uvDer2, v);

	// Calculate the other side of the Bezier tensor
	return BezierCalculator::CalculateBezier3(finalCoeff0, finalCoeff1, finalCoeff2, u);
}

bool BezierPatch::IsWrappedInDirection(SurfaceWrapDirection wrapDir)
{
	return wrapDir == SurfaceWrapDirection::None;
}

bool BezierPatch::ParamsInsideBounds(float u, float v)
{
	bool UinRange = u >= 0 && u <= 1.f;
	bool VinRange = v >= 0 && v <= 1.f;
	return (UinRange && VinRange);
}

ParameterPair BezierPatch::GetWrappedParams(float u, float v)
{
	return ParameterPair(u, v);
}

float BezierPatch::GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep)
{
	float res = defStep;
	DirectX::XMFLOAT2 params = DirectX::XMFLOAT2(u, v);
	DirectX::XMFLOAT2 movedParams = params + dir * defStep;

	if (ParamsInsideBounds(movedParams.x, movedParams.y) == false)
	{
		// check for nearest intersection with u and v edges
		// Check u edges
		if (dir.x != 0.0f)
		{
			float step0 = (0.f - u) / dir.x;
			float step1 = (1.f - u) / dir.x;

			if (step0 * defStep > 0 && abs(step0) < abs(res)) //has the same sign
			{
				res = step0;
			}

			if (step1 * defStep > 0 && abs(step1) < abs(res)) //has the same sign
			{
				res = step1;
			}
		}

		// Check v edges
		if (dir.y != 0.0f)
		{
			float step0 = (0.f - v) / dir.y;
			float step1 = (1.f - v) / dir.y;

			if (step0 * defStep > 0 && abs(step0) < abs(res)) //has the same sign
			{
				res = step0;
			}

			if (step1 * defStep > 0 && abs(step1) < abs(res)) //has the same sign
			{
				res = step1;
			}
		}

	}

	return res;
}
