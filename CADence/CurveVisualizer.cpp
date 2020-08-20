#include "CurveVisualizer.h"
#include "vertexStructures.h"
#include "mathUtils.h"

#include "Trimmer.h"

CurveVisualizer::CurveVisualizer(GuiManager* manager, RenderState* renderState, int width, int height)
{
	// Initialize the textures (1 for each surface)
	auto result = InitializeTextures(renderState->m_device.m_device.get(), width, height);
	
	dbg_trimmer = new Trimmer();

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Height = height;
	viewport.Width = width;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	m_viewPort = viewport;
	m_renderState = renderState;
	guiManager = manager;
	m_width = width;
	m_height = height;
	assert(result == true);
}

CurveVisualizer::~CurveVisualizer()
{
	if (m_shaderResourceView1)
	{
		m_shaderResourceView1->Release();
		m_shaderResourceView1 = 0;
	}

	if (m_renderTargetView1)
	{
		m_renderTargetView1->Release();
		m_renderTargetView1 = 0;
	}

	if (m_renderTargetTexture1)
	{
		m_renderTargetTexture1->Release();
		m_renderTargetTexture1 = 0;
	}

	if (m_shaderResourceView2)
	{
		m_shaderResourceView2->Release();
		m_shaderResourceView2 = 0;
	}

	if (m_renderTargetView2)
	{
		m_renderTargetView2->Release();
		m_renderTargetView2 = 0;
	}

	if (m_renderTargetTexture2)
	{
		m_renderTargetTexture2->Release();
		m_renderTargetTexture2 = 0;
	}

	return;
}

void CurveVisualizer::VisualizeCurve(IntersectionCurve* curve)
{
	
	// Get Parameter lists from the curve
	auto params1 = curve->GetParameterList(IntersectedSurface::SurfaceP);
	auto params2 = curve->GetParameterList(IntersectedSurface::SurfaceQ);

	// Render the image onto the member texture
	/*RenderImage(m_renderTargetView1, m_shaderResourceView1, params1);
	RenderImage(m_renderTargetView2, m_shaderResourceView2, params2);*/
	RenderTrimmedSpace(m_renderTargetView1, m_shaderResourceView1, params1);
	RenderTrimmedSpace(m_renderTargetView2, m_shaderResourceView2, params2);

	// Call a new Imgui Window with texture section
	guiManager->EnableDoubleTextureWindow("", "Intersection curve in parameter space", m_shaderResourceView1, m_width, m_height, m_shaderResourceView2, m_width, m_height);
}

ID3D11ShaderResourceView* CurveVisualizer::GetShaderResourceView(IntersectedSurface affectedSurface)
{
	ID3D11ShaderResourceView* srv;
	
	if (affectedSurface == IntersectedSurface::SurfaceQ)
	{
		srv = m_shaderResourceView1;
	}
	else {
		srv = m_shaderResourceView2;
	}

	return srv;
}

ID3D11RenderTargetView* CurveVisualizer::GetRenderTargetView(IntersectedSurface affectedSurface)
{
	ID3D11RenderTargetView* rtv;

	if (affectedSurface == IntersectedSurface::SurfaceQ)
	{
		rtv = m_renderTargetView1;
	}
	else {
		rtv = m_renderTargetView2;
	}

	return rtv;
}

