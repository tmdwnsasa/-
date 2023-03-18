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
	virtual void Update() override;

private:
	float			_speed = 100.0f;
};

