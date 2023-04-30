#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Engine.h"
#include "Material.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Camera.h"
#include "Light.h"
#include "Input.h"

#include "CameraScript.h"
#include "Resources.h"
#include "ParticleSystem.h"
#include "Terrain.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "MeshData.h"
#include "TestDragon.h"

#include "Player.h"
#include "Font.h"
#include "Enemy.h"
#include "Shop.h"
#include "Gun.h"

#include <iostream>

//cout 출력용 코드
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

void SceneManager::Update()
{
	if (_activeScene == nullptr)
		return;

	_activeScene->Update();
	_activeScene->LateUpdate();
	_activeScene->FinalUpdate();
}

// TEMP
void SceneManager::Render()
{
	if (_activeScene)
		_activeScene->Render();
}

void SceneManager::LoadScene(wstring sceneName)
{
	// TODO : 기존 Scene 정리
	// TODO : 파일에서 Scene 정보 로드

	_activeScene = LoadTestScene();

	_activeScene->Awake();
	_activeScene->Start();
}

void SceneManager::SetLayerName(uint8 index, const wstring& name)
{
	// 기존 데이터 삭제
	const wstring& prevName = _layerNames[index];
	_layerIndex.erase(prevName);

	_layerNames[index] = name;
	_layerIndex[name] = index;
}

uint8 SceneManager::LayerNameToIndex(const wstring& name)
{
	auto findIt = _layerIndex.find(name);
	if (findIt == _layerIndex.end())
		return 0;

	return findIt->second;
}

shared_ptr<GameObject> SceneManager::Pick(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GetActiveScene()->GetMainCamera();

	float width = static_cast<float>(GEngine->GetWindow().width);
	float height = static_cast<float>(GEngine->GetWindow().height);

	Matrix projectionMatrix = camera->GetProjectionMatrix();

	// ViewSpace에서 Picking 진행
	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
	float viewY = (-2.0f * screenY / height + 1.0f) / projectionMatrix(1, 1);

	Matrix viewMatrix = camera->GetViewMatrix();
	Matrix viewMatrixInv = viewMatrix.Invert();

	auto& gameObjects = GET_SINGLE(SceneManager)->GetActiveScene()->GetGameObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> picked;

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetBoxCollider() == nullptr)
			continue;

		if(gameObject->GetName() != L"Wall")
			continue;

		// ViewSpace에서의 Ray 정의
		Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);

		// WorldSpace에서의 Ray 정의
		rayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
		rayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
		rayDir.Normalize();

		// WorldSpace에서 연산
		float distance = 0.f;
		if (gameObject->GetBoxCollider()->Intersects(rayOrigin, rayDir, OUT distance) == false)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}

	return picked;
}

shared_ptr<Scene> SceneManager::LoadTestScene()
{
#pragma region LayerMask
	SetLayerName(0, L"Default");
	SetLayerName(1, L"UI");
#pragma endregion

#pragma region ComputeShader
	{
		shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"ComputeShader");

		// UAV 용 Texture 생성
		shared_ptr<Texture> texture = GET_SINGLE(Resources)->CreateTexture(L"UAVTexture",
			DXGI_FORMAT_R8G8B8A8_UNORM, 1024, 1024,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"ComputeShader");
		material->SetShader(shader);
		material->SetInt(0, 1);
		GEngine->GetComputeDescHeap()->SetUAV(texture->GetUAVHandle(), UAV_REGISTER::u0);

		// 쓰레드 그룹 (1 * 1024 * 1)
		material->Dispatch(1, 1024, 1);
	}
#pragma endregion

	shared_ptr<Scene> scene = make_shared<Scene>();

#pragma region Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->SetName(L"Main_Camera");
		camera->AddComponent(make_shared<Transform>());
		camera->AddComponent(make_shared<Camera>()); // Near=1, Far=1000, FOV=45도
		camera->AddComponent(make_shared<CameraScript>());
		camera->GetCamera()->SetFar(10000.f);
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		uint8 layerIndex = GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, true); // UI는 안 찍음
		scene->AddGameObject(camera);
	}
#pragma endregion

#pragma region UI_Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->SetName(L"Orthographic_Camera");
		camera->AddComponent(make_shared<Transform>());
		camera->AddComponent(make_shared<Camera>()); // Near=1, Far=1000, 800*600
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		camera->GetCamera()->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC);
		uint8 layerIndex = GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskAll(); // 다 끄고
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, false); // UI만 찍음
		scene->AddGameObject(camera);
	}
#pragma endregion

