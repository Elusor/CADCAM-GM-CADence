#pragma once
#include <d3d11.h>
#include <vector>
#include <unordered_map>
#include <typeindex>

#include "dxDevice.h"

class InputLayoutManager
{
public:
	ID3D11InputLayout* GetLayout(std::type_index VertexDataTypeIndex);

	template <class T>
	void RegisterLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutElements, const std::vector<BYTE> vsCode, const DxDevice& device);

private:
	std::unordered_map<std::type_index, mini::dx_ptr<ID3D11InputLayout>> m_layouts;
};

template<class T>
inline void InputLayoutManager::RegisterLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutElements, const std::vector<BYTE> vsCode, const DxDevice& device)
{
	std::type_index key = std::type_index(typeid(T));
	auto createdLayout = device.CreateInputLayout(inputLayoutElements, vsCode);
	m_layouts[key] = std::move(createdLayout);
}