std::vector<DirectX::XMFLOAT3> CurveVisualizer::GetInterpolationCurvePoints(std::vector<DirectX::XMFLOAT2> parameters)
{
	std::vector<DirectX::XMFLOAT3> interpolationPoints;
	std::vector<DirectX::XMFLOAT3> virtualPoints;

	for (auto params : parameters)
	{
		interpolationPoints.push_back(DirectX::XMFLOAT3(params.x, params.y, 0.0f));
	}

	std::vector<float> upperDiag, diag, lowerDiag;
	std::vector<float> xVector, yVector, zVector;
	std::vector<float> distances;

	std::vector<DirectX::XMFLOAT3> a, b, c, d;

	for (int i = 0; i < interpolationPoints.size() - 1; i++)
	{
		auto p1 = interpolationPoints[i];
		auto p2 = interpolationPoints[i + 1];
		// DISTANCES FROM d0 to dk-1, where k is knots count
		float dist = GetDistanceBetweenPoints(p2, p1);
		dist = max(dist, 0.001f);
		distances.push_back(dist);
	}

	for (int i = 0; i < interpolationPoints.size() - 2; i++)
	{
		diag.push_back(2.0f);
	}

	for (int i = 1; i < interpolationPoints.size() - 1; i++)
	{
		float dist = distances[i];
		float distPrev = distances[i - 1];
		float denom = dist + distPrev;

		// calculate R's alphas (lower diag) and betas (upper diag)				
		if (i == 1)
		{
			lowerDiag.push_back(0.f);
		}
		else
		{
			float alphai = distances[i - 1]; //TODO
			alphai /= denom;
			lowerDiag.push_back(alphai);
		}

		if (i == interpolationPoints.size() - 2)
		{
			upperDiag.push_back(0.f);
		}
		else
		{
			float betai = distances[i]; //TODO
			betai /= denom;
			upperDiag.push_back(betai);
		}

		// calculate and assing Ri
		auto P = interpolationPoints[i];
		auto Pnext = interpolationPoints[i + 1];
		auto Pprev = interpolationPoints[i - 1];

		auto diff = XMF3TimesFloat(XMF3SUB(Pnext, P), 1.f / dist);
		auto diffPrev = XMF3TimesFloat(XMF3SUB(P, Pprev), 1.f / distPrev);
		auto R = XMF3SUB(diff, diffPrev);
		R = XMF3TimesFloat(R, 3.f / denom);
		xVector.push_back(R.x);
		yVector.push_back(R.y);
		zVector.push_back(R.z);
	}

	std::vector<float> xRes, yRes, zRes;

	if (xVector.size() > 1)
	{
		xRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, xVector);
		yRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, yVector);
		zRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, zVector);
	}
	else
	{
		if (xVector.size() == 1)
		{
			xRes.push_back(xVector[0] / 2.f);
			yRes.push_back(yVector[0] / 2.f);
			zRes.push_back(zVector[0] / 2.f);
		}
	}

	std::vector<DirectX::XMFLOAT3> resultPos;
	if (interpolationPoints.size() > 2)
	{
		DirectX::XMFLOAT3 zero = { 0.f, 0.f, 0.f };

		auto p0 = interpolationPoints[0];
		auto p1 = interpolationPoints[1];

		c.push_back(zero);
		a.push_back(p0);
		//Xres size is K - 2
		for (int i = 0; i < xRes.size(); i++)
		{
			c.push_back(DirectX::XMFLOAT3(xRes[i], yRes[i], zRes[i]));
			a.push_back(interpolationPoints[i + 1]);
			//d.push_back(interpolationKnots[i + 2].lock()->m_object->GetPosition());
		}

		auto plast = interpolationPoints[interpolationPoints.size() - 1];
		c.push_back(zero);
		a.push_back(plast);

		// d can be shorter by 1 than a
		//calculate d's
		for (int i = 1; i < c.size(); i++)
		{
			float distPrev = distances[i - 1];
			XMFLOAT3 cDiff = XMF3SUB(c[i], c[i - 1]);
			XMFLOAT3 dmod = XMF3TimesFloat(cDiff, 1.f / (3.f * distPrev));
			d.push_back(dmod);
		}

		for (int i = 1; i < c.size(); i++)
		{
			float distPrev = distances[i - 1];
			float dpSqr = distPrev * distPrev;
			float dpCube = dpSqr * distPrev;
			XMFLOAT3 bmod;
			XMFLOAT3 aDiff = XMF3SUB(a[i], a[i - 1]);
			XMFLOAT3 dModified = XMF3TimesFloat(d[i - 1], dpCube);
			XMFLOAT3 cModified = XMF3TimesFloat(c[i - 1], dpSqr);
			bmod = XMF3SUB(aDiff, dModified);
			bmod = XMF3SUB(bmod, cModified);
			bmod = XMF3TimesFloat(bmod, 1.f / distPrev);
			b.push_back(bmod);
		}

		for (int i = 0; i < b.size(); i++)
		{
			float dist = distances[i];
			a[i] = XMF3TimesFloat(a[i], 1.f);
			b[i] = XMF3TimesFloat(b[i], dist);
			c[i] = XMF3TimesFloat(c[i], dist * dist);
			d[i] = XMF3TimesFloat(d[i], dist * dist * dist);

		}

		// convert abcd to bernstein basis
		for (int i = 0; i < b.size(); i++)
		{
			DirectX::XMFLOAT4X4 resMat;
			DirectX::XMFLOAT4X4 mtx = {
				a[i].x, b[i].x, c[i].x, d[i].x,
				a[i].y, b[i].y, c[i].y, d[i].y,
				a[i].z, b[i].z, c[i].z, d[i].z,
				0.f, 0.f, 0.f, 0.f
			};

			DirectX::XMMATRIX vectorMat = DirectX::XMLoadFloat4x4(&mtx);

			auto res = vectorMat * DirectX::XMLoadFloat4x4(&m_changeBasisMtx);

			DirectX::XMStoreFloat4x4(&resMat, (res));
			auto k1 = XMFLOAT3(resMat._11, resMat._21, resMat._31);
			auto k2 = XMFLOAT3(resMat._12, resMat._22, resMat._32);
			auto k3 = XMFLOAT3(resMat._13, resMat._23, resMat._33);
			auto k4 = XMFLOAT3(resMat._14, resMat._24, resMat._34);

			resultPos.push_back(k1);
			resultPos.push_back(k2);
			resultPos.push_back(k3);
		}

		resultPos.push_back(interpolationPoints[interpolationPoints.size() - 1]);
		// Each segmenent is built from 4 points, the middle points are the same
	}
	else
	{
		XMFLOAT3 pos0 = interpolationPoints[0];
		XMFLOAT3 pos1 = interpolationPoints[1];

		resultPos.push_back(pos0);
		resultPos.push_back(F3Lerp(pos0, pos1, 1.f / 3.f));
		resultPos.push_back(F3Lerp(pos0, pos1, 2.f / 3.f));
		resultPos.push_back(pos1);
	}

	return resultPos;
}

