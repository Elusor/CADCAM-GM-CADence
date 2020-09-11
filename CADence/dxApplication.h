#pragma once
#include <DirectXMath.h>
#include "TransformationController.h"
#include "Scene.h"
#include "camera.h"
#include "CameraController.h"
#include "windowApplication.h"
#include "dxDevice.h"
#include "vertexStructures.h"
#include "objects.h"
#include "PointSelector.h"
#include "renderState.h"

#include "Trimmer.h"
#include "BackBufferRenderTarget.h"
#include "TextureRenderTarget.h"
#include "IRenderPass.h"
#include "DefaultRenderPass.h"
#include "StereoscopicRenderPass.h"
#include "sceneImporter.h"
#include "SceneExporter.h"
#include "FileManager.h"
#include "GuiManager.h"
#include "PointCollapser.h"
#include "IntersectionFinder.h"
#include "CurveVisualizer.h"
#include "DeletionManager.h"

class DxApplication : public mini::WindowApplication
{
public:
	explicit DxApplication(HINSTANCE hInstance);	

protected:
	int MainLoop() override;

private:
	void InitImguiWindows();
	void Update();

	bool m_isStereo;
	bool m_stereoChanged;
	bool m_filterObjects;

	std::unique_ptr<TransformationController> m_transController;
	std::unique_ptr<PointSelector> m_pSelector;
	std::shared_ptr<Scene> m_scene;	
	std::unique_ptr<CameraController> m_camController;		
	std::unique_ptr<RenderState> m_renderState;
	std::unique_ptr<SceneImporter> m_importer;
	std::unique_ptr<SceneExporter> m_exporter;
	std::unique_ptr<FileManager> m_fileManager;
	std::unique_ptr<GuiManager> m_guiManager;
	std::unique_ptr<PointCollapser> m_pointCollapser;
	std::unique_ptr<IntersectionFinder> m_intersectionFinder;
	std::unique_ptr<DeletionManager> m_deletionManager;

	IRenderPass* m_defPass;
	IRenderPass* m_stereoPass;
	IRenderPass* m_activePass;

	BackBufferRenderTarget* m_target;
};