#pragma region SkyBox
	{
		shared_ptr<GameObject> skybox = make_shared<GameObject>();
		skybox->AddComponent(make_shared<Transform>());
		skybox->SetCheckFrustum(false);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
			meshRenderer->SetMesh(sphereMesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Skybox");
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Sky01", L"..\\Resources\\Texture\\Sky01.jpg");
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		skybox->AddComponent(meshRenderer);
		scene->AddGameObject(skybox);
	}
#pragma endregion

#pragma region Object
		//{
		//	shared_ptr<GameObject> obj = make_shared<GameObject>();
		//	obj->SetName(L"OBJ");
		//	obj->AddComponent(make_shared<Transform>());
		//	obj->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		//	obj->GetTransform()->SetLocalPosition(Vec3(100.f, 100.f, 500.f));
		//	obj->SetCheckFrustum(false);

		//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		//	{
		//		shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
		//		meshRenderer->SetMesh(sphereMesh);

		//		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject");
		//		material->SetInt(0, 1);
		//		meshRenderer->SetMaterial(material);
		//	}

		//	shared_ptr<SphereCollider> spherecollider = make_shared<SphereCollider>();
		//	obj->SetStatic(false);

		//	spherecollider->SetRadius(0.5f);
		//	spherecollider->SetCenter(Vec3(0.f, 0.f, 0.f));

		//	obj->AddComponent(spherecollider);
		//	obj->AddComponent(meshRenderer);
		//	scene->AddGameObject(obj);
		//}
#pragma endregion

#pragma region Terrain
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->AddComponent(make_shared<Transform>());
		obj->AddComponent(make_shared<Terrain>());
		obj->AddComponent(make_shared<MeshRenderer>());

		obj->GetTransform()->SetLocalScale(Vec3(50.f, 250.f, 50.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-2000.f, -100.f, -6000.f));
		obj->SetStatic(true);
		obj->GetTerrain()->Init(256, 256);
		obj->SetCheckFrustum(false);

		scene->AddGameObject(obj);
	}
#pragma endregion

#pragma region UI_Test
	for (int32 i = 0; i < 6; i++)
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-350.f + (i * 120), 500.f, 500.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture;
			if (i < 3)
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->GetRTTexture(i);
			else if (i < 5)
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->GetRTTexture(i - 3);
			else
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->GetRTTexture(0);

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		obj->AddComponent(meshRenderer);
		scene->AddGameObject(obj);
	}
#pragma endregion

#pragma region Text_HP
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetName(L"HealthText");
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(10.f, 10.f, 1.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-300.f, -200.f, 900.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

		shared_ptr<Font> font = make_shared<Font>();
		font->BuildFont();
		shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadFontMesh(font->GetTextVB("100"));
		mesh = GET_SINGLE(Resources)->LoadFontMesh(font->GetTextVB("90"));
		meshRenderer->SetMesh(mesh);

		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Font");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"original", L"..\\Resources\\Font\\text.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
			obj->AddComponent(meshRenderer);
			obj->AddComponent(font);
			scene->AddGameObject(obj);
		}
	}
#pragma endregion

#pragma region Text_Bullet
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetName(L"BulletText");
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(10.f, 10.f, 10.f));
		obj->GetTransform()->SetLocalPosition(Vec3(300.f, -200.f, 500.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

		shared_ptr<Font> font = make_shared<Font>();
		font->BuildFont();
		shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadFontMesh(font->GetTextVB("100"));
		meshRenderer->SetMesh(mesh);

		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Font");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"original", L"..\\Resources\\Font\\text.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
			obj->AddComponent(meshRenderer);
			obj->AddComponent(font);
			scene->AddGameObject(obj);
		}
	}
#pragma endregion

#pragma region Text_Money
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetName(L"MoneyText");
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(10.f, 10.f, 10.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-350.f, 250.f, 500.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

		shared_ptr<Font> font = make_shared<Font>();
		font->BuildFont();
		shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadFontMesh(font->GetTextVB("100000"));
		meshRenderer->SetMesh(mesh);

		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Font");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"original", L"..\\Resources\\Font\\text.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
			obj->AddComponent(meshRenderer);
			obj->AddComponent(font);
			scene->AddGameObject(obj);
		}
	}
#pragma endregion

#pragma region Directional Light
	{
		shared_ptr<GameObject> light = make_shared<GameObject>();
		light->AddComponent(make_shared<Transform>());
		light->GetTransform()->SetLocalPosition(Vec3(0, 1000, 500));
		light->AddComponent(make_shared<Light>());
		light->GetLight()->SetLightDirection(Vec3(0, -1, 1.f));
		light->GetLight()->SetLightType(LIGHT_TYPE::DIRECTIONAL_LIGHT);
		light->GetLight()->SetDiffuse(Vec3(1.f, 1.f, 1.f));
		light->GetLight()->SetAmbient(Vec3(0.1f, 0.1f, 0.1f));
		light->GetLight()->SetSpecular(Vec3(0.1f, 0.1f, 0.1f));

		scene->AddGameObject(light);
	}
