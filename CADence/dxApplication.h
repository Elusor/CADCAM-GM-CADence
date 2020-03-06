#pragma once
#include "windowApplication.h"
#include "dxDevice.h"
#include "mathStructures.h"
#include <DirectXMath.h>

class DxApplication : public mini::WindowApplication
{
public:
	explicit DxApplication(HINSTANCE hInstance);
	
	std::vector<DirectX::XMFLOAT2> CreateTriangleVertices() {
		
		DirectX::XMFLOAT2 p1, p2, p3;		
		p1.x = -0.8f;
		p1.y = -0.8f;

		p2.x = 0;
		p2.y = 0.7f;

		p3.x = 0.8f;
		p3.y = -0.8f;

		std::vector<DirectX::XMFLOAT2> vector;		
		vector.push_back(p1);
		vector.push_back(p2);
		vector.push_back(p3);
		return vector;
	}	
	

protected:
	int MainLoop() override;

private:
	void Render();
	void Update();

	DxDevice m_device;

	static std::vector<VertexPositionColor> CreateCubeVertices();
	static std::vector<unsigned short> CreateCubeIndices();

	//Holds resources used by the pipeline
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11InputLayout> m_layout;

	DirectX::XMFLOAT4X4 m_modelMat, m_viewMat, m_projMat;
	mini::dx_ptr<ID3D11Buffer> m_cbMVP;
};
