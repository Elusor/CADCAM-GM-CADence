#include "dxApplication.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "vertexStructures.h"
#include "torusGenerator.h"
#include "camera.h"
#include "Scene.h"
#include "renderState.h"
#include "ObjectFactory.h"
#include "PointSelector.h"
#include "IOExceptions.h"
#include "HoleDetector.h"
#include "IntersectionFinder.h"
#include "GeometricFunctions.h"
#include "IntersectionExceptions.h"
#include "DeletionManager.h"

using namespace mini;
using namespace DirectX;
using namespace std;

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance)
{
	// init viewport
	SIZE wndSize = m_window.getClientSize();
	Viewport viewport{ wndSize };	

	// init camera
	shared_ptr<Camera> camera = make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
		XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
		XMFLOAT2(0.0f, -0.55f), // yaw, pitch
		viewport.Width,
		viewport.Height,
		DirectX::XM_PIDIV4, 1.f, 200); // fov, zNear, zFar		
			
	m_camController = make_unique<CameraController>(camera);
	m_renderState = make_unique<RenderState>(m_window, viewport, camera);
	m_guiManager = make_unique<GuiManager>();
	m_scene = make_shared<Scene>(m_guiManager.get(), m_renderState.get());
	m_transController = make_unique<TransformationController>(m_scene);
	m_pSelector = make_unique<PointSelector>(m_renderState->m_camera);
	m_importer = make_unique<SceneImporter>(m_scene.get(), m_guiManager.get());
	m_exporter = make_unique<SceneExporter>(m_scene.get(), m_guiManager.get());
	m_fileManager = make_unique<FileManager>();
	m_pointCollapser = make_unique<PointCollapser>(m_scene.get());
	m_intersectionFinder = make_unique<IntersectionFinder>(m_scene.get());
	m_deletionManager = make_unique<DeletionManager>(m_scene);

	//// RENDER PASS
	m_defPass = new DefaultRenderPass(m_renderState, wndSize);
	m_stereoPass = new StereoscopicRenderPass(m_renderState, wndSize);
	m_activePass = m_defPass;
	////

	//Setup imGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(this->m_window.getHandle());
	ImGui_ImplDX11_Init(m_renderState->m_device.m_device.get(), m_renderState->m_device.m_context.get());
	ImGui::StyleColorsDark();

}

int DxApplication::MainLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof msg);
	do
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (m_stereoChanged)
			{
				m_isStereo = !m_isStereo;
				if (m_isStereo)
				{
					m_activePass = m_stereoPass;
				}
				else{
					m_activePass = m_defPass;
				}
				m_stereoChanged = false;
			}

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			m_scene->ClearModifiedTag();

			m_transController->ProcessInput(ImGui::GetIO());
			if (m_transController->IsTransforming() == false) {
				m_camController->ProcessMessage(&ImGui::GetIO());
				m_pSelector->ProcessInput(m_scene, m_window.getClientSize());
				
			}					

			m_deletionManager->ProcessInput(ImGui::GetIO());

			InitImguiWindows();
			m_guiManager->Update();
			Update();
			m_activePass->Execute(m_renderState, m_scene.get());		
			m_pSelector->DrawSelectionWindow(m_renderState, m_window.getClientSize());
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			m_renderState->m_device.m_swapChain.get()->Present(0, 0);
		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::Update()
{
	m_scene->UpdateScene();
	m_scene->LateUpdate();
}