#pragma endregion


#pragma region Player
	{
		shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\batman.fbx");

		vector<shared_ptr<GameObject>> gameObjects = meshData->Instantiate();

		for (auto& gameObject : gameObjects)
		{
			gameObject->SetName(L"Player");
			gameObject->SetCheckFrustum(false);
			gameObject->GetTransform()->SetLocalPosition(Vec3(2700.f, 0.f, -600.f));
			gameObject->GetTransform()->SetLocalScale(Vec3(0.1f, 0.1f, 0.1f));
			gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
			gameObject->AddComponent(make_shared<Player>());
			scene->AddGameObject(gameObject);
		}
	}
#pragma endregion

#pragma region Enemy
	{
		shared_ptr<MeshData> ZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Craw.fbx");

		for (int i = 0; i < 1; i++)
		{

			vector<shared_ptr<GameObject>> gameObjects = ZombieMesh->Instantiate();

			for (auto& gameObject : gameObjects)
			{

				shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
				boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
				boxCollider->SetExtents(Vec3(10.f, 40.f, 10.f));

				gameObject->SetName(L"Enemy");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(Vec3(1200.f, 100.f, -900.f));
				gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
				gameObject->AddComponent(boxCollider);
				gameObject->AddComponent(make_shared<Enemy>());
				gameObject->GetMeshRenderer()->GetMaterial()->SetInt(0, 1);
				scene->AddGameObject(gameObject);
			}
		}

		//vector<shared_ptr<GameObject>> gameObjects = ZombieMesh->Instantiate();
		//for (auto& gameObject : gameObjects)
		//{
		//	//shared_ptr<Enemy> EnemyObj = make_shared<Enemy>();
		//	gameObject->SetName(L"Enemy");
		//	gameObject->SetCheckFrustum(false);
		//	gameObject->GetTransform()->SetLocalPosition(Vec3(2000.f, 300.f, 0.f));
		//	gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.01f, 1.01f));
		//	gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
		//	//gameObject->AddComponent(EnemyObj);


		//	shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		//	boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
		//	boxCollider->SetExtents(Vec3(10.f, 40.f, 10.f));
		//	gameObject->AddComponent(boxCollider);
		//	gameObject->GetMeshRenderer()->GetMaterial()->SetInt(0, 1);
		//	gameObject->AddComponent(make_shared<Enemy>());
		//	scene->AddGameObject(gameObject);
		//}


		
		
	}
	

#pragma endregion


#pragma region MapDesign

	{
		// 벽 기본스케일 300,300,10
		//////////////////////////////////////////////////////////////////////24.12 ~ 24.19
		for (int i = 0; i < 8; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (3.5 - i), 0.f, baseZ));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);

		}
		//////////////////////////////////////////////////////////////////////24.12 23.12 ,//24.19 23.19
		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3((baseX - scale * 4), 0.f, baseZ - scale * (0.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3((baseX + scale * 4), 0.f, baseZ - scale * (0.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////////////23.7,23.11 / 23.20 , 23.24
		for (int i = 0; i < 5; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(((baseX - scale * (4.5 + i))), 0.f, baseZ-scale*2));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 5; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(((baseX + scale * (4.5 + i))), 0.f, baseZ - scale * 2));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		for (int i = 0; i < 2; i++)
		{
			//////////////////////////////////////////////////////////////////////23.7 22.6
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 9, 0.f, baseZ - scale * (2.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////////////////////////////21.2 ~ 21.6
		for (int i = 0; i < 5; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (9.5 + i), 0.f, baseZ - scale * 4));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		for (int i = 0; i < 2; i++)
		{
			//////////////////////////////////////////////////////////////////////21.2 20.2
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 14, 0.f, (baseZ - scale * 4.5) - scale * 3 * i));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////////////////////////////20.2 ~ 20.3
		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 13.5, 0.f, (baseZ - scale * (7 - 2*i)))); //z -600
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 2; i++)
		{
			//19.3 18.3, 18.9 19.9
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3((baseX + scale * 13), 0.f, baseZ - scale * (5.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 2; i++)
		{
			//19.3 18.3, 18.9 19.9
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3((baseX + scale * 7), 0.f, baseZ - scale * (5.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 3; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (11.5 + i), 0.f, baseZ - scale * 8));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 10; i++)
		{
			//20.2 18.2
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 11, 0.f, baseZ - scale * (8.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (11.5 + i), 0.f, baseZ - scale * 18));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//????????????????????????
		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 5; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 13, 0.f, baseZ - scale * (18.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 6; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (7.5 + i), 0.f, baseZ - scale * 23));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 7, 0.f, (baseZ - scale * (21.5 + i))));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 7, 0.f, (baseZ - scale * (18.5 + i))));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 3; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (7.5 + i), 0.f, baseZ - scale * 18));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 4; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 10, 0.f, baseZ - scale * (14.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 7; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (3.5 + i), 0.f, (baseZ - scale * 14)));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 7; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (3.5 + i), 0.f, (baseZ - scale * 13)));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 5; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 10, 0.f, baseZ - scale * (8.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////////////////////
		for (int i = 0; i < 4; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (6.5 + i), 0.f, baseZ - scale * 8));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 2; i++)
		{
			/////////////////////////////////////////
			////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 6, 0.f, (baseZ - scale * 4.5) - ((baseZ - scale * 3) * i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		//////////////////////////////////////////////17.9 20.9
		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 6.5, 0.f, (baseZ - scale * (7 - 2 * i))));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////20.10,20.9
		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (6.5 + i), 0.f, baseZ - scale * 4));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
