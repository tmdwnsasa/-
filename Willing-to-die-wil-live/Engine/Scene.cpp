#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "Light.h"
#include "Engine.h"
#include "Resources.h"
#include "Transform.h"
#include "CameraScript.h"
#include "MeshRenderer.h"
#include "MeshData.h"
#include "Material.h"
#include "Astar.h"
#include "TileMap.h"
#include "Enemy.h"
#include "StalkerEnemy.h"
#include "BruserEnemy.h"
#include "Player.h"
#include "Input.h"
#include "BoxCollider.h"
#include "Font.h"
#include "Shop.h"
#include "Button.h"
#include "Gun.h"
#include "SoundManager.h"
#include <iostream>

//cout 출력용 코드
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")


void Scene::Awake()
{
	//CursorClipping();
	_gameObjects.reserve(1000);
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Awake();
	}

	ZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Parasite.fbx");
	StalkerZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\war.fbx");
	BruserZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\SpZombie.fbx");

	//parasite,war,Spzombie
}

void Scene::Start()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Start();
	}
}

void Scene::Update()
{
	//// 플레이어 위치 저장
	//for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	//{
	//	if (gameObject->GetName() == L"Player")
	//	{
	//		float x, y, z;
	//		x = gameObject->GetTransform()->GetLocalPosition().x;
	//		y = gameObject->GetTransform()->GetLocalPosition().y;
	//		z = gameObject->GetTransform()->GetLocalPosition().z;
	//	}
	//}
	MakeEnemy(CurrentWave);

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Gun3")
		{
			//cout << gameObject->GetTransform()->GetLocalPosition().x << ", " << gameObject->GetTransform()->GetLocalPosition().y << "," << gameObject->GetTransform()->GetLocalPosition().z << endl;
			for (const shared_ptr<GameObject>& object : _gameObjects)
			{
				if (object->GetName() == L"Player")
				{
					gameObject->GetTransform()->SetLocalPosition(object->GetTransform()->GetLocalPosition());
				}
			}
		}
	}

	//상점열기
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Shop")
		{
			if (_shopOpened == true)
			{
				if (gameObject->GetShop()->GetPurchase() == true)
				{
					for (auto& object : _gameObjects)
					{
						if (object->GetName() == L"Player")
						{
							object->GetPlayer()->ChangeWeapon(gameObject->GetShop()->GetSelected()->GetButton()->GetMerchandise());
							for (auto& makeTrash : _gameObjects)	// 무기를 사면 총을 지우고
							{
								if (makeTrash->GetName() == L"Gun")
								{
									_trashBin.push_back(makeTrash);
								}
							}
							for (auto& gunobject : object->GetPlayer()->GetGun())	//총을 만든다
							{
								_gameObjects.push_back(gunobject);
							}
						}
					}
					gameObject->GetShop()->SetPurchase(false);
				}
			}
			if (gameObject->GetShop()->GetShopState() == _shopOpened)
			{
				break;
			}
			else
			{
				_shopOpened = gameObject->GetShop()->GetShopState();
				if (_shopOpened == true)
				{
					for (auto& gameObject : gameObject->GetShop()->GetShopObjects())
					{
						_gameObjects.push_back(gameObject);
					}
				}
				else
				{
					for (auto& gameObject : gameObject->GetShop()->GetShopObjects())
					{
						_trashBin.push_back(gameObject);
					}
				}
			}
		}

		if (gameObject->GetName() == L"Player")
		{
			gameObject->GetPlayer()->SetShopOpened(_shopOpened);
		}

		if (gameObject->GetName() == L"Main_Camera")
		{
			gameObject->GetCameraScript()->SetShopOpened(_shopOpened);
		}
	}

	// 총알 생성
	int temp = 0;
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Player")
		{
			for (const shared_ptr<GameObject>& bullet : gameObject->GetPlayer()->GetBullet())
			{
				if (bullet->GetBullet()->GetState() == BULLET_STATE::DEAD)
				{
					_trashBin.push_back(bullet);
				}

				if (bullet->GetBullet()->GetState() == BULLET_STATE::LIVE)
				{
					for (const shared_ptr<GameObject>& gun : _gameObjects)
					{
						if (gun->GetGun() != nullptr)
						{
							gun->GetGun()->SetRecoil(gameObject->GetPlayer()->GetRecoil());
						}
					}

					for (const shared_ptr<GameObject>& camera : _gameObjects)
					{
						if (camera->GetCameraScript() != nullptr)
						{
							camera->GetCameraScript()->SetRecoil(gameObject->GetPlayer()->GetRecoil());
						}
					}

					for (const shared_ptr<GameObject>& muzzleflash : gameObject->GetPlayer()->GetMuzzleFlash())
					{
						_gameObjects.push_back(muzzleflash);
					}

					_gameObjects.push_back(bullet);
					bullet->GetBullet()->SetState(BULLET_STATE::SHOOT);
				}
			}
			for (const shared_ptr<GameObject>& muzzleflash : gameObject->GetPlayer()->GetMuzzleFlash())
			{
				if (muzzleflash->GetMuzzleFlash()->GetState() == MUZZLEFLASH_STATE::DEAD)
				{
					_trashBin.push_back(muzzleflash);
				}
			}
		}
	}

	SetPlayerPosToEnemy();

	////복수 생성
	/*map<wstring, pair<uint32, vector<shared_ptr<GameObject>>>> name;
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		name[gameObject->GetName()].first++;
		name[gameObject->GetName()].second.push_back(gameObject);
	}

	for (auto& object : name)
	{
		if (object.first != L"")
		{
			if (object.second.first > 1)
			{
				for (auto& nameObject : object.second.second)
					if (nameObject->GetMeshRenderer() != NULL)
						nameObject->GetMeshRenderer()->GetMaterial()->SetInt(0, 1);
			}

			else
			{

				if ((*object.second.second.begin())->GetMeshRenderer() != NULL)
				{
					(*object.second.second.begin())->GetMeshRenderer()->GetMaterial()->SetInt(0, 0);
				}
			}
		}
<<<<<<< HEAD
	}

	// Wave
	//Enemy수를 Wave마다 정하고 생성
	//EnemyCount가 0이 되면 Wave 체인지
	CheckWave();

=======
	}*/

	CollisionPlayerToWall();

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Update();
	}
}