void DxApplication::InitImguiWindows()
{

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) {
				// This will delete any unsaved work
				// Clear Scene
				m_scene->ClearScene();
				// Reset the camera
				m_renderState->m_camera->ResetCamera();
				// Turn of transformations
				m_transController->Reset();
				// Reset the colors
				// Set all things to default
			}
			if (ImGui::MenuItem("Open")) {
				// Open dialog to select file
				try{
					wstring filename = m_fileManager->OpenFileDialog();
					// Check if file is correct
					bool validFile = m_importer->InvalidateFile(filename);
					if (validFile)
					{
						// Import scene from file
						if (m_importer->Import(filename)) {
							m_guiManager->EnableCustomModal("Scene imported successfully.", "Operation complete");
						}
					}					
				}
				catch (IncorrectFileExtensionException & e)
				{
					m_guiManager->EnableCustomModal(e.what(), "Reading Error");
				}
				catch(...) 
				{
					m_guiManager->EnableCustomModal("Something went wrong.", "Reading Error");
				}
				
			}
			if (ImGui::MenuItem("Save")) {
				// If a file has been saved 
					// Save to the lastest file
				// else
				// Trigger Save As
			}
			if (ImGui::MenuItem("Save As...")) {
				try {
					// Open dialog to choose save location			
					bool validFile = m_exporter->InvalidateScene();
					// Save the file
					if (validFile)
					{
						wstring filename = m_fileManager->SaveFileDialog();
						if (m_exporter->Export(filename))
						{
							m_guiManager->EnableCustomModal("Object saved successfully.", "Operation complete");
						}
					}
					else {
						// Scene is invalid modal
					}
				}
				catch (IncorrectFileExtensionException & e)
				{
					m_guiManager->EnableCustomModal(e.what(), "Reading Error");
				}
				catch (...)
				{
					m_guiManager->EnableCustomModal("Something went wrong.", "Reading Error");
				}
				
			}
			//ShowExampleMenuFile();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
	

	if (ImGui::CollapsingHeader("Hierarchy"))
	{				

		try
		{
			if (m_scene->m_selectedNodes.size() == 1)
			{
				auto objectRef = m_scene->m_selectedNodes[0];
				if (auto object = objectRef.lock())
				{
					auto surf = dynamic_cast<IParametricSurface*>(object->m_object.get());
					if (surf != nullptr)
					{
						if (ImGui::Button("Intersect with self"))
						{
							m_intersectionFinder->FindInterSection(objectRef, objectRef);
						}

						if (ImGui::Button("Intersect with self - cursor"))
						{
							auto cursorPos = m_scene->m_spawnMarker->GetPosition();
							m_intersectionFinder->FindIntersectionWithCursor(objectRef, objectRef, cursorPos);
						}

					}
				}
			}

			if (m_scene->m_selectedNodes.size() == 2)
			{
				auto p1 = m_scene->m_selectedNodes[0];
				auto p2 = m_scene->m_selectedNodes[1];
				if (typeid(*p1.lock()->m_object.get()) == typeid(Point) &&
					typeid(*p2.lock()->m_object.get()) == typeid(Point))
				{
					if (ImGui::Button("Collapse points"))
					{
						m_pointCollapser->Collapse(p1, p2);
					}
				}

				if (m_intersectionFinder->AreObjectIntersectable(p1, p2))
				{					
					if (ImGui::Button("Intersect surfaces"))
					{
						m_intersectionFinder->FindInterSection(p1, p2);
					}

					if (ImGui::Button("Intersect surfaces with cursor"))
					{
						auto cursorPos = m_scene->m_spawnMarker->GetPosition();
						m_intersectionFinder->FindIntersectionWithCursor(p1, p2, cursorPos);
					}
				}
			}
		}
		catch (IntersectionNotFoundException ienf)
		{
			m_guiManager->EnableCustomModal(ienf.what(), "Intersection Error");
		}
		catch (IntersectionParallelSurfacesException ipse)
		{
			m_guiManager->EnableCustomModal(ipse.what(), "Intersection Error");
		}
		catch (IntersectionTooFewPointsException itfpe)
		{
			m_guiManager->EnableCustomModal(itfpe.what(), "Intersection Error");
		}
		catch (IntersectionTooManyPointsException itmpe)
		{
			m_guiManager->EnableCustomModal(itmpe.what(), "Intersection Error");
		}
		catch (...)
		{
			m_guiManager->EnableCustomModal("Something went wrong.", "Intersection Error - General Error");
		}

		if (m_scene->m_selectedNodes.size() == 3)
		{
			auto p1 = m_scene->m_selectedNodes[0];
			auto p2 = m_scene->m_selectedNodes[1];
			auto p3 = m_scene->m_selectedNodes[2];
			if (typeid(*p1.lock()->m_object.get()) == typeid(BezierPatch) &&
				typeid(*p2.lock()->m_object.get()) == typeid(BezierPatch) &&
				typeid(*p3.lock()->m_object.get()) == typeid(BezierPatch))
			{
				auto hole = HoleDetector::DetectHole(
					(BezierPatch*)p1.lock()->m_object.get(),
					(BezierPatch*)p2.lock()->m_object.get(),
					(BezierPatch*)p3.lock()->m_object.get());

				if (hole.isValid) 
				{
					if (ImGui::Button("Fill the hole"))
					{
						
						auto pts1 = p1.lock()->m_object->GetReferences().GetRefObjects();
						auto pts2 = p2.lock()->m_object->GetReferences().GetRefObjects();
						auto pts3 = p3.lock()->m_object->GetReferences().GetRefObjects();
						auto patch = m_scene->m_objectFactory->CreateGregoryPatch(hole);
						m_scene->AttachObject(patch);
					}
				}
				
			}
		}

		ImGui::Checkbox("Hide Points", &m_filterObjects);

		m_scene->DrawSceneHierarchy(m_filterObjects);
		ImGui::Spacing();
	}
	if (ImGui::CollapsingHeader("Cursor"))
	{
		m_scene->m_spawnMarker->CreateParamsGui();	
	}
	if (ImGui::CollapsingHeader("Grid"))
	{
		m_scene->m_grid->CreateParamsGui();
	}
	if (ImGui::CollapsingHeader("Intersections"))
	{
		m_intersectionFinder->CreateParamsGui();
	}
	if (ImGui::CollapsingHeader("Transformations"))
	{
		ImGui::Text("Center trasformations at:");
		ImGui::Spacing();

		ImGui::Checkbox("Center trasformations at cursor", &(m_transController->m_transAroundCursor));
	}
	if (ImGui::CollapsingHeader("Stereoscopy"))
	{
		if (m_isStereo) {

			StereoscopicRenderPass* stereo = static_cast<StereoscopicRenderPass*>(m_stereoPass);

			if (ImGui::Button("Disable stereoscopy"))
			{
				m_stereoChanged = true;
			}
			if (ImGui::DragFloat("Focus plane distance", &(stereo->m_focusPlaneDistance), .5f, 0.5f, 100.f));
			if (ImGui::DragFloat("Eye distance", &(stereo->m_eyeDistance), 0.05f, 0.1f, 2.f));
		}
		else {
			if (ImGui::Button("Enable stereoscopy"))
			{
				m_stereoChanged = true;
			}
		}
	}	

	ImGui::Separator();
	ImGui::Spacing();
	m_transController->CreateGuiStatus();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

}