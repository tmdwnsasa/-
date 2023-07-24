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
//#include "SoundManager.h"
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

void Player::Start()
{
	//ChangeWeapon(PLAYER_WEAPON::PISTOL);
}

void Player::Update()
{
	Vec3 pos = GetTransform()->GetLocalPosition();
	Vec3 oldPos = pos;
	_isMoving = false;

	if (INPUT->GetButton(KEY_TYPE::W))
	{
		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;
		
		//if (GET_SINGLE(SoundManager)->IsPlaying("Footwalksound") == false)
		//{
		//	GET_SINGLE(SoundManager)->PlayLoopSound("Footwalksound", 0.4f);
		//}
		_isMoving = true;
	}

	if (INPUT->GetButton(KEY_TYPE::S))
	{
		pos -= GetTransform()->GetLook() * _speed * DELTA_TIME;
		//if (GET_SINGLE(SoundManager)->IsPlaying("Footwalksound") == false)
		//{
		//	GET_SINGLE(SoundManager)->PlayLoopSound("Footwalksound", 0.4f);
		//}
		_isMoving = true;
	}

	if (INPUT->GetButton(KEY_TYPE::A))
	{
		pos -= GetTransform()->GetRight() * _speed * DELTA_TIME;
		//if (GET_SINGLE(SoundManager)->IsPlaying("Footwalksound") == false)
		//{
		//	GET_SINGLE(SoundManager)->PlayLoopSound("Footwalksound", 0.4f);
		//}
		_isMoving = true;
	}

	if (INPUT->GetButton(KEY_TYPE::D))
	{
		pos += GetTransform()->GetRight() * _speed * DELTA_TIME;
		//if (GET_SINGLE(SoundManager)->IsPlaying("Footwalksound") == false)
		//{
		//	GET_SINGLE(SoundManager)->PlayLoopSound("Footwalksound", 0.4f);
		//}
		_isMoving = true;
	}

	//if (INPUT->GetButtonUp(KEY_TYPE::W) && _isMoving == false)
	//{
	//	GET_SINGLE(SoundManager)->StopLoopSound("Footwalksound");
	//}
	//if (INPUT->GetButtonUp(KEY_TYPE::A) && _isMoving == false)
	//{
	//	GET_SINGLE(SoundManager)->StopLoopSound("Footwalksound");
	//}	
	//if (INPUT->GetButtonUp(KEY_TYPE::S) && _isMoving == false)
	//{
	//	GET_SINGLE(SoundManager)->StopLoopSound("Footwalksound");
	//}
	//if (INPUT->GetButtonUp(KEY_TYPE::D) && _isMoving == false)
	//{
	//	GET_SINGLE(SoundManager)->StopLoopSound("Footwalksound");
	//}

	//점프 구현 필요
	if (INPUT->GetButton(KEY_TYPE::SPACE))
	{
		pos += GetTransform()->GetUp() * _speed * DELTA_TIME;
		//if (!_jump)
		//{
		//	pos += GetTransform()->GetUp() * _speed * DELTA_TIME;
		//	_jump = true;
		//}
	}

	if (INPUT->GetButton(KEY_TYPE::SHIFT))
	{
		_running = true;
		if (_stamina > 0.f)
		{
			_speed += 1000;
			_stamina -= DELTA_TIME;
		}
		else
		{
			_speed = 5000.0f;
		}
	}

	if (INPUT->GetButtonUp(KEY_TYPE::SHIFT))
	{
		_running = false;
		_speed = 5000;
		_stamina -= DELTA_TIME;
	}

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
	{
		PlayerRotate();
	}

	if (INPUT->GetButtonDown(KEY_TYPE::ESC))
	{
		_rotateLock = true;
	}

	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
	{
		if (_currAmmo > 0 && _rotateLock == false && _shopOpened == false && _currWeapon != PLAYER_WEAPON::NONE)
		{
			_reloading = false;

//			if(_currWeapon == PLAYER_WEAPON::PISTOL)
//				GET_SINGLE(SoundManager)->PlaySound("Pistolsound", 0.3f);
//			if (_currWeapon == PLAYER_WEAPON::SMG)
//				GET_SINGLE(SoundManager)->PlaySound("Smgsound", 0.25f);
//			if (_currWeapon == PLAYER_WEAPON::SHOTGUN)
//				GET_SINGLE(SoundManager)->PlaySound("Shotgunsound", 0.25f);
//			if (_currWeapon == PLAYER_WEAPON::RIFLE)
//				GET_SINGLE(SoundManager)->PlaySound("Snipersound", 0.20f);
//
#pragma region Muzzle Flash
			{
				shared_ptr<GameObject> muzzleflash = make_shared<GameObject>();
				muzzleflash->SetName(L"MuzzleFlash");
				muzzleflash->AddComponent(make_shared<Transform>());
				muzzleflash->AddComponent(make_shared<MuzzleFlash>());
				muzzleflash->GetTransform()->SetLocalScale(Vec3(80.f, 80.f, 80.f));
				muzzleflash->GetTransform()->LookAt(gunObject[0]->GetTransform()->GetLook());
				
				Vec3 MuzzleFlashFixedPos = gunObject[0]->GetTransform()->GetLocalPosition();
				MuzzleFlashFixedPos += Vec3(0, 100, 0);
				MuzzleFlashFixedPos += gunObject[0]->GetTransform()->GetLook() * 45;
				muzzleflash->GetTransform()->SetLocalPosition(MuzzleFlashFixedPos);

				muzzleflash->SetCheckFrustum(false);
				muzzleflash->SetStatic(false);

				shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
				{
					shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadRectangleMesh();
					meshRenderer->SetMesh(sphereMesh);
					
					shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"MuzzleFlash");
					meshRenderer->SetMaterial(material->Clone());
				}
				muzzleflash->AddComponent(meshRenderer);

				muzzleFlashObject.push_back(muzzleflash);
			}

#pragma endregion 

#pragma region bullet

			for (int i = 0; i < _pellet; i++)
			{
				shared_ptr<GameObject> bullet = make_shared<GameObject>();

				bullet->SetName(L"Bullet");
				bullet->AddComponent(make_shared<Transform>());
				bullet->AddComponent(make_shared<Bullet>());
				bullet->SetCheckFrustum(false);
				shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();

				bullet->GetTransform()->SetLocalPosition(Vec3(GetTransform()->GetLocalPosition().x, GetTransform()->GetLocalPosition().y, GetTransform()->GetLocalPosition().z-11));
				bullet->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				bullet->GetTransform()->LookAt(cameraLookForBullet);
				Vec3 rot = bullet->GetTransform()->GetLocalRotation();
				bullet->GetTransform()->SetLocalRotation(Vec3(rot.x + ((float)(RandomInt() - 50) / _accuracy), rot.y + ((float)(RandomInt() - 50) / _accuracy), rot.z));
				bullet->SetStatic(false);

				//shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
				//{
				//	shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
				//	meshRenderer->SetMesh(sphereMesh);
				//}
				//{
				//	shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject");
				//	material->SetInt(0, 1);
				//	meshRenderer->SetMaterial(material);
				//}

				boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
				boxCollider->SetExtents(Vec3(10.f, 10.f, 10.f));
				bullet->AddComponent(boxCollider);
				//bullet->AddComponent(meshRenderer);
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

	if (_running == false)
	{
		_stamina += DELTA_TIME;
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

	void BulletMuzzleFlashErase();
	GetTransform()->SetLocalPosition(pos);
	if (_jump)
	{
		pos -= GetTransform()->GetUp() * _speed * DELTA_TIME;
		_jump = false;
	}
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
	gunObject.clear();
	if (_currWeapon == PLAYER_WEAPON::PISTOL)
	{
		_damage = 25.f;
		_pellet = 1;
		_maxAmmo = 15;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 0.5f;
		_reloadPerAmmo = _maxAmmo;
		_price = 500;
		_accuracy = 5000;
		_weaponRecoil = 2;
		_fullauto = false;

#pragma region Pistol
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\beretta m9.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(0.2f, 0.2f, 0.2f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Gun");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
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
		_accuracy = 3000;
		_weaponRecoil = 1;
		_fullauto = true;

#pragma region SMG
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\m4a1_s.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(0.2f, 0.2f, 0.2f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Gun");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
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
		_reloadPerAmmo = 1;
		_price = 2000;
		_accuracy = 1000;
		_weaponRecoil = 0.5;
		_fullauto = false;

#pragma region Shotgun
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\ShotgunFix.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(0.2f, 0.2f, 0.2f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Gun");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
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
		_accuracy = 10000;
		_weaponRecoil = 4;
		_fullauto = false;

#pragma region Rifle
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\AWP_Dragon_Lore.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(0.2f, 0.2f, 0.2f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Gun");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
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

void Player::BulletMuzzleFlashErase()
{
	for (int i = 0; i < bullets.size(); i++)
	{
		if (bullets[i]->GetBullet()->GetState() == BULLET_STATE::DEAD)
		{
			bullets.erase(bullets.begin() + i);
		}
	}
	for (int i = 0; i < muzzleFlashObject.size(); i++)
	{
		if (muzzleFlashObject[i]->GetMuzzleFlash()->GetState() == MUZZLEFLASH_STATE::DEAD)
		{
			muzzleFlashObject.erase(muzzleFlashObject.begin() + i);
		}
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

MuzzleFlash::MuzzleFlash() : Component(COMPONENT_TYPE::MUZZLEFLASH)
{
}

MuzzleFlash::~MuzzleFlash()
{

}

void MuzzleFlash::Update()
{
	_currLifeTime -= DELTA_TIME;

	if (_currLifeTime <= 0.f)
	{
		_currState = MUZZLEFLASH_STATE::DEAD;
	}
}