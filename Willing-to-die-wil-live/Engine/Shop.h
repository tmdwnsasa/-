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
	void ShowSelectedMerchandise();		//선택된 상품의 정보를 출력한다.
	void SetSelectedObject(shared_ptr<GameObject> object) { _selectedObject = object; };
	shared_ptr<GameObject> GetSelectedObject() { return _selectedObject; };
	shared_ptr<GameObject> GetSelected() { return _selected; };
	int GetSelectedNum() { return _selectedNum; };

private:
	vector<shared_ptr<GameObject>> _shopObject;
	bool		_shopState = false;
	int			_shopMerchandise = 4;
	POINT		_mousePos;
	int			_selectedNum;

	shared_ptr<GameObject> _selected = NULL;
	shared_ptr<GameObject> _selectedObject = NULL;
	bool		_purchase = false;

};	