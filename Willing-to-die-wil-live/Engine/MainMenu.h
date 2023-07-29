#pragma once
#include "Component.h"

class MainMenu : public Component
{
public:
	MainMenu();
	virtual ~MainMenu();

public:
	virtual void Update() override;

	bool GetMenuState() { return _menuState; };
	void SetMenuState(bool state) { _menuState = state; };
	vector<shared_ptr<GameObject>> GetMenuObjects() { return _menuObject; };

private:
	vector<shared_ptr<GameObject>> _menuObject;
	bool		_menuState = true;
	POINT		_mousePos;
	bool		_start = false;

};