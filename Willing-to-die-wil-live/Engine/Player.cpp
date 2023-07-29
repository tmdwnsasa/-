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
#include "SceneManager.h"
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
	Bleeding();
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
		if (GetCurrAmmo(_currWeapon) > 0 && _rotateLock == false && _shopOpened == false && _currWeapon != PLAYER_WEAPON::NONE)
		{
			_reloading = false;

			//if(_currWeapon == PLAYER_WEAPON::PISTOL)
			//	GET_SINGLE(SoundManager)->PlaySound("Pistolsound", 0.3f);
			//if (_currWeapon == PLAYER_WEAPON::SMG)
			//	GET_SINGLE(SoundManager)->PlaySound("Smgsound", 0.25f);
			//if (_currWeapon == PLAYER_WEAPON::SHOTGUN)
			//	GET_SINGLE(SoundManager)->PlaySound("Shotgunsound", 0.25f);
			//if (_currWeapon == PLAYER_WEAPON::SNIPER)
			//	GET_SINGLE(SoundManager)->PlaySound("Snipersound", 0.20f);
			Vec3 coutpos = gunObject[0]->GetTransform()->GetLocalPosition();
			MakeMuzzleFlash();


#pragma endregion bullet
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
			if (_currWeapon == PLAYER_WEAPON::PISTOL)
			{
				currPistolAmmo--;
			}
			if (_currWeapon == PLAYER_WEAPON::SMG)
			{
				currSmgAmmo--;
			}
			if (_currWeapon == PLAYER_WEAPON::SHOTGUN)
			{
				currShotgunAmmo--;
			}
			if (_currWeapon == PLAYER_WEAPON::SNIPER)
			{
				currSniperAmmo--;
			}
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

	Reload();

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

	BulletMuzzleFlashErase();
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
		_ammoPrice = 50;
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
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"GunDeffered");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
				gameObject->AddComponent(make_shared<Gun>(PLAYER_WEAPON::PISTOL));
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
		_ammoPrice = 100;
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
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"GunDeffered");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
				gameObject->AddComponent(make_shared<Gun>(PLAYER_WEAPON::SMG));
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
		_ammoPrice = 200;
		_accuracy = 1000;
		_weaponRecoil = 0.3;
		_fullauto = false;

#pragma region Shotgun
		{
			shared_ptr<MeshData> GunMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\shotgun.fbx");
			vector<shared_ptr<GameObject>> gun = GunMesh->Instantiate();

			for (auto& gameObject : gun)
			{
				gameObject->SetName(L"Gun");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(cameraPosForBullet);
				gameObject->GetTransform()->SetLocalScale(Vec3(0.2f, 0.2f, 0.2f));
				gameObject->GetTransform()->LookAt(cameraLookForBullet);
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"GunDeffered");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
				gameObject->AddComponent(make_shared<Gun>(PLAYER_WEAPON::SHOTGUN));
				gunObject.push_back(gameObject);
			}
		}
#pragma endregion
	}

	if (_currWeapon == PLAYER_WEAPON::SNIPER)	
	{
		_damage = 50.f;
		_pellet = 1;
		_maxAmmo = 20;
		_rateOfFire = 0.5f;
		_reloadMaxTime = 2.5f;
		_reloadPerAmmo = _maxAmmo;
		_price = 2500;
		_ammoPrice = 250;
		_accuracy = 10000;
		_weaponRecoil = 3;
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
				shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"GunDeffered");
				gameObject->GetMeshRenderer()->GetMaterial()->SetShader(shader);
				gameObject->AddComponent(make_shared<Gun>(PLAYER_WEAPON::SNIPER));
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

