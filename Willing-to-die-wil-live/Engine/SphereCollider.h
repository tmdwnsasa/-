#pragma once
#include "BaseCollider.h"

class SphereCollider : public Component
{
public:
	SphereCollider();
	virtual ~SphereCollider();

	virtual void FinalUpdate() override;
	virtual bool Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance);

	void SetRadius(float radius) { _radius = radius; }
	void SetCenter(Vec3 center) { _center = center; }

	 BoundingSphere GetColliderSphere() { return _boundingSphere; };

private:
	// Local ����
	float		_radius = 1.f;
	Vec3		_center = Vec3(0, 0, 0);

	BoundingSphere _boundingSphere;
};

