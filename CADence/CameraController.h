#pragma once
#include "camera.h"
#include <d3d11.h>
#include "imgui.h"

class CameraController
{
public:
	CameraController();
	CameraController(Camera* camera);
	void ProcessMessage(ImGuiIO * imGuiIO);

private:
	Camera* m_camera;
	ImVec2 prevPos;
	bool m_captureRot, m_captureTrans;
};