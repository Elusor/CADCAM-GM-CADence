#include "Scene.h"
#include "imgui.h"
#include "ObjectFactory.h"

Scene::Scene()
{
	m_objectFactory = std::unique_ptr<ObjectFactory>(new ObjectFactory());
	m_spawnMarker = m_objectFactory->CreateSpawnMarker();

	Object* middleMarker = new Object();
	m_middleMarker = std::unique_ptr<Object>(middleMarker);
}

void Scene::AttachObject(std::unique_ptr<Object>& object)
{
	Node* newNode = new Node();
	newNode->m_object = move(object);
	m_nodes.push_back(std::shared_ptr<Node>(newNode));
}

void Scene::RemoveObject(std::unique_ptr<Object>& object)
{	
	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (m_nodes[i]->m_object == object)
		{			
			m_nodes[i]->m_object.reset();
			m_nodes[i].reset();
			m_nodes.erase(m_nodes.begin() + i);			
		}
	}
}

void Scene::GetMiddleMarkerCoords()
{
	if (m_selectedNodes.size() > 0)
	{
		int count = m_selectedNodes.size();
		DirectX::XMVECTOR pos = DirectX::XMVectorZero();

		for (int j = 0; j < m_selectedNodes.size(); j++)
		{
			if (auto node = m_selectedNodes[j].lock())
			{
				DirectX::XMVECTOR posj = DirectX::XMLoadFloat3(&(node->m_object->m_transform.m_pos));
				pos = DirectX::XMVectorAdd(pos, posj);
			}
		}

		float countf = (float)count;

		DirectX::XMFLOAT3 newPos;
		DirectX::XMStoreFloat3(&newPos, pos);

		newPos.x /= countf;
		newPos.y /= countf;
		newPos.z /= countf;

		m_middleMarker->m_transform.m_pos = newPos;		
	}
}

void Scene::ClearScene()
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->m_object.reset();
		m_nodes[i].reset();
	}

	m_nodes.clear();
	m_selectedNodes.clear();
}

void Scene::DrawScenePopupMenu()
{
	if (ImGui::BeginPopupContextItem("item context menu"))
	{
		if (ImGui::BeginMenu("Add child object"))
		{
			if (ImGui::MenuItem("Torus"))
			{
				std::unique_ptr<Object> obj = m_objectFactory->CreateTorus(m_spawnMarker->m_transform);
				AttachObject(obj);
			}

			if (ImGui::MenuItem("Point"))
			{
				std::unique_ptr<Object> obj = m_objectFactory->CreatePoint(m_spawnMarker->m_transform);
				AttachObject(obj);

			}
			ImGui::EndMenu();
		}

		if (ImGui::Selectable("Clear scene"))
		{
			ClearScene();
		}
		ImGui::EndPopup();
	}
}

