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
#include "Material.h"
#include "Astar.h"
#include "TileMap.h"
#include "Player.h"
#include "Input.h"
#include "BoxCollider.h"
#include "Font.h"
#include "Shop.h"
#include "Button.h"
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
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{

		if (gameObject->GetName() == L"Gun3")
		{
			cout << gameObject->GetTransform()->GetLocalPosition().x << ", " << gameObject->GetTransform()->GetLocalPosition().y << "," << gameObject->GetTransform()->GetLocalPosition().z << endl;
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
							object->GetPlayer()->ChangeWeapon(gameObject->GetShop()->GetSelected()->GetButton()->GetMerchandise());

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
			for (const shared_ptr<GameObject>& bullet : (gameObject->GetPlayer()->getBullet()))
			{
				if (bullet->GetBullet()->GetState() == BULLET_STATE::DEAD)
				{
					_trashBin.push_back(bullet);
				}

				if (bullet->GetBullet()->GetState() == BULLET_STATE::LIVE)
				{
					_gameObjects.push_back(bullet);
					bullet->GetBullet()->SetState(BULLET_STATE::SHOOT);
				}
			}
		}
	}

	SetPlayerPosToEnemy();
	
	//복수 생성
	map<wstring, pair<uint32, vector<shared_ptr<GameObject>>>> name;
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
				if ((*object.second.second.begin())->GetMeshRenderer() != NULL)
				{
					(*object.second.second.begin())->GetMeshRenderer()->GetMaterial()->SetInt(0, 0);
				}

		}
	}

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
						//적 공격받는 함수
						_trashBin.push_back(gameObject);
					}
				}
			}
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
	Vec3 PlayerPos;
	Vec3 CameraPos;
	Vec3 CameraLook;

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
		if (gameObject->GetName() == L"Player")
		{
			PlayerPos = gameObject->GetTransform()->GetLocalPosition();
		}

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
		if (gameObject->GetName() == L"Main_Camera")
		{
			gameObject->GetTransform()->SetLocalPosition(PlayerPos);
			CameraPos = gameObject->GetTransform()->GetLocalPosition();
			CameraLook = gameObject->GetTransform()->GetLook();
		}

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
		if (gameObject->GetName() == L"Player")
		{
			gameObject->GetPlayer()->SetBulletPos(CameraPos);
			gameObject->GetPlayer()->SetBulletLook(CameraLook);
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