void CurveVisualizer::RenderImage(ID3D11RenderTargetView* texture, ID3D11ShaderResourceView* srv, std::vector<DirectX::XMFLOAT2> paramList)
{
	auto context = m_renderState->m_device.m_context.get();
	auto device = m_renderState->m_device.m_device.get();

	UINT originalVPCount = 1;
	// Probe the current viewport count
	context->RSGetViewports(&originalVPCount, NULL);
	// Get current viewport
	D3D11_VIEWPORT* originalVPs = new D3D11_VIEWPORT[originalVPCount];
	context->RSGetViewports(&originalVPCount, originalVPs);
	
	// Set visualizer viewport
	context->RSSetViewports(1, &m_viewPort);

	// Clear the previous texture
	ClearTexture(texture, context, m_renderState->m_depthBuffer.get(), 1.f, 1.f, 1.f, 1.f);

	context->OMSetRenderTargets(1, &texture, m_renderState->m_depthBuffer.get());

	context->VSSetShader(m_renderState->m_paramSpaceVS.get(), nullptr, 0);
	context->PSSetShader(m_renderState->m_paramSpacePS.get(), nullptr, 0);

	auto bezierPoints = GetInterpolationCurvePoints(paramList);
	auto curveDesc = PrepareCurveDrawingData(bezierPoints);

	std::vector<VertexPositionColor> positions;
	std::vector<unsigned short> indices;

	int k = indices.size();
	DirectX::XMFLOAT3 gridColor = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
	
	// Set up grid points
	for (float u = 0.1f; u < 1.f; u += 0.1f)
	{
		positions.push_back(VertexPositionColor{
			DirectX::XMFLOAT3(u, 0.0f, 0.2f),
			gridColor});
		positions.push_back(VertexPositionColor{
			DirectX::XMFLOAT3(u, 1.0f, 0.2f),
			gridColor });

		indices.push_back(k);
		indices.push_back(k+1);
		k += 2;
	}
	
	for (float u = 0.1f; u < 1.f; u += 0.1f)
	{
		positions.push_back(VertexPositionColor{
			DirectX::XMFLOAT3(0.0f, u , 0.2f),
			gridColor });
		positions.push_back(VertexPositionColor{
			DirectX::XMFLOAT3(1.0f, u, 0.2f),
			gridColor });
		indices.push_back(k);
		indices.push_back(k + 1);
		k += 2;
	}
	
	auto vertices = m_renderState->m_device.CreateVertexBuffer(positions);
	auto indicesBuf = m_renderState->m_device.CreateIndexBuffer(indices);

	// Draw grid
	ID3D11Buffer* vbs[] = { vertices.get() };
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };
	m_renderState->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	m_renderState->m_device.context()->IASetIndexBuffer(indicesBuf.get(), DXGI_FORMAT_R16_UINT, 0);
	m_renderState->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_renderState->m_device.m_context->DrawIndexed(indices.size(), 0, 0);

	// Draw curve
	vertices = m_renderState->m_device.CreateVertexBuffer(curveDesc.vertices);
	indicesBuf = m_renderState->m_device.CreateIndexBuffer(curveDesc.indices);

	// Set up GS buffer
	DirectX::XMFLOAT4 data = DirectX::XMFLOAT4(20 / 20, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR GSdata = DirectX::XMLoadFloat4(&data);
	auto buf = m_renderState->SetConstantBuffer<DirectX::XMVECTOR>(m_renderState->m_cbGSData.get(), GSdata);
	ID3D11Buffer* cbs[] = { buf };
	m_renderState->m_device.context()->GSSetConstantBuffers(0, 1, cbs);

	// Turn on bezier geometry shader
	m_renderState->m_device.context()->GSSetShader(m_renderState->m_bezierGeometryShader.get(), nullptr, 0);
	// turn off bezier geometry shader

	ID3D11Buffer* vbs2[] = { vertices.get() };
	m_renderState->m_device.context()->IASetVertexBuffers(0, 1, vbs2, strides, offsets);
	m_renderState->m_device.context()->IASetIndexBuffer(indicesBuf.get(), DXGI_FORMAT_R16_UINT, 0);
	m_renderState->m_device.context()->IASetPrimitiveTopology(curveDesc.m_primitiveTopology);
	m_renderState->m_device.m_context->DrawIndexed(curveDesc.indices.size(), 0, 0);

	// Reset viewport
	context->RSSetViewports(originalVPCount, originalVPs);
	delete[] originalVPs;

	// Reset GS
	m_renderState->m_device.context()->GSSetShader(nullptr, nullptr, 0);
}