void Player::Reload()
{
	if (_reloading == true)
	{
		_reloadTime -= DELTA_TIME;
		if (_reloadTime <= 0.f)
		{
			if (_currWeapon == PLAYER_WEAPON::PISTOL && currPistolAmmo < _maxAmmo +1 && PistolAmmo > 0)
			{
				if (PistolAmmo >= _maxAmmo - currPistolAmmo)
				{
					if (currPistolAmmo != 0 && PistolAmmo + currPistolAmmo > _maxAmmo )
					{
						PistolAmmo -= _reloadPerAmmo - (currPistolAmmo - 1);
						currPistolAmmo += _reloadPerAmmo - (currPistolAmmo - 1);
					}
					else
					{
						PistolAmmo -= _reloadPerAmmo - currPistolAmmo;
						currPistolAmmo += _reloadPerAmmo - currPistolAmmo;
					}
				}
				else if (PistolAmmo < _reloadPerAmmo)
				{
					currPistolAmmo += PistolAmmo;
					PistolAmmo = 0;
				}
			}
			if (_currWeapon == PLAYER_WEAPON::SMG && currSmgAmmo < _maxAmmo + 1 && SmgAmmo > 0)
			{
				if (SmgAmmo >= _maxAmmo - currSmgAmmo)
				{
					if (currSmgAmmo != 0 && SmgAmmo + currSmgAmmo > _maxAmmo)
					{
						SmgAmmo -= _reloadPerAmmo - (currSmgAmmo - 1);
						currSmgAmmo += _reloadPerAmmo - (currSmgAmmo - 1);
					}
					else
					{
						SmgAmmo -= _reloadPerAmmo - currSmgAmmo;
						currSmgAmmo += _reloadPerAmmo - currSmgAmmo;
					}
				}
				else if (SmgAmmo < _reloadPerAmmo)
				{
					currSmgAmmo += SmgAmmo;
					SmgAmmo = 0;
				}
			}
			if (_currWeapon == PLAYER_WEAPON::SHOTGUN && currShotgunAmmo < _maxAmmo + 1 && ShotgunAmmo > 0)
			{
				ShotgunAmmo -= _reloadPerAmmo;
				currShotgunAmmo += _reloadPerAmmo;
			}
			if (_currWeapon == PLAYER_WEAPON::SNIPER && currSniperAmmo < _maxAmmo + 1 && SniperAmmo > 0)
			{
				if (SniperAmmo >= _maxAmmo - currSniperAmmo)
				{
					if (currSniperAmmo != 0 && SniperAmmo + currSniperAmmo > _maxAmmo)
					{
						SniperAmmo -= _reloadPerAmmo - (currSniperAmmo-1);
						currSniperAmmo += _reloadPerAmmo - (currSniperAmmo-1);
					}
					else
					{
						SniperAmmo -= _reloadPerAmmo - currSniperAmmo;
						currSniperAmmo += _reloadPerAmmo - currSniperAmmo;
					}
				}
				else if (SniperAmmo < _reloadPerAmmo)
				{
					currSniperAmmo += SniperAmmo;
					SniperAmmo = 0;
				}
			}

			_reloadTime = _reloadMaxTime;
			if (currPistolAmmo >= _maxAmmo && _currWeapon == PLAYER_WEAPON::PISTOL)
			{
				_reloading = false;
			}
			if (currSmgAmmo >= _maxAmmo && _currWeapon == PLAYER_WEAPON::SMG)
			{
				_reloading = false;
			}
			if (currShotgunAmmo >= _maxAmmo && _currWeapon == PLAYER_WEAPON::SHOTGUN)
			{
				_reloading = false;
			}
			if (currSniperAmmo >= _maxAmmo && _currWeapon == PLAYER_WEAPON::SNIPER)
			{
				_reloading = false;
			}
		}
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

int Player::GetCurrAmmo(PLAYER_WEAPON weapon)
{
	if (_currWeapon == PLAYER_WEAPON::NONE)
	{
		return 0;
	}
	if (_currWeapon == PLAYER_WEAPON::PISTOL)
	{
		return currPistolAmmo;
	}
	if (_currWeapon == PLAYER_WEAPON::SMG)
	{
		return currSmgAmmo;
	}
	if (_currWeapon == PLAYER_WEAPON::SHOTGUN)
	{
		return currShotgunAmmo;
	}
	if (_currWeapon == PLAYER_WEAPON::SNIPER)
	{
		return currSniperAmmo;
	}
}

int Player::GetMaxAmmo()
{
	if (_currWeapon == PLAYER_WEAPON::NONE)
	{
		return 0;
	}
	if (_currWeapon == PLAYER_WEAPON::PISTOL)
	{
		return PistolAmmo;
	}
	if (_currWeapon == PLAYER_WEAPON::SMG)
	{
		return SmgAmmo;
	}
	if (_currWeapon == PLAYER_WEAPON::SHOTGUN)
	{
		return ShotgunAmmo;
	}
	if (_currWeapon == PLAYER_WEAPON::SNIPER)
	{
		return SniperAmmo;
	}
}


void Player::MakeMuzzleFlash()
{
	shared_ptr<GameObject> muzzleflash = make_shared<GameObject>();
	muzzleflash->SetName(L"MuzzleFlash");
	muzzleflash->AddComponent(make_shared<Transform>());
	muzzleflash->AddComponent(make_shared<MuzzleFlash>());
	muzzleflash->GetTransform()->SetLocalScale(Vec3(80.f, 80.f, 80.f));

	muzzleflash->GetTransform()->LookAt(gunObject[0]->GetTransform()->GetLook());

	Vec3 MuzzleFlashFixedPos = gunObject[0]->GetTransform()->GetLocalPosition();
	if (PLAYER_WEAPON::PISTOL == _currWeapon)
	{
		MuzzleFlashFixedPos += Vec3(0, 60, 0);
		MuzzleFlashFixedPos += gunObject[0]->GetTransform()->GetLook() * 20;
	}
	if (PLAYER_WEAPON::SMG == _currWeapon)
	{
		MuzzleFlashFixedPos += Vec3(0, 20, 0);
		MuzzleFlashFixedPos += gunObject[0]->GetTransform()->GetLook() * 200;
	}
	if (PLAYER_WEAPON::SHOTGUN == _currWeapon)
	{
		MuzzleFlashFixedPos += Vec3(0, 70, 0);
		MuzzleFlashFixedPos += gunObject[0]->GetTransform()->GetLook() * 30;
	}
	if (PLAYER_WEAPON::SNIPER == _currWeapon)
	{
		MuzzleFlashFixedPos += Vec3(0, 140, 0);
		MuzzleFlashFixedPos += gunObject[0]->GetTransform()->GetLook() * 50;
	}
	muzzleflash->GetTransform()->SetLocalPosition(MuzzleFlashFixedPos);

	muzzleflash->SetCheckFrustum(false);
	muzzleflash->SetStatic(false);

	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	{
		shared_ptr<Mesh> rectangleMesh = GET_SINGLE(Resources)->LoadRectangleMesh();
		meshRenderer->SetMesh(rectangleMesh);

		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"MuzzleFlash");
		material->SetInt(0, 1);
		meshRenderer->SetMaterial(material->Clone());
	}
	muzzleflash->AddComponent(meshRenderer);

	muzzleFlashObject.push_back(muzzleflash);
	///////////
	Vec3 coutpos = gunObject[0]->GetTransform()->GetLocalPosition();
}

void Player::AddMaxAmmo(PLAYER_WEAPON weapon)
{
	if (weapon == PLAYER_WEAPON::PISTOL)
	{
		PistolAmmo += 15;
	}
	else if (weapon == PLAYER_WEAPON::SMG)
	{
		SmgAmmo += 25;
	}
	else if (weapon == PLAYER_WEAPON::SHOTGUN)
	{
		ShotgunAmmo += 6;
	}
	else if (weapon == PLAYER_WEAPON::SNIPER)
	{
		SniperAmmo += 20;
	}
	_money -= _ammoPrice;
}

void Player::Bleeding()
{
#pragma region BloodyScreen
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetName(L"BloodyScreen");
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(GEngine->GetWindow().width, GEngine->GetWindow().height, 1000.f));
		obj->GetTransform()->SetLocalPosition(Vec3(0, 0, 900.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

		shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
		meshRenderer->SetMesh(mesh);

		{
			shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"BloodyScreen");
			meshRenderer->SetMaterial(material);
			obj->AddComponent(meshRenderer);
			bleedingUI = obj;
		}
	}
#pragma endregion
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