void Scene::DrawNodePopupMenu(const std::shared_ptr<Node> node)
{	
	std::string hashes = "##";
	std::string labelName = node->m_object->m_name + hashes + node->m_object->m_defaultName;

	if (node->m_isRenamed)
	{
		std::string name = "##input";

		int size = 30;
		char* text = new char[size];
		memset(text, 0, sizeof(text));
		node->m_object->m_name.copy(text, node->m_object->m_name.size() + 1);
		text[node->m_object->m_name.size()] = '\0';
		bool entered = ImGui::InputText(name.c_str(), text, sizeof(char) * size, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SetItemDefaultFocus();
		ImGui::SetKeyboardFocusHere(-1);
		if (entered)
		{
			node->m_object->m_name = text;
			node->m_isRenamed = false;
		}	
		delete [] text;
	}
	else
	{
		ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (node->m_isSelected)
		{
			leaf_flags |= ImGuiTreeNodeFlags_Selected;
		}
		

		ImGui::TreeNodeEx(labelName.c_str(), leaf_flags);

		// Process the click of the tree node 
		if (ImGui::IsItemClicked())
		{
			// If ctrl is not pressed
			// select only the clicked node
			if (ImGui::GetIO().KeyCtrl == false)
			{
				// clear selected nodes
				for (int i = 0; i < m_nodes.size(); i++)
				{
					m_nodes[i]->m_isSelected = false;
				}
				m_selectedNodes.clear();

				// select this node
				node->m_isSelected = true;
				std::weak_ptr<Node> weakNode = node;
				m_selectedNodes.push_back(weakNode);
			}
			else
			{
				// Ctrl is pressed
				// Add not selected node to selection or
				// Remove selected node from selection
				if (node->m_isSelected)
				{
					// deselect this node and
					// remove this node from m_selectedNodes
					node->m_isSelected = false;
					auto it = m_selectedNodes.begin();
					while (it != m_selectedNodes.end())
					{
						if (auto selectedNode = it->lock())
						{
							if (selectedNode->m_object == node->m_object)
							{
								it = m_selectedNodes.erase(it);
								break;
							}
							else
							{
								it++;
							}
						}
					}
				}
				else
				{
					// select this node
					node->m_isSelected = true;
					std::weak_ptr<Node> weakNode = node;
					m_selectedNodes.push_back(weakNode);
				}

			}
		}

		// Add unique popup id generator
		if (ImGui::BeginPopupContextItem(labelName.c_str()))
		{

			if (ImGui::Selectable("Rename object"))
			{
				node->m_isRenamed = true;
				// Trigger a popup for renaming objects - probably check if name is availible through name registry
			}

			if (ImGui::Selectable("Remove object"))
			{
				RemoveObject(node->m_object);
			}

			ImGui::EndPopup();
		}
	}
	
}

void Scene::DrawSceneHierarchy()
{
	bool node_open = ImGui::TreeNode("Scene");	
	
	DrawScenePopupMenu();

	if (node_open)
	{
		if (m_nodes.size() != 0)
		{
			for (int i = 0; i < m_nodes.size(); i++)
			{
				DrawNodePopupMenu(m_nodes[i]);
			}
		}
		else 
		{
			ImGui::TreeNodeEx("<No objects in scene>", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		}		

		ImGui::TreePop();
	}	
}

void Scene::DrawSceneGUI()
{
	if (ImGui::CollapsingHeader("Hierarchy"))
	{
		DrawSceneHierarchy();
		ImGui::Spacing();
	}
	if (ImGui::CollapsingHeader("Cursor"))
	{
		m_spawnMarker->CreateParamsGui();
	}
}

void Scene::ProcessInput()
{
}

void Scene::RenderScene(std::unique_ptr<RenderState>& renderData)
{
	m_spawnMarker->RenderObject(renderData);	
	
	for (int i = 0; i < m_nodes.size(); i++)
	{
		// TODO [MG] : check if this item is currently selected		
		m_nodes[i]->Render(renderData);
	}		

	if (m_selectedNodes.size() > 0)
	{
		m_middleMarker->RenderCoordinates(renderData);
	}	
}

std::vector<std::weak_ptr<IRenderable>> Scene::GetItemsToRender()
{

	std::vector<std::weak_ptr<IRenderable>> result;

	for (int i = 0; i < m_nodes.size(); i++)
	{
		std::weak_ptr<IRenderable> node = m_nodes[i];
	}

	return result;
}

void Scene::UpdateScene()
{
	// Currently not in use - if necessary implement setting active objects (i.e. rotating etc.)
	// as dirty and run object->Update only on dirty objects
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->Update();
	}
}

void Scene::UpdateSelectedNodes()
{	
	// Draw the inspector window
	// foreach selected node
	//    Draw section for each node

	auto it = m_selectedNodes.begin();
	while (it != m_selectedNodes.end())
	{
		// TODO [MG] : add collapsing headers
		if (auto selectedNode = it->lock())
		{
			if (selectedNode->m_object)
			{
				bool selectedObjectModified = selectedNode->m_object->CreateParamsGui();

				if (selectedObjectModified)
				{
					selectedNode->Update();
				}
			}
			it++;
		}
		else {
			it = m_selectedNodes.erase(it);
		}		
	}
}
