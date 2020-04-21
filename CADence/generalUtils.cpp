#include "generalUtils.h"

std::weak_ptr<Node> FindObjectNode(std::vector<std::weak_ptr<Node>> nodesVector, Object& object)
{
	std::weak_ptr<Node> weakNode;

	for (int j = 0; j < nodesVector.size(); j++)
	{
		if (auto node = nodesVector[j].lock())
		{
			if (node->m_object.get() == &object)
			{
				weakNode = node;
			}
		}
	}

	return weakNode;
}

std::shared_ptr<Node> FindObjectNode(std::vector<std::shared_ptr<Node>> nodesVector, Object& object)
{
	std::shared_ptr<Node> sharedNode = nullptr;

	for (int j = 0; j < nodesVector.size(); j++)
	{		
		if (nodesVector[j]->m_object.get() == &object)
		{
			sharedNode = nodesVector[j];
		}		
	}

	return sharedNode;
}