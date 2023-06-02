#include "pch.h"
#include "Engine.h"
#include "Player.h"
#include "Input.h"
#include "Animator.h"
#include "Transform.h"
#include "Timer.h"
#include "GameObject.h"
#include "Gun.h"

#include "Mesh.h"
#include "MeshData.h"
#include "MeshRenderer.h"
#include "BoxCollider.h"
#include "Resources.h"
#include <iostream>
//////////////////////////////////////////////////
// Player
//////////////////////////////////////////////////
Player::Player() : Component(COMPONENT_TYPE::PLAYER)
{
	_oldMousePos = { GEngine->GetWindow().width / 2, GEngine->GetWindow().height / 2 };
}

Player::~Player()
{

}

void Player::Update()
{
	for (auto iter = bullets.begin(); iter != bullets.end(); iter++)
	{
		if (iter->get()->GetBullet()->GetState() == BULLET_STATE::DEAD)
		{
			//bullets.erase(iter);
		}
	}

	Vec3 pos = GetTransform()->GetLocalPosition();
	Vec3 oldPos = pos;


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
		if(_reloading == false)
			_reloadTime = _reloadMaxTime;

		_reloading = true;
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

				bullet->GetTransform()->SetLocalPosition(GetTransform()->GetLocalPosition());
				bullet->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				bullet->GetTransform()->LookAt(cameraLookForBullet);
				Vec3 rot = bullet->GetTransform()->GetLocalRotation();
				float a = (((float)(RandomInt() - 50.f)) / 100000.f);
				bullet->GetTransform()->SetLocalRotation(Vec3(rot.x + ((float)(RandomInt() - 50) / 1000), rot.y + ((float)(RandomInt() - 50) / 1000), rot.z + ((float)(RandomInt() - 50) / 1000)));
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
			_currAmmo += _reloadPerAmmo;
			if (_currAmmo >= _maxAmmo + 1)
			{
				_currAmmo = _maxAmmo + 1;
			}
			_reloadTime = _reloadMaxTime;
			if (_currAmmo >= _maxAmmo)
			{
				_reloading = false;

			}
		}
	}

	if (_front == true)
	{
		if (pos.z > oldPos.z)
			pos.z = oldPos.z;
	}
	if (_back == true)
	{
		if (pos.z < oldPos.z)
			pos.z = oldPos.z;
	}
	if (_right == true)
	{
		if (pos.x > oldPos.x)
			pos.x = oldPos.x;
	}
	if (_left == true)
	{
		if (pos.x < oldPos.x)
			pos.x = oldPos.x;
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
		_price = 500;

#pragma region Gun
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\shot.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				gameObject->AddComponent(make_shared<Gun>());
				gunObject.push_back(gameObject);
			}

		}
#pragma endregion

	}

	if (_currWeapon == PLAYER_WEAPON::SMG)
	{
		_damage = 25.f;
		_pellet = 1;
		_maxAmmo = 25;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 2.5f;
		_reloadPerAmmo = _maxAmmo;
		_price = 1000;

#pragma region Gun
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\shot.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				gameObject->AddComponent(make_shared<Gun>());
				gunObject.push_back(gameObject);
			}

		}
#pragma endregion
	}

	if (_currWeapon == PLAYER_WEAPON::SHOTGUN)
	{
		_damage = 25.f;
		_pellet = 8;
		_maxAmmo = 6;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 0.5f;
		_reloadPerAmmo = 2;
		_price = 2000;

#pragma region Gun
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\shot.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				gameObject->AddComponent(make_shared<Gun>());
				gunObject.push_back(gameObject);
			}

		}
#pragma endregion
	}

	if (_currWeapon == PLAYER_WEAPON::RIFLE)
	{
		_damage = 50.f;
		_pellet = 1;
		_maxAmmo = 20;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 2.5f;
		_reloadPerAmmo = _maxAmmo;
		_price = 2000;

#pragma region Gun
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\shot.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				gameObject->AddComponent(make_shared<Gun>());
				gunObject.push_back(gameObject);
			}

		}
#pragma endregion
	}
	_money -= _price;
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