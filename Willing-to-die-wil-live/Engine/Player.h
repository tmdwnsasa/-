#pragma once
#include "MonoBehaviour.h"

enum class PLAYER_STATE : uint8
{
	IDLE,
	WALK,


	DIE,
	END
};

class Player : public MonoBehaviour
{
public:
	Player();
	virtual ~Player();
public:
	virtual void Update() override;
	void PlayerRotate();

private:
	float		_speed = 100.0f;
	POINT		_mousePos = {};
	POINT		_oldMousePos = {};
	float		cxdelta = 0.f;
};