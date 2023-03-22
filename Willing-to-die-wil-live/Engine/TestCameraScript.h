#pragma once
#include "MonoBehaviour.h"

class TestCameraScript : public MonoBehaviour
{
public:
	TestCameraScript();
	virtual ~TestCameraScript();

	virtual void Update() override;

	virtual void LateUpdate() override;

private:
	float		_speed = 100.f;

	POINT		_mousePos = {};
	POINT		_oldMousePos = {};
	float		cxdelta = 0.f;
	float		cydelta = 0.f;
};

