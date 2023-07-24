#include "pch.h"
#include "Engine.h"
#include "Shop.h"
#include "Input.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "Transform.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Resources.h"
#include "Shader.h"
#include "Button.h"


Shop::Shop() : Component(COMPONENT_TYPE::SHOP)
{
#pragma region Shop Background
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(GEngine->GetWindow().width, GEngine->GetWindow().height, 1.f));
		obj->GetTransform()->SetLocalPosition(Vec3(0, 0, 500.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);

			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"ShopBackGround", L"..\\Resources\\Texture\\ShopBackground.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		obj->AddComponent(meshRenderer);
		_shopObject.push_back(obj);
	}
#pragma endregion

#pragma region Purchase Button
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetName(L"Button");
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(300.f, 100.f, 1.f));
		obj->GetTransform()->SetLocalPosition(Vec3(0, -300, 5.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);

			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"PurchaseButton", L"..\\Resources\\Texture\\PurchaseButton.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		shared_ptr<Button> button = make_shared<Button>();
		button->SetType(BUTTON_TYPE::PURCHASE);
		obj->AddComponent(button);
		obj->AddComponent(meshRenderer);
		_shopObject.push_back(obj);
	}
#pragma endregion

	for (int i = 0; i < _shopMerchandise; i++)
	{
		MakeMerchandise(i, (PLAYER_WEAPON)i);
	}
}

Shop::~Shop()
{

}

void Shop::Update()
{
	if (INPUT->GetButtonDown(KEY_TYPE::O))
	{
		if (_shopState == false)
			_shopState = true;
		else
			_shopState = false;
	}
	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
	{
		if (_shopState == true)
		{
			_mousePos = GET_SINGLE(Input)->GetMousePos();
			for (auto& object : _shopObject)
			{
				if (object->GetName() == L"Button")
				{
					if (object->GetButton()->CheckPress(_mousePos) == true)
					{
						if (object->GetButton()->GetType() == BUTTON_TYPE::SHOP)
							_selected = object;
						if (object->GetButton()->GetType() == BUTTON_TYPE::PURCHASE && _selected != NULL)
							_purchase = true;
						ShowSelectedMerchandise();
					}
				}
			}
		}
	}
	if (INPUT->GetButtonDown(KEY_TYPE::ESC))
	{
		if (_shopState == true)
		{
			_shopState == false;
		}
	}
}

void Shop::MakeMerchandise(int count, PLAYER_WEAPON weapon)
{
#pragma region Shop Button
	{
		shared_ptr<GameObject> ButtonBG = make_shared<GameObject>();
		ButtonBG->SetName(L"Button");
		ButtonBG->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		ButtonBG->AddComponent(make_shared<Transform>());
		ButtonBG->GetTransform()->SetLocalScale(Vec3(300.f, 75.f, 10.f));
		ButtonBG->GetTransform()->SetLocalPosition(Vec3((GEngine->GetWindow().width / 2) - 300, 150.f - 110.f * count, 500.f));
		shared_ptr<MeshRenderer> itemMeshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			itemMeshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"ShopButton", L"..\\Resources\\Texture\\ShopItemBackground.png");

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			itemMeshRenderer->SetMaterial(material);
		}

		shared_ptr<Button> button = make_shared<Button>();
		button->SetType(BUTTON_TYPE::SHOP);
		button->SetMerchandise(weapon);
		button->SetMerchandiseType(MERCHANDISE_TYPE::GUN);
		ButtonBG->AddComponent(button);
		ButtonBG->AddComponent(itemMeshRenderer);
		_shopObject.push_back(ButtonBG);
	}
#pragma endregion

#pragma region Shop Item Image
	{
		shared_ptr<GameObject> ItemBG = make_shared<GameObject>();
		ItemBG->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		ItemBG->AddComponent(make_shared<Transform>());
		ItemBG->GetTransform()->SetLocalScale(Vec3(300.f, 60.f, 10.f));
		ItemBG->GetTransform()->SetLocalPosition(Vec3((GEngine->GetWindow().width / 2) - 300, 150.f - 110.f * count, 500.f));
		shared_ptr<MeshRenderer> buttonMeshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();

			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			wstring a = L"..\\Resources\\Texture\\ShopButtonBackground";
			wstring b = to_wstring(count);
			wstring c = L".png";
			wstring d = L"ShopItem";

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(d + b, a + b + c);;

			shared_ptr<Material> material = make_shared<Material>();
			buttonMeshRenderer->SetMesh(mesh);
			material->SetShader(shader);
			material->SetTexture(0, texture);
			buttonMeshRenderer->SetMaterial(material);
		}
		ItemBG->AddComponent(buttonMeshRenderer);
		_shopObject.push_back(ItemBG);

	}
