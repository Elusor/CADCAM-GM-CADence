#include "Node.h"
#include "imgui.h"

void Node::Render(std::unique_ptr<RenderData> & renderData)
{
	if (m_object != nullptr)
		m_object->RenderObject(renderData);	
}

void Node::Update()
{
	if (m_object != nullptr)
		m_object->UpdateObject();
}