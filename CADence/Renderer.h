#pragma once
#include <memory>
#include "RenderState.h"
#include "Transform.h"

using namespace std;

class Renderer
{
public:
	explicit Renderer(mini::Window& window);	
	

protected:

	void SetShaders();
	void UpdateInputLayout();
	unique_ptr<RenderState> m_renderState;
};