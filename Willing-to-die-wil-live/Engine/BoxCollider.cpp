#include "pch.h"
#include "BoxCollider.h"
#include "GameObject.h"
#include "Transform.h"
#include <iostream>

BoxCollider::BoxCollider() : Component(COMPONENT_TYPE::BOXCOLLIDER)
{

}

BoxCollider::~BoxCollider()
{

}

void BoxCollider::FinalUpdate()
{
	Vec3 pos = GetGameObject()->GetTransform()->GetWorldPosition();

	_boundingBox.Center = pos;
		//Vec3(pos.x + _center.x, pos.y + _center.y, pos.z + _center.z);

	Vec3 scale = GetGameObject()->GetTransform()->GetLocalScale();
	Vec3 rotation = GetGameObject()->GetTransform()->GetLocalRotation();
	scale /= 2;
	if (rotation.y > py * 0.5)
	{
		_boundingBox.Extents = Vec3(_extents.x * scale.z, _extents.y * scale.y, _extents.z * scale.x);
	}
	else
		_boundingBox.Extents = Vec3(_extents.x * scale.x, _extents.y * scale.y, _extents.z * scale.z);
}

bool BoxCollider::Intersects(BoundingBox other)
{
	if (_onOff == true)
	{
		return _boundingBox.Intersects(other);
	}
	else
		return false;
}

bool BoxCollider::Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	if (_onOff == true)
	{
		return _boundingBox.Intersects(rayOrigin, rayDir, OUT distance);
	}
	else
		return false;
}