void Scene::LateUpdate()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Main_Camera")
			SetCameraPosToPlayer();

		//총알 <-> 적  충돌처리
		if (gameObject->GetName() == L"Bullet")
		{
			for (const shared_ptr<GameObject>& Object : _gameObjects)
			{
				if (Object->GetName() == L"Enemy")
				{
					if (Object->GetBoxCollider()->Intersects(gameObject->GetBoxCollider()->GetColliderBox()) == true)
					{
						gameObject->GetBullet()->SetState(BULLET_STATE::DEAD);
						Object->GetEnemy()->LostHp();
						EnemyHp = Object->GetEnemy()->CurHp();
						_trashBin.push_back(gameObject);

						//cout << EnemyHp << endl;
						if (EnemyHp <= 0)
						{
							Object->GetBoxCollider()->SetOnOff(false);
							//Object->GetEnemy()->AnimationCount();
							DeathCount++;
							_trashBin.push_back(gameObject);
							
							//_trashBin.push_back(Object);
						}
					}
				}
			}
		}
		//wave 변경
		if (DeathCount == EnemyCount)
		{
			//Wave 끝
			CurrentWave++;
		}
		//체력, 총알, 돈 출력
		if (gameObject->GetName() == L"Player")
		{
			for (const shared_ptr<GameObject>& Object : _gameObjects)
			{
				if (Object->GetName() == L"HealthText")
				{
					shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
					Object->GetMeshRenderer()->SetMesh(GET_SINGLE(Resources)->LoadFontMesh(Object->GetFont()->GetTextVB(to_string(gameObject->GetPlayer()->GetHP()))));
				}
				if (Object->GetName() == L"BulletText")
				{
					shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
					Object->GetMeshRenderer()->SetMesh(GET_SINGLE(Resources)->LoadFontMesh(Object->GetFont()->GetTextVB(to_string(gameObject->GetPlayer()->GetCurrAmmo()))));
				}
				if (Object->GetName() == L"MoneyText")
				{
					shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
					Object->GetMeshRenderer()->SetMesh(GET_SINGLE(Resources)->LoadFontMesh(Object->GetFont()->GetTextVB(to_string(gameObject->GetPlayer()->GetMoney()))));
				}
			}
		}
		gameObject->LateUpdate();
	}
}

