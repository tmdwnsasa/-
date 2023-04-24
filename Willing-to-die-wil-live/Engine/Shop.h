#pragma once
#include "Component.h"
#include "Player.h"

class Shop : public Component
{
public:
	Shop();
	virtual ~Shop();

public:
	virtual void Update() override;

	bool GetShopState() { return _shopState; };
	vector<shared_ptr<GameObject>> GetShopObjects() { return _shopObject; };


	void MakeMerchandise(int count, PLAYER_WEAPON weapon);
	void ShowSelectedMerchandise();

private:
	vector<shared_ptr<GameObject>> _shopObject;
	bool		_shopState = false;
	int			_shopMerchandise = 4;
	POINT		_mousePos;

	shared_ptr<GameObject> _selected = NULL;

};	