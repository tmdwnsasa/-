#pragma once
#include "BaseCollider.h"

class BoxCollider : public Component
{
public:
	BoxCollider();
	virtual ~BoxCollider();

	virtual void FinalUpdate() override;

	virtual bool Intersects(BoundingBox other);

	void SetExtents(Vec3 extents) { _extents = extents; }
	void SetCenter(Vec3 center) { _center = center; }

	BoundingBox GetColliderBox() { return _boundingBox; };
private:
	// Local ±‚¡ÿ
	Vec3		_center = Vec3(0.f, 0.f, 0.f);
	Vec3		_extents = Vec3(1.f, 1.f, 1.f);

	BoundingBox _boundingBox;
};

