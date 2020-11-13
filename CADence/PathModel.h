#pragma once
#include <vector>
#include <string>
#include "Object.h"

class PathModel
{
public:
	PathModel();

	void SetModelObjects(std::vector<ObjectRef> modelObjects);
	std::vector<ObjectRef> GetModelObjects();
	std::vector<std::string> GetObjectNames();
private:
	std::vector<std::string> m_objectNames;
	std::vector<ObjectRef> m_modelObjects;
};