void Scene::FinalUpdate()
{
	for (const shared_ptr<GameObject>& trash : _trashBin)
	{
		RemoveGameObject(trash);
	}
	_trashBin.clear();
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->FinalUpdate();
	}
}

shared_ptr<Camera> Scene::GetMainCamera()
{
	if (_cameras.empty())
		return nullptr;

	return _cameras[0];
}

void Scene::Render()
{
	PushLightData();

	ClearRTV();

	RenderShadow();

	RenderDeferred();

	RenderLights();

	RenderFinal();

	RenderForward();
}

void Scene::ClearRTV()
{
	// SwapChain Group 초기화
	int8 backIndex = GEngine->GetSwapChain()->GetBackBufferIndex();
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->ClearRenderTargetView(backIndex);
	// Shadow Group 초기화
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->ClearRenderTargetView();
	// Deferred Group 초기화
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->ClearRenderTargetView();
	// Lighting Group 초기화
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->ClearRenderTargetView();
}

void Scene::RenderShadow()
{
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->OMSetRenderTargets();

	for (auto& light : _lights)
	{
		if (light->GetLightType() != LIGHT_TYPE::DIRECTIONAL_LIGHT)
			continue;

		light->RenderShadow();
	}

	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->WaitTargetToResource();
}

void Scene::RenderDeferred()
{
	// Deferred OMSet
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->OMSetRenderTargets();

	shared_ptr<Camera> mainCamera = _cameras[0];
	mainCamera->SortGameObject();
	mainCamera->Render_Deferred();

	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->WaitTargetToResource();
}

void Scene::RenderLights()
{
	shared_ptr<Camera> mainCamera = _cameras[0];
	Camera::S_MatView = mainCamera->GetViewMatrix();
	Camera::S_MatProjection = mainCamera->GetProjectionMatrix();

	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->OMSetRenderTargets();

	// 광원을 그린다.
	for (auto& light : _lights)
	{
		light->Render();
	}

	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->WaitTargetToResource();
}

void Scene::RenderFinal()
{
	// Swapchain OMSet
	int8 backIndex = GEngine->GetSwapChain()->GetBackBufferIndex();
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->OMSetRenderTargets(1, backIndex);

	GET_SINGLE(Resources)->Get<Material>(L"Final")->PushGraphicsData();
	GET_SINGLE(Resources)->Get<Mesh>(L"Rectangle")->Render();
}

void Scene::RenderForward()
{
	shared_ptr<Camera> mainCamera = _cameras[0];
	mainCamera->Render_Forward();

	for (auto& camera : _cameras)
	{
		if (camera == mainCamera)
			continue;

		camera->SortGameObject();
		camera->Render_Forward();
	}
}

void Scene::PushLightData()
{
	LightParams lightParams = {};

	for (auto& light : _lights)
	{
		const LightInfo& lightInfo = light->GetLightInfo();

		light->SetLightIndex(lightParams.lightCount);

		lightParams.lights[lightParams.lightCount] = lightInfo;
		lightParams.lightCount++;
	}

	CONST_BUFFER(CONSTANT_BUFFER_TYPE::GLOBAL)->SetGraphicsGlobalData(&lightParams, sizeof(lightParams));
}

void Scene::AddGameObject(shared_ptr<GameObject> gameObject)
{
	if (gameObject->GetCamera() != nullptr)
	{
		_cameras.push_back(gameObject->GetCamera());
	}
	else if (gameObject->GetLight() != nullptr)
	{
		_lights.push_back(gameObject->GetLight());
	}

	_gameObjects.push_back(gameObject);
}

void Scene::RemoveGameObject(shared_ptr<GameObject> gameObject)
{
	if (gameObject->GetCamera())
	{
		auto findIt = std::find(_cameras.begin(), _cameras.end(), gameObject->GetCamera());
		if (findIt != _cameras.end())
			_cameras.erase(findIt);
	}
	else if (gameObject->GetLight())
	{
		auto findIt = std::find(_lights.begin(), _lights.end(), gameObject->GetLight());
		if (findIt != _lights.end())
			_lights.erase(findIt);
	}

	auto findIt = std::find(_gameObjects.begin(), _gameObjects.end(), gameObject);
	if (findIt != _gameObjects.end())
		_gameObjects.erase(findIt);
}

