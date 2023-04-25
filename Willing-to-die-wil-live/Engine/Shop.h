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
	bool GetPurchase() { return _purchase; };
	void SetPurchase(bool purchase) { _purchase = purchase; };

	void MakeMerchandise(int count, PLAYER_WEAPON weapon);
	void ShowSelectedMerchandise();
	shared_ptr<GameObject> GetSelected() { return _selected; };

private:
	vector<shared_ptr<GameObject>> _shopObject;
	bool		_shopState = false;
	int			_shopMerchandise = 4;
	POINT		_mousePos;

	shared_ptr<GameObject> _selected = NULL;
	bool		_purchase = false;

};	