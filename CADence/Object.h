#pragma once
#include <memory>
#include <string>
#include "Transform.h"
#include "Renderer.h"
struct Object 
{
	std::string m_name = "";
	std::string m_defaultName = "";
	Transform m_transform;
	Renderer m_renderer;

	virtual void RenderObject();
	virtual void RenderCoordinates();


	virtual void UpdateObject();
	virtual bool CreateParamsGui();
};