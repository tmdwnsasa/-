#pragma once
#include "Component.h"

class Gun : public Component
{
public:
	Gun();
	virtual ~Gun();

	virtual void Update() override;

	void SetCameraTransform(shared_ptr<Transform> transform) { CameraTransform = transform; };

private:
	shared_ptr<Transform> CameraTransform;
};

