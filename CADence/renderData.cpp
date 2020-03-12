#include "renderData.h"

RenderData::RenderData(mini::Window& windows, Camera* camera): m_device(windows)
{
	m_camera = camera;
}
