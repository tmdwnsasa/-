#pragma once
#include "Component.h"
#include "Player.h"

enum class BUTTON_TYPE
{
	NONE,
	SHOP,
	PURCHASE,
	MENU,


	END
};

enum class MERCHANDISE_TYPE : uint8
{
	GUN,
	BULLET,
	ARMOR,
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

	MERCHANDISE_TYPE GetMerchandiseType() { return _type; };
	void SetMerchandiseType(MERCHANDISE_TYPE merchandise) { _type = merchandise; };

	bool CheckPress(POINT mousePos);

private:
	BUTTON_TYPE _buttonType;
	MERCHANDISE_TYPE _type;
	PLAYER_WEAPON _merchandise;

};

