#pragma once
#include "Component.h"
#include "Player.h"

class Gun : public Component
{
public:
	Gun();
	Gun(PLAYER_WEAPON weapon);
	virtual ~Gun();

	virtual void Update() override;
	void recoil();
	void SetRecoil(float recoil) { _curRecoil += recoil; };
	void SetCameraTransform(shared_ptr<Transform> transform) { CameraTransform = transform; };

private:

	PLAYER_WEAPON weapon;
	shared_ptr<Transform> CameraTransform;
	float				_recoilAmount = 0.0f;
	float				_curRecoil = 0.0f;
	float				_downForce = 1.f;
};