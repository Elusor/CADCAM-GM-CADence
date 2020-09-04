#include "InputLayoutManager.h"
#include <stdexcept>
#include <cassert>


ID3D11InputLayout* InputLayoutManager::GetLayout(std::type_index vertexDataTypeIndex)
{
	ID3D11InputLayout* correspondingLayout = nullptr;
	std::type_index key = vertexDataTypeIndex;
	try
	{
		auto layout = m_layouts.at(key).get();
		correspondingLayout = layout;
	}
	catch (const std::out_of_range & oorEx)
	{
		assert("Corresponding layout not registered" && false);
	}

	return correspondingLayout;
}

