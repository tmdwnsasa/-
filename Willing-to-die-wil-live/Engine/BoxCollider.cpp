#include "pch.h"
#include "BoxCollider.h"
#include "GameObject.h"
#include "Transform.h"

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
	_boundingBox.Extents = Vec3(_extents.x * scale.x, _extents.y * scale.y, _extents.z * scale.z);
}

bool BoxCollider::Intersects(BoundingBox other)
{
	return _boundingBox.Intersects(other);
}