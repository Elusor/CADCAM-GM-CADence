#pragma once
#include <memory>
#include "Scene.h"
#include "camera.h"
#include "Node.h"

class PointSelector
{
public:	
	explicit PointSelector(std::shared_ptr<Camera> camera);

	std::weak_ptr<Node> GetNearestPoint(int mouseX, int mouseY, std::vector<std::shared_ptr<Node>> nodes, int w, int h, float radius);
	std::vector<std::weak_ptr<Node>> GetAllPointsInArea(std::vector<std::shared_ptr<Node>> nodes, int width, int height);
	void ProcessInput(Scene* scene, SIZE windowSize);
	void StartCaptureMultiselect(int mouseX, int mouseY);
	void EndCaptureMultiselect(int mouseX, int mouseY);
	bool IsCapturing();
private:
	bool m_isCapturing;
	DirectX::XMFLOAT2 m_p0;
	DirectX::XMFLOAT2 m_p1;
	std::shared_ptr<Camera> m_camera;
};