#pragma endregion

#pragma region Shop Magazine Button		//배경 버튼
	{
		shared_ptr<GameObject> ButtonBG = make_shared<GameObject>();
		ButtonBG->SetName(L"Button");
		ButtonBG->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		ButtonBG->AddComponent(make_shared<Transform>());
		ButtonBG->GetTransform()->SetLocalScale(Vec3(75.f, 75.f, 10.f));
		ButtonBG->GetTransform()->SetLocalPosition(Vec3((GEngine->GetWindow().width / 2) - 500, 150.f - 110.f * count, 500.f));
		shared_ptr<MeshRenderer> itemMeshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			itemMeshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"ShopButton", L"..\\Resources\\Texture\\ShopItemBackground.png");

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			itemMeshRenderer->SetMaterial(material);
		}

		shared_ptr<Button> button = make_shared<Button>();
		button->SetType(BUTTON_TYPE::SHOP);
		button->SetMerchandise(weapon);
		button->SetMerchandiseType(MERCHANDISE_TYPE::BULLET);
		ButtonBG->AddComponent(button);
		ButtonBG->AddComponent(itemMeshRenderer);
		_shopObject.push_back(ButtonBG);
	}
#pragma endregion

#pragma region Shop Magazine Image		//상품
	{
		shared_ptr<GameObject> ItemBG = make_shared<GameObject>();
		ItemBG->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		ItemBG->AddComponent(make_shared<Transform>());
		ItemBG->GetTransform()->SetLocalScale(Vec3(40.f, 60.f, 10.f));
		ItemBG->GetTransform()->SetLocalPosition(Vec3((GEngine->GetWindow().width / 2) - 500, 150.f - 110.f * count, 500.f));
		shared_ptr<MeshRenderer> buttonMeshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();

			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Magazine", L"..\\Resources\\Texture\\Magazine.png");

			shared_ptr<Material> material = make_shared<Material>();
			buttonMeshRenderer->SetMesh(mesh);
			material->SetShader(shader);
			material->SetTexture(0, texture);
			buttonMeshRenderer->SetMaterial(material);
		}
		ItemBG->AddComponent(buttonMeshRenderer);
		_shopObject.push_back(ItemBG);
	}
#pragma endregion
}

void Shop::ShowSelectedMerchandise()
{
	if (_selected != NULL)
	{
		shared_ptr<GameObject> selectedItem = make_shared<GameObject>();
		selectedItem->SetName(L"SelectedItem");
		selectedItem->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		selectedItem->AddComponent(make_shared<Transform>());
		selectedItem->GetTransform()->SetLocalScale(Vec3(400.f, 400.f, 10.f));
		selectedItem->GetTransform()->SetLocalPosition(Vec3(-400.f, 0.f, 1.f));
		
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);

			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture1 = GET_SINGLE(Resources)->Load<Texture>(L"Pistl Price", L"..\\Resources\\Texture\\Pistol Price.png");;
			shared_ptr<Texture> texture2 = GET_SINGLE(Resources)->Load<Texture>(L"Rifle Price", L"..\\Resources\\Texture\\Rifle Price.png");;
			shared_ptr<Texture> texture3 = GET_SINGLE(Resources)->Load<Texture>(L"Shotgun Price", L"..\\Resources\\Texture\\Shotgun Price.png");;
			shared_ptr<Texture> texture4 = GET_SINGLE(Resources)->Load<Texture>(L"Sniper Price", L"..\\Resources\\Texture\\Sniper Price.png");;

			shared_ptr<Material> material = make_shared<Material>();

			material->SetShader(shader);
			if (_selected.get()->GetButton()->GetMerchandise() == PLAYER_WEAPON::PISTOL)
			{
				material->SetTexture(0, texture1);
				_selectedNum = 0;
			}
			if (_selected.get()->GetButton()->GetMerchandise() == PLAYER_WEAPON::SMG)
			{
				material->SetTexture(0, texture2);
				_selectedNum = 1;
			}
			if (_selected.get()->GetButton()->GetMerchandise() == PLAYER_WEAPON::SHOTGUN)
			{
				material->SetTexture(0, texture3);
				_selectedNum = 2;
			}
			if (_selected.get()->GetButton()->GetMerchandise() == PLAYER_WEAPON::SNIPER)
			{
				material->SetTexture(0, texture4);
				_selectedNum = 3;
			}
			meshRenderer->SetMaterial(material);
		}
		selectedItem->AddComponent(meshRenderer);
		_selectedObject = selectedItem;
	}
}