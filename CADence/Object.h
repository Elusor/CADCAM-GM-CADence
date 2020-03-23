#pragma once
#include <string>
#include "Transform.h"
#include "renderState.h"

struct Object {
	std::string m_name = "";
	std::string m_defaultName = "";
	Transform m_transform;

	virtual void RenderObject(std::unique_ptr<RenderState>& renderState);
	virtual void RenderCoordinates(std::unique_ptr<RenderState>& renderState);
	virtual void UpdateObject();
	virtual bool CreateParamsGui();
};