#pragma once
#include "windowApplication.h"
#include "dxDevice.h"
#include "mathStructures.h"
#include "cadStructures.h"
#include <DirectXMath.h>

class DxApplication : public mini::WindowApplication
{
public:
	explicit DxApplication(HINSTANCE hInstance);	

protected:
	int MainLoop() override;
	
private:
	void Clear();
	void Render();
	void InitImguiWindows();
	void Update();

	DxDevice m_device;

	SurfaceObject* m_surObj;	

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
