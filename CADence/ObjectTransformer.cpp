#include "ObjectTransformer.h"

void ObjectTransformer::RotateObject(std::unique_ptr<Object>& object, DirectX::XMFLOAT3 pivot, DirectX::XMFLOAT3 eulerAngles)
{
    DirectX::XMFLOAT3 pos = object->m_transform.m_pos;

    DirectX::XMFLOAT3 diff;
    
    diff.x = pos.x - pivot.x;
    diff.y = pos.y - pivot.y;
    diff.z = pos.z - pivot.z;

    auto rotVect = DirectX::XMLoadFloat3(&eulerAngles);
    auto rot = DirectX::XMMatrixRotationRollPitchYawFromVector(rotVect);
    // multiply difference vector by rotation matrix
    DirectX::XMVECTOR diffVec;
    diffVec = DirectX::XMLoadFloat3(&diff);
    diffVec = DirectX::XMVector3Transform(diffVec, rot);
    DirectX::XMStoreFloat3(&diff, diffVec);

    object->m_transform.m_pos.x = pivot.x + diff.x;
    object->m_transform.m_pos.y = pivot.y + diff.y;
    object->m_transform.m_pos.z = pivot.z + diff.z;

    auto curRot = DirectX::XMLoadFloat3(&(object->m_transform.m_rotation));

    // TODO check if the order is right
    auto curRotMatrix = DirectX::XMMatrixRotationRollPitchYawFromVector(curRot);
    auto completeRot = DirectX::XMMatrixMultiply(rot, curRotMatrix);        
    //Rotation = (quat * Quaternion.FromEulerAngles(Rotation)).EulerAngles();   
    object->m_transform.m_rotation.x += eulerAngles.x;
    object->m_transform.m_rotation.y += eulerAngles.y;
    object->m_transform.m_rotation.z += eulerAngles.z;
}

void ObjectTransformer::ScaleObject(std::unique_ptr<Object>& object, DirectX::XMFLOAT3 pivot, DirectX::XMFLOAT3 scale)
{
    DirectX::XMFLOAT3 pos = object->m_transform.m_pos;

    DirectX::XMFLOAT3 diff;

    diff.x = pos.x - pivot.x;
    diff.y = pos.y - pivot.y;
    diff.z = pos.z - pivot.z;

    DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

    DirectX::XMVECTOR diffVec;
    diffVec = DirectX::XMLoadFloat3(&diff);
    diffVec = DirectX::XMVector3Transform(diffVec, scaleMat);
    DirectX::XMStoreFloat3(&diff, diffVec);

    object->m_transform.m_pos.x = pivot.x + diff.x;
    object->m_transform.m_pos.y = pivot.y + diff.y;
    object->m_transform.m_pos.z = pivot.z + diff.z;

    object->m_transform.m_scale.x *= scale.x;
    object->m_transform.m_scale.y *= scale.y;
    object->m_transform.m_scale.z *= scale.z;
}

void ObjectTransformer::TranslateObject(std::unique_ptr<Object>& object, DirectX::XMFLOAT3 pivot, DirectX::XMFLOAT3 translation)
{
	object->m_transform.m_pos.x	+= translation.x;
	object->m_transform.m_pos.y	+= translation.y;
	object->m_transform.m_pos.z += translation.z;
}
