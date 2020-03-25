#pragma once
#include <string>
#include "Transform.h"
#include "renderState.h"

class Scene;
class Node; 

struct Object {
	std::string m_name = "";
	std::string m_defaultName = "";
	std::weak_ptr<Node> m_parent;
	Transform m_transform;

	virtual void RenderObject(std::unique_ptr<RenderState>& renderState);
	virtual void RenderCoordinates(std::unique_ptr<RenderState>& renderState);
	virtual void RenderObjectSpecificContextOptions(Scene& scene);
	virtual void UpdateObject();
	virtual bool CreateParamsGui();
};