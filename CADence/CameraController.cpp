#include "CameraController.h"

CameraController::CameraController() : CameraController(nullptr)
{}

CameraController::CameraController(Camera* camera)
{
	m_captureRot = m_captureTrans = false;
	m_camera = camera;
}

ImVec2 CameraController::ProcessMessage(ImGuiIO* imguiIO)
{
	imguiIO->ConfigDockingWithShift = true;
	float deltaX = 0;
	float deltaY = 0;

	float dispdeltaX = 0;
	float dispdeltaY = 0;

	if (imguiIO->MouseDown[2])
	{
		m_camera->ResetCamera();
	}	

	bool lDown = imguiIO->MouseDown[0];
	if (lDown)
	{
		auto pos = imguiIO->MousePos;
		dispdeltaX = pos.x;
		dispdeltaY = pos.y;
		m_captureTrans = true;
	}
	bool lUp = imguiIO->MouseReleased[0];
	if (lUp)
	{
		m_captureTrans = false;
	}
	if (m_captureTrans)
	{		
		auto pos = imguiIO->MousePos;
		deltaX = pos.x - prevPos.x;
		deltaY = pos.y - prevPos.y;
		m_camera->TranslateCamera(-deltaX, deltaY);
		if (deltaX != 0) dispdeltaX = deltaX;
		if (deltaY != 0) dispdeltaY = deltaY;
	}

	bool rDown = imguiIO->MouseDown[1];
	if (rDown)
	{				
		m_captureRot = true;
	}
	bool rUp = imguiIO->MouseReleased[1];
	if (rUp)
	{
		m_captureRot = false;
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
	return ImVec2(dispdeltaX, dispdeltaY);
}