void Scene::SetCameraPosToPlayer()	//플레이어와 카메라에게 서로의 위치를 준다.
{
	Vec3 playerPos;
	Vec3 cameraPos;
	Vec3 cameraLook;
	shared_ptr<Transform> cameraTrasform;

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Player")
		{
			playerPos = gameObject->GetTransform()->GetLocalPosition();
		}

	}


	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Main_Camera")
		{
			gameObject->GetTransform()->SetLocalPosition(playerPos);
			cameraPos = gameObject->GetTransform()->GetLocalPosition();
			cameraLook = gameObject->GetTransform()->GetLook();
			cameraTrasform = gameObject->GetTransform();
		}
		if (gameObject->GetName() == L"Light1")
		{
			gameObject->GetTransform()->SetLocalPosition(Vec3(playerPos.x-800.f, 2000.f, playerPos.z-500.f));
		}
	}


	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Player")
		{
			gameObject->GetPlayer()->SetBulletPos(Vec3(cameraPos.x, cameraPos.y, cameraPos.z-20.f));
			gameObject->GetPlayer()->SetBulletLook(cameraLook);
		}
		if (gameObject->GetName() == L"Gun")
		{
			gameObject->GetGun()->SetCameraTransform(cameraTrasform);
		}
	}
}


void Scene::SetPlayerPosToEnemy()
{
	Vec3 PlayerPos;
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
		if (gameObject->GetName() == L"Player")
			PlayerPos = gameObject->GetTransform()->GetLocalPosition();
	
	PlayerObPos = PlayerPos;

}

void Scene::SetWallPosToEnemy()
{
	Vec3 WallPos;
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
		if (gameObject->GetName() == L"Wall")
			WallPos = gameObject->GetTransform()->GetLocalPosition();

	WallObPos = WallPos;
}

void Scene::CursorClipping()
{
	POINT p1, p2;

	::GetClientRect(GEngine->GetWindow().hwnd, &rc);

	p1.x = rc.left;
	p1.y = rc.top;
	p2.x = rc.right;
	p2.y = rc.bottom;

	::ClientToScreen(GEngine->GetWindow().hwnd, &p1);
	::ClientToScreen(GEngine->GetWindow().hwnd, &p2);

	rc.left = p1.x;
	rc.top = p1.y;
	rc.right = p2.x;
	rc.bottom = p2.y;

	::ClipCursor(&rc);
}

void Scene::CollisionPlayerToWall()
{
	Vec4 playerPos;
	Transform transform;
	Vec4 front;
	Vec4 right;

	float minDistanceF = FLT_MAX;
	float minDistanceB = FLT_MAX;
	float minDistanceL = FLT_MAX;
	float minDistanceR = FLT_MAX;

	shared_ptr<GameObject> picked;
	for (auto& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Player")
		{
			Vec3 Pos = gameObject->GetTransform()->GetLocalPosition();
			Vec3 Look = gameObject->GetTransform()->GetLook();
			Vec3 Right = gameObject->GetTransform()->GetRight();
			playerPos = Vec4(Pos.x, Pos.y, Pos.z, 1.f);
			front = Vec4(transform.GetLook().x, transform.GetLook().y, transform.GetLook().z, 0.0f);
			right = Vec4(transform.GetRight().x, transform.GetRight().y, transform.GetRight().z, 0.0f);
		}

	}

	for (auto& gameObject : _gameObjects)
	{
		if (gameObject->GetBoxCollider() == nullptr)
			continue;

		if (gameObject->GetName() != L"Wall")
			continue;

		float distanceF = FLT_MAX;
		float distanceB = FLT_MAX;
		float distanceL = FLT_MAX;
		float distanceR = FLT_MAX;

		if (gameObject->GetBoxCollider()->Intersects(playerPos, front , OUT distanceF) == true)
		{
			if (distanceF < minDistanceF)
			{
				minDistanceF = distanceF;
				picked = gameObject;
			}
		}

		if (gameObject->GetBoxCollider()->Intersects(playerPos, -front, OUT distanceB) == true)
		{
			if (distanceB < minDistanceB)
			{
				minDistanceB = distanceB;
				picked = gameObject;
			}
		}

		if (gameObject->GetBoxCollider()->Intersects(playerPos, right, OUT distanceR) == true)
		{
			if (distanceR < minDistanceR)
			{
				minDistanceR = distanceR;
				picked = gameObject;
			}
		}

		if (gameObject->GetBoxCollider()->Intersects(playerPos, -right, OUT distanceL) == true)
		{
			if (distanceL < minDistanceL)
			{
				minDistanceL = distanceL;
				picked = gameObject;
			}
		}
	}
	for (auto& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == L"Player")
		{
			if (minDistanceF < DistanceWall && 0 < DistanceWall)
				gameObject->GetPlayer()->collisionFront(true);
			else
				gameObject->GetPlayer()->collisionFront(false);

			if (minDistanceB < DistanceWall && 0 < DistanceWall)
			gameObject->GetPlayer()->collisionBack(true);
			else
				gameObject->GetPlayer()->collisionBack(false);

			if (minDistanceR < DistanceWall && 0 < DistanceWall)
				gameObject->GetPlayer()->collisionRight(true);
			else
				gameObject->GetPlayer()->collisionRight(false);

			if (minDistanceL < DistanceWall && 0 < DistanceWall)
				gameObject->GetPlayer()->collisionLeft(true);
			else
				gameObject->GetPlayer()->collisionLeft(false);

		}
	}
}