////////////////// 수정 가능성 
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 8, 0.f, baseZ - scale * 3.5));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////
		for (int i = 0; i < 8; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (0.5 + i), 0.f, baseZ - scale * 3));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		//////////////////////////////////////////////
		for (int i = 0; i < 7; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * (1.5 + i), 0.f, baseZ - scale * 3));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
		////////// 문제 
		for (int i = 0; i < 3; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX, 0.f, baseZ - scale * (3.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 3; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale, 0.f, baseZ - scale * (3.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 3; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (0.5 + i), 0.f, baseZ - scale * 6));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 3; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * (1.5 + i), 0.f, baseZ - scale * 6));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 7; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 3, 0.f, baseZ - scale * (6.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 4; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * 4, 0.f, baseZ - scale * (6.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 6; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * (-1.5 + i), 0.f, baseZ - scale * 10));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 10; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 2, 0.f, baseZ - scale * (10.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 6; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * 3, 0.f, baseZ - scale * (14.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 7; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (0.5 + i), 0.f, baseZ - scale * 21));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 4; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (3.5 + i), 0.f, baseZ - scale * 20));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 2; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + scale * (0.5 + i), 0.f, baseZ - scale * 20));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 2; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX, 0.f, baseZ - scale * (21.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}


		for (int i = 0; i < 2; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX, 0.f, baseZ - scale * (18.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 5; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * (0.5 + i), 0.f, baseZ - scale * 23));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 5; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * (0.5 + i), 0.f, baseZ - scale * 18));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * 5.5, 0.f, baseZ - scale * 21));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 2; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * 5, 0.f, baseZ - scale * (21.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 9; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * 5, 0.f, baseZ - scale * (9.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 11; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 1.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * 6, 0.f, baseZ - scale * (10.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 3; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * (5.5 + i), 0.f, baseZ - scale * 9));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 3; i++)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * (6.5 + i), 0.f, baseZ - scale * 10));
			obj->SetCheckFrustum(false);
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 6; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * 8, 0.f, baseZ - scale * (3.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}

		for (int i = 0; i < 8; i++)
		{
			//////////////////////////////////////////////////////////////////////
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->AddComponent(make_shared<Transform>());
			obj->SetName(L"Wall");
			obj->SetCheckFrustum(false);
			obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 10.f));
			obj->GetTransform()->SetLocalPosition(Vec3(baseX - scale * 9, 0.f, baseZ - scale * (2.5 + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
			shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

			{
				shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
				meshRenderer->SetMesh(mesh);
			}
			{
				shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Wall");
				material->SetInt(0, 1);
				meshRenderer->SetMaterial(material);
			}
			shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
			boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
			boxCollider->SetExtents(Vec3(1.f, 1.f, 1.f));
			obj->AddComponent(boxCollider);

			obj->AddComponent(meshRenderer);
			scene->AddGameObject(obj);
		}
	}
#pragma endregion

#pragma region Shop
	{
		shared_ptr<GameObject> ShopUI = make_shared<GameObject>();
		ShopUI->SetName(L"Shop");
		ShopUI->AddComponent(make_shared<Transform>());
		ShopUI->AddComponent(make_shared<Shop>());
		ShopUI->GetTransform()->SetLocalPosition(Vec3(0, 1000, 500));
		scene->AddGameObject(ShopUI);
	}
#pragma endregion
	return scene;
}

