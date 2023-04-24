#pragma once
#include "Component.h"
#include "Player.h"

enum class BUTTON_TYPE
{
	NONE,
	SHOP,
	MENU,

	END
};

class Button : public Component
{
public:
	Button();
	virtual ~Button();

	BUTTON_TYPE GetType() { return _buttonType; };
	void SetType(BUTTON_TYPE buttonType) { _buttonType = buttonType; };
	PLAYER_WEAPON GetMerchandise() { return _merchandise; };
	void SetMerchandise(PLAYER_WEAPON merchandise) { _merchandise = merchandise; };

	bool CheckPress(POINT mousePos);
private:
	BUTTON_TYPE _buttonType;
	PLAYER_WEAPON _merchandise;

};

