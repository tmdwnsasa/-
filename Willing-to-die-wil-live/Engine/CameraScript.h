#pragma once
#include "MonoBehaviour.h"

class CameraScript : public Component
{
public:
	CameraScript();
	virtual ~CameraScript();

	virtual void Update() override;

	virtual void LateUpdate() override;

	void CameraRotation();

	void SetMouseLock(bool lock) { _mouseLock = lock; };
	bool GetMouseLock() { return _mouseLock; };
	void SetShopOpened(bool open) { _shopOpened = open; };

private:
	float		_speed = 100.f;
	POINT		_oldMousePos = {};

	bool		_shopOpened = false;
	bool		_mouseLock = false;
	POINT		_mousePos = {};
	float		cxdelta = 0.f;
	float		cydelta = 0.f;
};