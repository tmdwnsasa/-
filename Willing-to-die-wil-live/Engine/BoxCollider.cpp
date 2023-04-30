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
	_boundingBox.Center = GetGameObject()->GetTransform()->GetWorldPosition();

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
	return _boundingBox.Intersects(other);
}

bool BoxCollider::Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return _boundingBox.Intersects(rayOrigin, rayDir, OUT distance);
}