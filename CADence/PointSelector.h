#pragma once
#include <memory>
#include "camera.h"
#include "Node.h"

class PointSelector
{
public:	
	explicit PointSelector(std::shared_ptr<Camera> camera);

	std::weak_ptr<Node> GetNearestPoint(int mouseX, int mouseY, std::vector<std::shared_ptr<Node>> nodes, int w, int h, float radius);
private:
	std::shared_ptr<Camera> m_camera;
};