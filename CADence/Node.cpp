#include "Node.h"
#include "imgui.h"

void Node::Render(std::unique_ptr<RenderData> & renderData, bool selected)
{
	if (m_object != nullptr)
	{
		m_object->RenderObject(renderData, selected);
		if (selected)
		{
			m_object->RenderCoordinates(renderData, selected);
		}
	}
		
}

void Node::Update()
{
	if (m_object != nullptr)
		m_object->UpdateObject();
}