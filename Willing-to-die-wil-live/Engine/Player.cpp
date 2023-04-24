#include "pch.h"
#include "Engine.h"
#include "Player.h"
#include "Input.h"
#include "Animator.h"
#include "Transform.h"
#include "Timer.h"
#include "GameObject.h"

#include "Mesh.h"
#include "MeshRenderer.h"
#include "BoxCollider.h"
#include "Resources.h"

//////////////////////////////////////////////////
// Player
//////////////////////////////////////////////////
Player::Player() : Component(COMPONENT_TYPE::PLAYER)
{
	_oldMousePos = { GEngine->GetWindow().width / 2, GEngine->GetWindow().height / 2 };
	ChangeWeapon(PLAYER_WEAPON::SHOTGUN);
}

Player::~Player()
{

}

void Player::Update()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	if (INPUT->GetButton(KEY_TYPE::W))
		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::S))
		pos -= GetTransform()->GetLook() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::A))
		pos -= GetTransform()->GetRight() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::D))
		pos += GetTransform()->GetRight() * _speed * DELTA_TIME;

	if (INPUT->GetButtonDown(KEY_TYPE::R))
	{
		_reloading = true;
		_reloadTime = _reloadMaxTime;
	}

	if (INPUT->GetButtonDown(KEY_TYPE::J))
		_hp -= 10;

	if (INPUT->GetButtonDown(KEY_TYPE::K))
		_hp += 10;


	//if (INPUT->GetButtonDown(KEY_TYPE::KEY_1))
	//{
	//	int32 count = GetAnimator()->GetAnimCount();
	//	int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

	//	int32 index = (currentIndex + 1) % count;
	//	GetAnimator()->Play(index);
	//}

	//if (INPUT->GetButtonDown(KEY_TYPE::KEY_2))
	//{
	//	int32 count = GetAnimator()->GetAnimCount();
	//	int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

	//	int32 index = (currentIndex - 1 + count) % count;
	//	GetAnimator()->Play(index);
	//}

	if (_rotateLock == false && _shopOpened == false)
		PlayerRotate();

	if (INPUT->GetButtonDown(KEY_TYPE::ESC))
	{
		_rotateLock = true;
	}

	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
	{
		if (_currAmmo > 0 && _rotateLock == false && _shopOpened == false)
		{
			_reloading = false;
			for (int i = 0; i < _pellet; i++)
			{
				shared_ptr<GameObject> bullet = make_shared<GameObject>();

				bullet->SetName(L"Bullet");
				bullet->AddComponent(make_shared<Transform>());
				bullet->AddComponent(make_shared<Bullet>());
				bullet->SetCheckFrustum(false);
				shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();

				bullet->GetTransform()->SetLocalPosition(cameraPosForBullet);
				bullet->GetTransform()->SetLocalScale(Vec3(10.f, 10.f, 10.f));
				bullet->GetTransform()->LookAt(cameraLookForBullet);
				bullet->SetStatic(false);

				shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
				{
					shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
					meshRenderer->SetMesh(sphereMesh);
				}
				{
					shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject");
					material->SetInt(0, 1);
					meshRenderer->SetMaterial(material);
				}

				boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
				boxCollider->SetExtents(Vec3(10.f, 10.f, 10.f));
				bullet->AddComponent(boxCollider);
				bullet->AddComponent(meshRenderer);
				bullets.push_back(bullet);
				temp += 300.f;
			}
			_currAmmo--;
		}
		else
		{
			_reloading = true;
			_reloadTime = _reloadMaxTime;
		}

		if (_rotateLock == true)
		{
			_rotateLock = false;
			SetCursorPos(INPUT->GetOldMousePos().x, INPUT->GetOldMousePos().y);
		}
	}

	if (_reloading == true)
	{
		_reloadTime -= DELTA_TIME;
		if (_reloadTime <= 0.f)
		{
			_currAmmo = _maxAmmo;
			_reloadTime = _reloadMaxTime;
			_reloading = false;
		}
	}
	GetTransform()->SetLocalPosition(pos);
}

void Player::PlayerRotate()
{
	Vec3 rotation = GetTransform()->GetLocalRotation();
	_mousePos = INPUT->GetMousePos();
	_cxdelta += (_mousePos.x - _oldMousePos.x) / 300.f;
	rotation.y = _cxdelta;
	GetTransform()->SetLocalRotation(rotation);
}

void Player::ChangeWeapon(PLAYER_WEAPON weapon)
{
	_currWeapon = weapon;

	if (_currWeapon == PLAYER_WEAPON::PISTOL)
	{
		_damage = 25.f;
		_pellet = 1;
		_maxAmmo = 15;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 0.5f;
		_reloadPerAmmo = _maxAmmo;
	}

	if (_currWeapon == PLAYER_WEAPON::SMG)
	{
		_damage = 25.f;
		_pellet = 1;
		_maxAmmo = 25;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 2.5f;
		_reloadPerAmmo = _maxAmmo;
	}

	if (_currWeapon == PLAYER_WEAPON::SHOTGUN)
	{
		_damage = 25.f;
		_pellet = 8;
		_maxAmmo = 6;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 2.5f;
		_reloadPerAmmo = 2;
	}

	if (_currWeapon == PLAYER_WEAPON::RIFLE)
	{
		_damage = 50.f;
		_pellet = 1;
		_maxAmmo = 20;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 2.5f;
		_reloadPerAmmo = _maxAmmo;
	}
}

bool Player::MoneyChange(int amount)
{
	if (amount < 0)
	{
		if (_money + amount >= 0)
		{
			_money += amount;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		_money += amount;
		return true;
	}
}

//////////////////////////////////////////////////
// Bullet
//////////////////////////////////////////////////

Bullet::Bullet() : Component(COMPONENT_TYPE::BULLET)
{

}

Bullet::~Bullet()
{

}

void Bullet::Update()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	pos += GetTransform()->GetLook() * _speed * DELTA_TIME;

	_currLifeTime -= DELTA_TIME;

	if (_currLifeTime <= 0.f)
	{
		_currState = BULLET_STATE::DEAD;
	}

	GetTransform()->SetLocalPosition(pos);
}