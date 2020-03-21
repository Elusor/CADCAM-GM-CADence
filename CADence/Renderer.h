#pragma once
#include <memory>
#include "renderData.h"
#include "Transform.h"

using namespace std;

class Renderer
{
public:
	explicit Renderer(mini::Window& window);	
	virtual void Render();
	
	void RenderImGUI();
protected:
	void Clear();
	void SetShaders();
	void UpdateInputLayout();
	float m_clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	unique_ptr<RenderState> m_renderState;
};