void CurveVisualizer::RenderTrimmedSpace(ID3D11RenderTargetView* texture, ID3D11ShaderResourceView* srv, std::vector<DirectX::XMFLOAT2> paramList)
{
	auto context = m_renderState->m_device.m_context.get();
	auto device = m_renderState->m_device.m_device.get();
	UINT originalVPCount = 0;
	
	// Probe the current viewport count
	context->RSGetViewports(&originalVPCount, NULL);
	// Get current viewport
	D3D11_VIEWPORT* originalVPs = new D3D11_VIEWPORT[originalVPCount];
	context->RSGetViewports(&originalVPCount, originalVPs);

	// Set visualizer viewport
	context->RSSetViewports(1, &m_viewPort);

	// Clear the previous texture
	ClearTexture(texture, context, m_renderState->m_depthBuffer.get(), 1.f, 1.f, 1.f, 1.f);

	context->OMSetRenderTargets(1, &texture, m_renderState->m_depthBuffer.get());
	context->VSSetShader(m_renderState->m_paramSpaceVS.get(), nullptr, 0);
	context->PSSetShader(m_renderState->m_paramSpacePS.get(), nullptr, 0);

	std::vector<VertexPositionColor> positions;
	std::vector<unsigned short> indices;

	DirectX::XMFLOAT3 gridColor = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);


	// Set up grid points
	//for (float u = 0.1f; u < 1.f; u += 0.1f)
	//{
	//	positions.push_back(VertexPositionColor{
	//		DirectX::XMFLOAT3(u, 0.0f, 0.2f),
	//		gridColor});
	//	positions.push_back(VertexPositionColor{
	//		DirectX::XMFLOAT3(u, 1.0f, 0.2f),
	//		gridColor });

	//	indices.push_back(k);
	//	indices.push_back(k+1);
	//	k += 2;
	//}
	//
	//for (float u = 0.1f; u < 1.f; u += 0.1f)
	//{
	//	positions.push_back(VertexPositionColor{
	//		DirectX::XMFLOAT3(0.0f, u , 0.2f),
	//		gridColor });
	//	positions.push_back(VertexPositionColor{
	//		DirectX::XMFLOAT3(1.0f, u, 0.2f),
	//		gridColor });
	//	indices.push_back(k);
	//	indices.push_back(k + 1);
	//	k += 2;
	//}

	TrimmedSpace trim = dbg_trimmer->Trim(paramList, 11, 11);

	// Add the trimmed space vertices
	for (auto pair : trim.vertices)
	{
		positions.push_back(VertexPositionColor{
			DirectX::XMFLOAT3(pair.x, pair.y, 0.1f),
			DirectX::XMFLOAT3(0.f,0.f,0.f) });
	}

	int currentIndexOffset = indices.size();
	for (auto ind : trim.indices)
	{
		indices.push_back(currentIndexOffset + ind);
	}

	auto vertices = m_renderState->m_device.CreateVertexBuffer(positions);
	auto idxBuff = m_renderState->m_device.CreateIndexBuffer(indices);

	// Draw grid
	ID3D11Buffer* vertBuff[] = { vertices.get() };
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };
	m_renderState->m_device.context()->IASetVertexBuffers(0, 1, vertBuff, strides, offsets);
	m_renderState->m_device.context()->IASetIndexBuffer(idxBuff.get(), DXGI_FORMAT_R16_UINT, 0);
	m_renderState->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_renderState->m_device.m_context->DrawIndexed(indices.size(), 0, 0);

	// Reset viewport
	context->RSSetViewports(originalVPCount, originalVPs);
	delete[] originalVPs;
}

