#include "ParametrizedObject2D.h"
#include "imgui.h"

bool ParametrizedObject2D::CreateParamsGui()
{
	bool objectChanged = Object::CreateParamsGui();
	bool surfaceObjectChanged = false;

	std::string mainR = "Density X##" + m_defaultName;
	std::string secR = "Density Y##" + m_defaultName;

	surfaceObjectChanged |= ImGui::SliderInt(mainR.c_str(), &(m_surParams.densityX), m_surParams.minDensityX, m_surParams.maxDensityX);
	surfaceObjectChanged |= ImGui::SliderInt(secR.c_str(), &(m_surParams.densityY), m_surParams.minDensityY, m_surParams.maxDensityY);
	return surfaceObjectChanged;
}