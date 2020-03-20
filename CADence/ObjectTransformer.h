#pragma once
#include <memory>

#include "cadStructures.h"

class ObjectTransformer
{
public:
	void RotateObject(std::unique_ptr<Object>& object, DirectX::XMFLOAT3 pivot, DirectX::XMFLOAT3 eulerAngles);
	void ScaleObject(std::unique_ptr<Object>& object, DirectX::XMFLOAT3 pivot, DirectX::XMFLOAT3 scale);
	void TranslateObject(std::unique_ptr<Object>& object, DirectX::XMFLOAT3 pivot, DirectX::XMFLOAT3 translation);
private:
};
