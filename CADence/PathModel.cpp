#include "PathModel.h"

PathModel::PathModel()
{
    SetModelObjects(std::vector<ObjectRef>());
}

void PathModel::SetModelObjects(std::vector<ObjectRef> modelObjects)
{
    m_modelObjects = modelObjects;

    for (auto wPtr : m_modelObjects)
    {
        if (auto ptr = wPtr.lock())
        {
            auto name = ptr->m_object->m_name;
            m_objectNames.push_back(name);
        }
    }

    if (m_modelObjects.size() == 0)
    {
        m_objectNames.push_back("Model empty");
    }
}

std::vector<ObjectRef> PathModel::GetModelObjects()
{
    return m_modelObjects;
}

std::vector<std::string> PathModel::GetObjectNames()
{
    return m_objectNames;
}
