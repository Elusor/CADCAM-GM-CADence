#include "CameraController.h"
CameraController::CameraController() : CameraController(nullptr)
{}

CameraController::CameraController(Camera* camera)
{
	m_captureRot = m_captureTrans = false;
	m_camera = camera;
}

void CameraController::ProcessMessage(ImGuiIO* imguiIO)
{	
	float deltaX = 0;
	float deltaY = 0;

	bool lDown = imguiIO->MouseDown[0];
	bool lUp = imguiIO->MouseReleased[0];
	bool rDown = imguiIO->MouseDown[1];
	bool rUp = imguiIO->MouseReleased[1];
	bool mDown = imguiIO->MouseDown[2];
	
	if (mDown)
	{
		m_camera->ResetCamera();
	}	

	if (lDown && imguiIO->WantCaptureMouse == false)
	{
		auto pos = imguiIO->MousePos;
		m_captureTrans = true;
	}

	if (lUp)
	{
		m_captureTrans = false;
	}

	if (rDown)
	{				
		m_captureRot = true;
	}

	if (rUp)
	{
		m_captureRot = false;
	}

	if (m_captureTrans)
	{
		auto pos = imguiIO->MousePos;
		deltaX = pos.x - prevPos.x;
		deltaY = pos.y - prevPos.y;
		m_camera->TranslateCamera(-deltaX, deltaY);
	}

	if (m_captureRot)
	{
		auto pos = imguiIO->MousePos;
		deltaX = pos.x - prevPos.x;
		deltaY = pos.y - prevPos.y;		
		m_camera->RotateCamera(-deltaX, deltaY);
	}

	if (imguiIO->MouseWheel != 0)
		m_camera->CameraZoom(imguiIO->MouseWheel);

	prevPos = imguiIO->MousePos;
}


