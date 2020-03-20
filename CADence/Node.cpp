#include "Node.h"
#include "imgui.h"

void Node::Render(std::unique_ptr<RenderData> & renderData)
{
	if (m_object != nullptr)
	{
		m_object->RenderObject(renderData, m_isSelected);
		/*if (m_isSelected)
		{
			m_object->RenderCoordinates(renderData, m_isSelected);
		}*/
	}	
}

void Node::Update()
{
	if (m_object != nullptr)
		m_object->UpdateObject();
}