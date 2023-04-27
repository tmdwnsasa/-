#pragma once
#include "Component.h"

enum class PLAYER_STATE : uint8
{
	IDLE,
	WALK,


	DIE,
	END
};

enum class PLAYER_WEAPON : uint8
{
	PISTOL,
	SMG,
	SHOTGUN,
	RIFLE,
	END
};

enum class BULLET_STATE : uint8
{
	LIVE,
	SHOOT,
	DEAD,
	END
};

class Player : public Component
{
public:
	Player();
	virtual ~Player();

public:
	virtual void Update() override;
	void PlayerRotate();
	void ChangeWeapon(PLAYER_WEAPON weapon);
	vector<shared_ptr<GameObject>> getBullet() { return bullets; };

public:
	void SetBulletPos(Vec3 pos) { cameraPosForBullet = pos; };
	void SetBulletLook(Vec3 Look) { cameraLookForBullet = Look; };

	int GetHP() { return _hp; };
	int GetCurrAmmo() { return _currAmmo; };
	int GetMaxAmmo() { return _maxAmmo; };
	int GetMoney() { return _money; };
	void SetHP(int hp) { _hp = hp; };
	void SetCurrAmmo(int currAmmo) { _currAmmo = currAmmo; };
	void SetMaxAmmo(int maxAmmo) {  _maxAmmo = maxAmmo; };

	void SetRotateLock(bool lock) { _rotateLock = lock; };
	void SetShopOpened(bool open) { _shopOpened = open; };

	bool MoneyChange(int amount);

private:
	// Shop
	bool			_rotateLock = false;
	bool			_shopOpened = false;
	
	// Status
	float			_speed = 10000.0f;
	POINT			_mousePos = {};
	POINT			_oldMousePos = {};
	float			_cxdelta = 0.f;
	int				_hp = 100;
	int				_Magazine = 0;
	bool			_reloading = 0;
	int				_money = 100000;

	// Weapon
	PLAYER_WEAPON	_currWeapon = PLAYER_WEAPON::PISTOL;
	int				_maxAmmo = 0.f;
	int				_currAmmo = 0.f;
	int				_damage = 25.f;
	int				_pellet = 1;
	int				_reloadPerAmmo = 15;
	float			_rateOfFire = 0.5f;
	float			_reloadMaxTime = 3.f;
	float			_reloadTime = 3.f;
	int				_price = 500;

	// Bullet
	vector<shared_ptr<GameObject>>	bullets;
	Vec3							cameraPosForBullet;
	Vec3							cameraLookForBullet;

	float temp = 300.f;
};

class Bullet : public Component
{
public:
	Bullet();
	virtual ~Bullet();

public:
	virtual void Update() override;

	BULLET_STATE GetState() { return _currState; };
	void SetState(BULLET_STATE state) { _currState = state; };

private:
	float			_lifeTime = 5.f;
	float			_currLifeTime = 5.f;
	BULLET_STATE	_currState = BULLET_STATE::LIVE;
	float			_speed = 100.0f;
};