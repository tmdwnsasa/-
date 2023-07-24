#include "pch.h"
#include "Button.h"
#include "Engine.h"
#include "Component.h"
#include "Transform.h"
#include "Input.h"

Button::Button() : Component(COMPONENT_TYPE::BUTTON)
{
	_buttonType = BUTTON_TYPE::NONE;
	_merchandise = PLAYER_WEAPON::NONE;
}

Button::~Button()
{

}

bool Button::CheckPress(POINT mousePos)
{
	Vec3 Pos = GetTransform()->GetLocalPosition();
	Pos.x += INPUT->GetOldMousePos().x;
	Pos.y *= -1;
	Pos.y += INPUT->GetOldMousePos().y;	
	Vec3 Scale = GetTransform()->GetLocalScale();

	if (mousePos.x >= Pos.x - Scale.x /2 &&
		mousePos.x <= Pos.x + Scale.x /2 &&
		mousePos.y >= Pos.y - Scale.y /1.2 &&
		mousePos.y <= Pos.y + Scale.y /3)
	{
		return true;
	}
	else
		return false;
}