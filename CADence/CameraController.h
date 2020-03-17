#pragma once
#include "camera.h"
#include <d3d11.h>
#include "imgui.h"
#include <memory>
class CameraController
{
public:
	CameraController();
	CameraController(std::shared_ptr<Camera> camera);
	void ProcessMessage(ImGuiIO * imGuiIO);

private:
	std::shared_ptr<Camera> m_camera;
	ImVec2 prevPos;
	bool m_captureRot, m_captureTrans;
};