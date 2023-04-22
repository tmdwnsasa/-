#include "pch.h"
#include "BaseCollider.h"

BaseCollider::BaseCollider(ColliderType colliderType)
	: Component(COMPONENT_TYPE::BASECOLLIDER), _colliderType(colliderType)
{

}

BaseCollider::~BaseCollider()
{

}