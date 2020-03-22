#include "Node.h"
#include "imgui.h"

void Node::Render()
{
	if (m_object != nullptr)
	{
		m_object->RenderObject();
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