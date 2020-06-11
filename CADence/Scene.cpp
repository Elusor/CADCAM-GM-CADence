#include "Scene.h"
#include "imgui.h"
#include "ObjectFactory.h"
#include "GroupNode.h"
#include "generalUtils.h"

Scene::Scene()
{
	m_objectFactory = std::unique_ptr<ObjectFactory>(new ObjectFactory());
	std::unique_ptr<Object> spawnMarker = move(m_objectFactory->CreateSpawnMarker()->m_object);
	m_spawnMarker = std::unique_ptr<SpawnMarker>(dynamic_cast<SpawnMarker *>(spawnMarker.release()));

	std::unique_ptr<Object> grid = move(m_objectFactory->CreateSceneGrid(50)->m_object);
	m_grid = std::unique_ptr<SceneGrid>(dynamic_cast<SceneGrid*>(grid.release()));

	Object* middleMarker = new Object();
	m_middleMarker = std::unique_ptr<Object>(middleMarker);
}

void Scene::AttachObject(std::shared_ptr<Node> node)
{
	m_nodes.push_back(std::shared_ptr<Node>(node));
}

void Scene::RemoveObject(std::unique_ptr<Object>& object)
{	
	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (m_nodes[i]->m_object == object)
		{
			//object->Cleanup();
			object.reset();
			m_nodes[i].reset();
			m_nodes.erase(m_nodes.begin() + i);			
		}
	}
}

void Scene::ClearScene()
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->m_object.reset();
		m_nodes[i].reset();
	}
	m_objectFactory->ClearScene();
	m_nodes.clear();
	m_selectedNodes.clear();
}