void CurveVisualizer::ClearTexture(ID3D11RenderTargetView* texture,
	ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencil,
	float r, float g, float b, float alpha, float depth)
{
	float color[4] = { r, g, b, alpha };
	context->ClearRenderTargetView(texture, color);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0.f);
}

bool CurveVisualizer::InitializeTextures(ID3D11Device* device, int width, int height)
{

	// initialize the texture 

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Set up texture description
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1; // Implement multisampling later
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture1);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture2);
	if (FAILED(result))
	{
		return false;
	}

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_renderTargetTexture1, &renderTargetViewDesc, &m_renderTargetView1);	
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateRenderTargetView(m_renderTargetTexture2, &renderTargetViewDesc, &m_renderTargetView2);
	if (FAILED(result))
	{
		return false;
	}

	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_renderTargetTexture1, &shaderResourceViewDesc, &m_shaderResourceView1);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateShaderResourceView(m_renderTargetTexture2, &shaderResourceViewDesc, &m_shaderResourceView2);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

MeshDescription CurveVisualizer::PrepareCurveDrawingData(std::vector<DirectX::XMFLOAT3> bezierPositions)
{	
	auto lastVertexDuplicationCount = 0;
	DirectX::XMFLOAT3 curveColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	MeshDescription desc;

	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;

	for (int i = 0; i < bezierPositions.size(); i++)
	{
		//add all vertices
		auto pos = bezierPositions[i];
		vertices.push_back(VertexPositionColor{
					pos,
					curveColor});
	}

	for (int i = 0; i < bezierPositions.size(); i += 3)
	{
		// mage edges out of the vertices
		// There are some elements that should be added
		int remVerts = bezierPositions.size() - 1 - i;
		if (remVerts > 0)
		{
			// add next 4 points normally
			if (bezierPositions.size() - i >= 4)
			{
				indices.push_back(i);
				indices.push_back(i + 1);
				indices.push_back(i + 2);
				indices.push_back(i + 3);
			}
			// add the rest of the nodes and add the last node multiple times
			else {
				for (int j = i; j < bezierPositions.size(); j++)
				{
					indices.push_back(j);
				}

				// add the rest of the vertices as duplicates of the last one
				int emptyVertices = 4 - (bezierPositions.size() - i);
				//m_lastVertexDuplicationCount = emptyVertices;
				for (int k = 0; k < emptyVertices; k++)
				{
					indices.push_back(bezierPositions.size() - 1);
				}
			}

		}
	}

	desc.vertices = vertices;
	desc.indices = indices;
	desc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;

	return desc;
}
