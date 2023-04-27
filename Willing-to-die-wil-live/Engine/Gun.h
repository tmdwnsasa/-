#pragma once
#include "Component.h"

class Gun : public Component
{
	Gun();
	virtual ~Gun();

	virtual void Update() override;
};

