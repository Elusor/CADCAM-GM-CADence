#pragma once
#include "camera.h"
#include <memory>

class CameraRegistry
{
public:
	std::shared_ptr<Camera> currentCamera;
};