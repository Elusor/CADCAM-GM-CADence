#pragma once
#include "camera.h"
#include <memory>

static class CameraRegistry
{
public:
	static std::shared_ptr<Camera> currentCamera;
};