void Scene::DrawScenePopupMenu()
{
	bool imGuiWorkaroundSuggestedByItsAuthor = false;
	bool imGuiWorkaroundSuggestedByItsAuthorC2 = false;
	if (ImGui::BeginPopupContextItem("item context menu##ScenePopup"))
	{
		if (ImGui::BeginMenu("Add child object##ScenePopup"))
		{
			if (ImGui::MenuItem("Torus##ScenePopup"))
			{
				AttachObject(m_objectFactory->CreateTorus(m_spawnMarker->GetTransform()));
			}
			
			if (ImGui::MenuItem("Bezier Surface##ScenePopup"))
			{
				//m_objectFactory->CreateBezierSurface(this, 0, 0, XMFLOAT3(0.0f, 0.0f, 0.0f));
				imGuiWorkaroundSuggestedByItsAuthor = true;								
			}
			
			if (ImGui::MenuItem("Bezier Surface C2##ScenePopup"))
			{
				//m_objectFactory->CreateBezierSurface(this, 0, 0, XMFLOAT3(0.0f, 0.0f, 0.0f));
				imGuiWorkaroundSuggestedByItsAuthorC2 = true;
			}
			/*if (ImGui::MenuItem("Bezier Patch"))
			{
				auto p1 = m_objectFactory->CreateBezierPatch(this);				
				AttachObject(p1);				
			}*/

			if (ImGui::MenuItem("Point##ScenePopup"))
			{
				std::shared_ptr<Node> newPoint = m_objectFactory->CreatePoint(m_spawnMarker->GetTransform());
				// Check if any bezier curves are selected, if so - add the point to them

				for (int i = 0; i < m_selectedNodes.size(); i++)
				{
					if (std::shared_ptr<Node> selectedNode = m_selectedNodes[i].lock())
					{						
						if (dynamic_cast<BezierCurve*>(selectedNode->m_object.get()) != nullptr)
						{
							BezierCurve* curves = dynamic_cast<BezierCurve*>(selectedNode->m_object.get());
							auto Node = dynamic_cast<GroupNode*>(selectedNode.get());
							Node->AddChild(newPoint);
							curves->AttachChild(newPoint);
						}
					}
				}

				AttachObject(newPoint);

			}
			if (ImGui::MenuItem("Bezier Curve C0##ScenePopup"))
			{
				AttachObject(m_objectFactory->CreateBezierCurve(m_selectedNodes));				
			}

			if (ImGui::BeginMenu("Bezier Curve C2##ScenePopup"))
			{
				if (ImGui::MenuItem("B-Spline basis##ScenePopup"))
				{
					AttachObject(m_objectFactory->CreateBezierCurveC2(m_selectedNodes, BezierBasis::BSpline));
				}

				if (ImGui::MenuItem("Bernstein basis##ScenePopup"))
				{
					AttachObject(m_objectFactory->CreateBezierCurveC2(m_selectedNodes, BezierBasis::Bernstein));
				}

				if (ImGui::MenuItem("Interpolation curve##ScenePopup"))
				{
					AttachObject(m_objectFactory->CreateInterpolBezierCurveC2(m_selectedNodes));
				}


				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::Selectable("Clear scene##ScenePopup"))
		{
			ClearScene();
		}
		ImGui::EndPopup();
	}

	// ImGui Cannot Open new modal windows from MenuItems,
	// as stated here: https://github.com/ocornut/imgui/issues/249
	// there is currently no official way to do this except workarounds
	if (imGuiWorkaroundSuggestedByItsAuthor)
	{
		ImGui::OpenPopup("Modal window");			
		m_sizeU = 1;
		m_sizeV = 1;
		m_sizeX = 15;
		m_sizeY = 15;
		m_altState = false;
		m_altDir = false;
	}

	if (imGuiWorkaroundSuggestedByItsAuthorC2)
	{
		ImGui::OpenPopup("Modal window##C2");
		m_sizeU = 1;
		m_sizeV = 1;
		m_sizeX = 15;
		m_sizeY = 15;
		m_altState = false;
		m_altDir = false;
	}

	DrawBezierSurfaceModal();
	DrawBezierSurfaceC2Modal();
}


void Scene::DrawBezierSurfaceModal()
{
	if (ImGui::BeginPopupModal("Modal window"))
	{
		ImGui::Text("Choose the size of your surface");

		ImGui::Checkbox("Is a cylinder", &m_altState);


		if (m_altState)
		{
			ImGui::Checkbox("Wrap height", &m_altDir);
			ImGui::DragInt("Radius patch count", &m_sizeU, 1, 1, 10);
			ImGui::DragInt("Length patch count", &m_sizeV, 1, 1, 10);
			ImGui::DragInt("Radius", &m_sizeX, 1, 1, 20);
			ImGui::DragInt("Length", &m_sizeY, 1, 1, 20);
		}
		else {
			ImGui::DragInt("Width patch count", &m_sizeU, 2, 1, 10);
			ImGui::DragInt("Length patch count", &m_sizeV, 2, 1, 10);
			ImGui::DragInt("Surface Width", &m_sizeX, 1, 1, 30);
			ImGui::DragInt("Surface Length", &m_sizeY, 1, 1, 30);
		}

		if (m_sizeU < 1)
			m_sizeU = 1;

		if (m_sizeV < 1)
			m_sizeV = 1;

		if (m_sizeU > 10)
			m_sizeU = 10;

		if (m_sizeV > 10)
			m_sizeV = 10;

		if (ImGui::Button("Submit"))
		{
			SurfaceWrapDirection wrap = SurfaceWrapDirection::None;

			if (m_altState)
			{
				if (m_altDir)
				{
					wrap = SurfaceWrapDirection::Height;
				}
				else {
					wrap = SurfaceWrapDirection::Width;
				}
			}

			if (m_altDir)
			{
				m_objectFactory->CreateBezierSurface(this, m_sizeV, m_sizeU, m_spawnMarker->GetPosition(),
					m_altState, m_sizeY, m_sizeX, wrap);
			}
			else {
				m_objectFactory->CreateBezierSurface(this, m_sizeU, m_sizeV, m_spawnMarker->GetPosition(),
					m_altState, m_sizeX, m_sizeY, wrap);
			}

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Scene::DrawBezierSurfaceC2Modal()
{
	if (ImGui::BeginPopupModal("Modal window##C2"))
	{
		ImGui::Text("Choose the size of your surface");

		ImGui::Checkbox("Is a cylinder", &m_altState);


		if (m_altState)
		{
			ImGui::Checkbox("Wrap height", &m_altDir);
			ImGui::DragInt("Radius patch count", &m_sizeU, 1, 1, 10);
			ImGui::DragInt("Length patch count", &m_sizeV, 1, 1, 10);
			ImGui::DragInt("Radius", &m_sizeX, 1, 1, 20);
			ImGui::DragInt("Length", &m_sizeY, 1, 1, 20);
		}
		else {
			ImGui::DragInt("Width patch count", &m_sizeU, 2, 1, 10);
			ImGui::DragInt("Length patch count", &m_sizeV, 2, 1, 10);
			ImGui::DragInt("Surface Width", &m_sizeX, 1, 1, 30);
			ImGui::DragInt("Surface Length", &m_sizeY, 1, 1, 30);
		}

		if (m_sizeU < 1)
			m_sizeU = 1;

		if (m_sizeV < 1)
			m_sizeV = 1;

		if (m_sizeU > 10)
			m_sizeU = 10;

		if (m_sizeV > 10)
			m_sizeV = 10;

		if (ImGui::Button("Submit"))
		{
			SurfaceWrapDirection wrap = SurfaceWrapDirection::None;

			if (m_altState)
			{
				if (m_altDir)
				{
					wrap = SurfaceWrapDirection::Height;
				}
				else {
					wrap = SurfaceWrapDirection::Width;
				}
			}

			if (m_altDir)
			{
				m_objectFactory->CreateBezierSurfaceC2(this, m_sizeV, m_sizeU, m_spawnMarker->GetPosition(),
					m_altState, m_sizeY, m_sizeX, wrap);
			}
			else {
				m_objectFactory->CreateBezierSurfaceC2(this, m_sizeU, m_sizeV, m_spawnMarker->GetPosition(),
					m_altState, m_sizeX, m_sizeY, wrap);
			}

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Scene::DrawNodePopupMenu(const std::shared_ptr<Node> node)
{	
	
}

void Scene::RenderMiddleMarker(std::unique_ptr<RenderState>& renderState)
{
	if (m_selectedNodes.size() > 0)
	{
		int count = m_selectedNodes.size();
		DirectX::XMVECTOR pos = DirectX::XMVectorZero();

		for (int j = 0; j < m_selectedNodes.size(); j++)
		{
			if (auto node = m_selectedNodes[j].lock())
			{
				DirectX::XMVECTOR posj = DirectX::XMLoadFloat3(&(node->m_object->GetPosition()));
				pos = DirectX::XMVectorAdd(pos, posj);
			}
		}

		float countf = (float)count;

		DirectX::XMFLOAT3 newPos;
		DirectX::XMStoreFloat3(&newPos, pos);

		newPos.x /= countf;
		newPos.y /= countf;
		newPos.z /= countf;

		m_middleMarker->SetPosition(newPos);
		m_middleMarker->RenderCoordinates(renderState);
	}
}


void Scene::DrawSceneHierarchy(bool filtered)
{
	bool node_open = ImGui::TreeNode("Scene");	
	
	DrawScenePopupMenu();

	if (node_open)
	{
		if (m_nodes.size() != 0)
		{
			for (int i = 0; i < m_nodes.size(); i++)
			{
				if (filtered)
				{
					if (typeid(*(m_nodes[i]->m_object.get())) != typeid(Point))
					{
						m_nodes[i]->DrawNodeGUI(*this);
					}
				}
				else {
					m_nodes[i]->DrawNodeGUI(*this);
				}
				
			}
		}
		else 
		{
			ImGui::TreeNodeEx("<No objects in scene>", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		}		

		ImGui::TreePop();
	}	
}

void Scene::SelectionChanged(Node& node)
{
	// If ctrl is not pressed
	// select only the clicked node

	bool uniqueSelection = ImGui::GetIO().KeyCtrl == false;
	uniqueSelection |= node.GetIsVirtual();

	if (uniqueSelection)
	{

		std::weak_ptr<Node> weakNode;
		for (int i = 0; i < m_nodes.size(); i++)
		{		
			// clear selected status
			m_nodes[i]->SetIsSelected(false);
			m_nodes[i]->ClearChildrenSelection();

			// Find weak_ptr to node
			if (m_nodes[i]->m_object == node.m_object)
			{
				weakNode = m_nodes[i];
			}

			Object& obj = *(node.m_object.get());			
			auto vNode = FindObjectNode(m_nodes[i]->GetChildren(), obj);
			if (vNode.expired() == false)
			{
				weakNode = vNode;
			}

		}

		m_selectedNodes.clear();

		// select this node
		node.SetIsSelected(true);			
		m_selectedNodes.push_back(weakNode);
	}
	else
	{
		// Ctrl is pressed
		// Add not selected node to selection or
		// Remove selected node from selection
		if (node.GetIsSelected())
		{
			// deselect this node and
			node.SetIsSelected(false);
			// remove this node from m_selectedNodes
			auto it = m_selectedNodes.begin();
			while (it != m_selectedNodes.end())
			{
				if (auto selectedNode = it->lock())
				{
					if (selectedNode->m_object == node.m_object)
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
			node.SetIsSelected(true);
			std::weak_ptr<Node> weakNode;
			Object& obj = *(node.m_object.get());
			auto vNode = FindObjectNode(m_nodes, obj);
			if (vNode)
			{
				weakNode = vNode;
			}

			m_selectedNodes.push_back(weakNode);
		}

	}
}

void Scene::RenderScene(std::unique_ptr<RenderState>& renderState)
{
	m_grid->RenderObject(renderState);
	m_spawnMarker->RenderObject(renderState);	
	for (int i = 0; i < m_nodes.size(); i++)
	{
		// TODO [MG] : check if this item is currently selected		
		m_nodes[i]->Render(renderState);
		auto children = m_nodes[i]->GetChildren();
		for (int j = 0; j < children.size(); j++)
		{
			if (auto child = children[j].lock())
			{
				child->Render(renderState);
			}
		}
	}		

	RenderMiddleMarker(renderState);	
}

void Scene::ClearModifiedTag()
{
	auto it = m_nodes.begin();
	while(it != m_nodes.end())
	{ 
		auto node = it->get();
		auto children = node->GetChildren();
		for (int j = 0; j < children.size(); j++)
		{
			if (auto child = children[j].lock())
			{
				child->m_object->SetModified(false);
			}
		}
		node->m_object->SetModified(false);
		it++;
	}
}

void Scene::UpdateScene()
{
	UpdateSelectedNode();

	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->Update();
	}
}

void Scene::LateUpdate()
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->LateUpdate();
	}
}

void Scene::UpdateSelectedNode()
{	
	// Draw the inspector window
	// foreach selected node
	//   Draw section for each node

	auto it = m_selectedNodes.begin();
	while (it != m_selectedNodes.end())
	{
		// TODO [MG] : add collapsing headers
		if (auto selectedNode = it->lock())
		{
			if (selectedNode->m_object)
			{
				if (selectedNode->m_object->CreateParamsGui())
				{
					selectedNode->m_object->SetModified(true);
				}
			}
			it++;
		}
		else {
			it = m_selectedNodes.erase(it);
		}
	}
}
