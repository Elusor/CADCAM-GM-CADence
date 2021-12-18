#pragma once
#include <string>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include <sstream>
#include <iomanip>

std::string PrepareMoveInstruction(DirectX::SimpleMath::Vector3 pos);

DirectX::XMFLOAT3 ConvertToMilimeters(DirectX::XMFLOAT3 point);