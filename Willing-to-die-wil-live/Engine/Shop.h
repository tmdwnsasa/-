#pragma once
#include "Component.h"

class Shop : public Component
{
public:
	Shop();
	virtual ~Shop();

public:
	virtual void Update() override;


};