void Scene:: CheckWave()
{
	switch (CurrentWave)
	{
	case 1:
		EnemyCount = Wave1;
		break;
	case 2:
		EnemyCount = Wave2;
		break;

	default:
		break;
	}
}

void Scene::MakeEnemy(int Wave)
{
	// 개체수는 나중에 생각
	switch (Wave)
	{
	case 1:
		ZCount = 5;
		BrZCount = 0;
		STZCount = 0;
		break;
	case 2:
		ZCount = 5;
		BrZCount = 1;
		STZCount = 2;
		break;
	case 3:
		ZCount = 5;
		BrZCount = 5;
		STZCount = 5;
		break;
	default:
		break;
	}
	for (int i = 0; i < ZCount; i++)
	{

		vector<shared_ptr<GameObject>> gameObjects = ZombieMesh->Instantiate();


		for (auto& gameObject : gameObjects)
		{

			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(150.f, 300.f, 150.f));

			gameObject->SetName(L"Enemy");
			gameObject->SetCheckFrustum(false);
			gameObject->GetTransform()->SetLocalPosition(Vec3(1200.f * (i + 1), -100.f, -900.f));
			gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
			gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
			gameObject->AddComponent(boxCollider);
			gameObject->AddComponent(make_shared<Enemy>());
			//gameObject->GetMeshRenderer()->GetMaterial()->SetInt(0, 1);
			AddGameObject(gameObject);
		}
	}

	for (int i = 0; i < STZCount; i++)
	{

		vector<shared_ptr<GameObject>> gameObjects = StalkerZombieMesh->Instantiate();


		for (auto& gameObject : gameObjects)
		{

			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(150.f, 300.f, 150.f));

			gameObject->SetName(L"STEnemy");
			gameObject->SetCheckFrustum(false);
			gameObject->GetTransform()->SetLocalPosition(Vec3(1200.f * (i + 1), -100.f, -900.f));
			gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
			gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
			gameObject->AddComponent(boxCollider);
			gameObject->AddComponent(make_shared<StalkerEnemy>());
			//gameObject->GetMeshRenderer()->GetMaterial()->SetInt(0, 1);
			AddGameObject(gameObject);
		}
	}

	for (int i = 0; i < BrZCount; i++)
	{

		vector<shared_ptr<GameObject>> gameObjects = BruserZombieMesh->Instantiate();


		for (auto& gameObject : gameObjects)
		{

			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(150.f, 300.f, 150.f));

			gameObject->SetName(L"BrEnemy");
			gameObject->SetCheckFrustum(false);
			gameObject->GetTransform()->SetLocalPosition(Vec3(1200.f * (i + 1), -100.f, -900.f));
			gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
			gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
			gameObject->AddComponent(boxCollider);
			gameObject->AddComponent(make_shared<BruserEnemy>());
			//gameObject->GetMeshRenderer()->GetMaterial()->SetInt(0, 1);
			AddGameObject(gameObject);
		}
	}
}
