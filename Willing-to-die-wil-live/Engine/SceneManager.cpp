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
#include "FBXLoader.h"

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
	//shared_ptr<MeshData> ZombieMesh2 = make_shared<MeshData>();
	//ZombieMesh2->Save(L"..\\Resources\\FBX\\tes.fbx");
	shared_ptr<MeshData> ZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\tes.bin");
	shared_ptr<MeshData> TZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\test.bin");
	shared_ptr<MeshData> TTZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\testt.bin");
	
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

#pragma region Terrain
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetName(L"Terrain");
		obj->AddComponent(make_shared<Transform>());
		obj->AddComponent(make_shared<Terrain>());
		obj->AddComponent(make_shared<MeshRenderer>());

		obj->GetTransform()->SetLocalScale(Vec3(50.f, 250.f, 50.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-4000.f, -100.f, -8000.f));
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
		obj->GetTransform()->SetLocalPosition(Vec3(-350.f + (i * 110), 550.f, 500.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture2");

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
		//scene->AddGameObject(obj);
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

//#pragma region Object
//	{
//		shared_ptr<GameObject> obj = make_shared<GameObject>();
//		obj->SetName(L"OBJ");
//		obj->AddComponent(make_shared<Transform>());
//		obj->GetTransform()->SetLocalScale(Vec3(200.f, 200.f, 200.f));
//		obj->GetTransform()->SetLocalPosition(Vec3(5000.f, 100.f, -1500.f));
//		obj->SetCheckFrustum(false);
//		obj->SetStatic(false);
//
//		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
//		{
//			shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
//			meshRenderer->SetMesh(sphereMesh);
//
//			shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject");
//			meshRenderer->SetMaterial(material->Clone());
//		}
//		obj->AddComponent(meshRenderer);
//		scene->AddGameObject(obj);
//	}
//#pragma endregion

#pragma region Directional Light
	{
		shared_ptr<GameObject> light = make_shared<GameObject>();
		light->SetName(L"Light1");
		light->AddComponent(make_shared<Transform>());
		light->GetTransform()->SetLocalPosition(Vec3(5000.f, 1200, -1500.f));
		light->AddComponent(make_shared<Light>());
		light->GetLight()->SetLightDirection(Vec3(0.3f, -1.0f, 0.3f));
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
			gameObject->GetTransform()->SetLocalPosition(Vec3(3700.f, 0.f, -3600.f));
			gameObject->GetTransform()->SetLocalScale(Vec3(0.1f, 0.1f, 0.1f));
			gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
			gameObject->AddComponent(make_shared<Player>());
			scene->AddGameObject(gameObject);
		}
	}
#pragma endregion

#pragma region Enemy
	{
<<<<<<< HEAD

		
=======
		//shared_ptr<MeshData> ZombieMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\tes.fbx");

		vector<shared_ptr<GameObject>> gameObjects1 = ZombieMesh->Instantiate();
		vector<shared_ptr<GameObject>> gameObjects2 = TZombieMesh->Instantiate();
		vector<shared_ptr<GameObject>> gameObjects3 = TTZombieMesh->Instantiate();

		for (int i = 0; i < 1; i++)
		{
			for (auto& gameObject : gameObjects1)
			{

				shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
				boxCollider->SetCenter(Vec3(0.f, 150.f, 0.f));
				boxCollider->SetExtents(Vec3(200.f, 300.f, 200.f));

				gameObject->SetName(L"Enemy");
				gameObject->SetCheckFrustum(false);
				gameObject->SetStatic(false);
				gameObject->GetTransform()->SetLocalPosition(Vec3(3500.f + (i * 100), -100.f, -900.f));
				gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
				gameObject->AddComponent(boxCollider);
				gameObject->AddComponent(make_shared<Enemy>());
				gameObject->GetMeshRenderer()->GetMaterial()->SetInt(1, 1);
				scene->AddGameObject(gameObject);
			}
		}

		
		for (int i = 0; i < 1; i++)
		{
			for (auto& gameObject : gameObjects3)
			{

				shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
				boxCollider->SetCenter(Vec3(0.f, 150.f, 0.f));
				boxCollider->SetExtents(Vec3(200.f, 300.f, 200.f));

				gameObject->SetName(L"Enemy"); 
				gameObject->SetCheckFrustum(false);
				gameObject->SetStatic(false);
				gameObject->GetTransform()->SetLocalPosition(Vec3(6250.f * (i + 1), -100.f, -6450.f));
				gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
				gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
				gameObject->AddComponent(boxCollider);
				gameObject->AddComponent(make_shared<Enemy>());
				scene->AddGameObject(gameObject);
			}
		}
>>>>>>> 36043eb59c0512caba7c30a8e4518f5d80178b63

		/*shared_ptr<MeshData> TestMesh = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\sniper.fbx");

		for (int i = 0; i < 1; i++)
		{

			vector<shared_ptr<GameObject>> gameObjects = TestMesh->Instantiate();

			for (auto& gameObject : gameObjects)
			{

				shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
				boxCollider->SetCenter(Vec3(0.f, 0.f, 0.f));
				boxCollider->SetExtents(Vec3(10.f, 40.f, 10.f));

				gameObject->SetName(L"Enemy");
				gameObject->SetCheckFrustum(false);
				gameObject->GetTransform()->SetLocalPosition(Vec3(1200.f * (i + 1), 1000.f, -900.f));
				gameObject->GetTransform()->SetLocalScale(Vec3(10.f, 10.f, 10.f));
				gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
				gameObject->AddComponent(boxCollider);
				gameObject->AddComponent(make_shared<Enemy>());
				gameObject->GetMeshRenderer()->GetMaterial()->SetInt(0, 1);
				scene->AddGameObject(gameObject);
			}
		}*/

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

#pragma region MapDesign

	{
		MakeWall(false, 2, -4, -2, scene);
		MakeWall(false, 2, 4, -2, scene);
		MakeWall(false, 2, 9, -4, scene);
		MakeWall(false, 1, 14, -8, scene);
		MakeWall(false, 1, 14, -5, scene);
		MakeWall(false, 2, 13, -7, scene);
		MakeWall(false, 2, 7, -7, scene);
		MakeWall(false, 10, 11, -18, scene);
		MakeWall(false, 5, 13, -23, scene);
		MakeWall(false, 2, 7, -23, scene);
		MakeWall(false, 2, 7, -20, scene);
		MakeWall(false, 4, 10, -18, scene);
		MakeWall(false, 5, 10, -13, scene);
		MakeWall(false, 1, 6, -5, scene);
		MakeWall(false, 1, 6, -8, scene);
		MakeWall(false, 1, 8, -4, scene);
		MakeWall(false, 3, 0, -6, scene);
		MakeWall(false, 3, -1, -6, scene);
		MakeWall(false, 7, 3, -13, scene);
		MakeWall(false, 4, -4, -10, scene);
		MakeWall(false, 10, 2, -20, scene);
		MakeWall(false, 2, 0, -23, scene);
		MakeWall(false, 2, 0, -20, scene);
		MakeWall(false, 2, -5, -23, scene);
		MakeWall(false, 9, -5, -18, scene);
		MakeWall(false, 11, -6, -21, scene);
		MakeWall(false , 6, -8, -9, scene);
		MakeWall(false, 8, -9, -10, scene);
		MakeWall(false, 6, 3, -20, scene);
		MakeWall(true, 8, -4, 0, scene);
		MakeWall(true, 5, -9, -2, scene);
		MakeWall(true, 5, 4, -2, scene);
		MakeWall(true, 5, 9, -4, scene);
		MakeWall(true, 1, 13, -7, scene);
		MakeWall(true, 1, 13, -5, scene);
		MakeWall(true, 3, 11, -8, scene);
		MakeWall(true, 2, 11, -18, scene);
		MakeWall(true, 6, 7, -23, scene);
		MakeWall(true, 3, 7, -18, scene);
		MakeWall(true, 7, 3, -14, scene);
		MakeWall(true, 7, 3, -13, scene);
		MakeWall(true, 4, 6, -8, scene);
		MakeWall(true, 1, 6, -7, scene);
		MakeWall(true, 1, 6, -5, scene);
		MakeWall(true, 2, 6, -4, scene);
		MakeWall(true, 8, 0, -3, scene);
		MakeWall(true, 7, -8, -3, scene);
		MakeWall(true, 3, 0, -6, scene);
		MakeWall(true, 3, -4, -6, scene);
		MakeWall(true, 6, -4, -10, scene);
		MakeWall(true, 2, 0, -20, scene);
		MakeWall(true, 7, 0, -21, scene);
		MakeWall(true, 4, 3, -20, scene);
		MakeWall(true, 1, -6, -21, scene);
		MakeWall(true, 5, -5, -23, scene);
		MakeWall(true, 5, -5, -18, scene);
		MakeWall(true, 3, -8, -9, scene);
		MakeWall(true, 3, -9, -10, scene);
	}
#pragma endregion

	return scene;
}

void SceneManager::MakeWall(bool horizontal, int value, float xchange, float ychange, shared_ptr<Scene>& scene)
{
	for (int i = 0; i < value; i++)
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->AddComponent(make_shared<Transform>());
		obj->SetName(L"Wall");
		obj->GetTransform()->SetLocalScale(Vec3(scale, scale, 50.f));
		if (horizontal == false)	//세로
		{
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + posScale * xchange, 0.f, baseZ + posScale * (ychange + 0.5f + i)));
			obj->GetTransform()->SetLocalRotation(Vec3(0.0f, py * 0.5, 0.0f));
		}
		else						//가로
		{
			obj->GetTransform()->SetLocalPosition(Vec3(baseX + posScale * (xchange + 0.5f + i), 0.f, baseZ + posScale * (ychange)));
		}
		obj->SetCheckFrustum(false);
		obj->SetStatic(false);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadCubeMesh();
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