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
		obj->GetTransform()->SetLocalScale(Vec3(700.f, 400.f, 1.f));
		obj->GetTransform()->SetLocalPosition(Vec3(0, 0, 1.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
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
		obj->GetTransform()->SetLocalScale(Vec3(200.f, 200.f, 1.f));
		obj->GetTransform()->SetLocalPosition(Vec3(0, -100, 1.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"ShopButton", L"..\\Resources\\Texture\\ShopItemBackground.png");;

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
		MakeMerchandise(i,(PLAYER_WEAPON)i);
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
						if(object->GetButton()->GetType() == BUTTON_TYPE::SHOP)
							_selected = object;
						if (object->GetButton()->GetType() == BUTTON_TYPE::PURCHASE)
							_purchase = true;
						//ShowSelectedMerchandise();
					}
				}
			}
		}
	}
}

void Shop::MakeMerchandise(int count, PLAYER_WEAPON weapon)
{
#pragma region Shop Button
		shared_ptr<GameObject> ButtonBG = make_shared<GameObject>();
		ButtonBG->SetName(L"Button");
		ButtonBG->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		ButtonBG->AddComponent(make_shared<Transform>());
		ButtonBG->GetTransform()->SetLocalScale(Vec3(200.f, 50.f, 10.f));
		ButtonBG->GetTransform()->SetLocalPosition(Vec3(200.f, 150.f - 60.f * count, 500.f));
		shared_ptr<MeshRenderer> itemMeshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			itemMeshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"ShopButton", L"..\\Resources\\Texture\\ShopItemBackground.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			itemMeshRenderer->SetMaterial(material);
		}
		shared_ptr<Button> button = make_shared<Button>();
		button->SetType(BUTTON_TYPE::SHOP);
		button->SetMerchandise(weapon);
		ButtonBG->AddComponent(button);
		ButtonBG->AddComponent(itemMeshRenderer);
		_shopObject.push_back(ButtonBG);
#pragma endregion

#pragma region Shop Item Image
		shared_ptr<GameObject> ItemBG = make_shared<GameObject>();
		ItemBG->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		ItemBG->AddComponent(make_shared<Transform>());
		ItemBG->GetTransform()->SetLocalScale(Vec3(40.f, 40.f, 10.f));
		ItemBG->GetTransform()->SetLocalPosition(Vec3(125.f, 150.f - 60.f * count, 500.f));
		shared_ptr<MeshRenderer> buttonMeshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();

			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"ShopItem", L"..\\Resources\\Texture\\ShopButtonBackground.png");;

			shared_ptr<Material> material = make_shared<Material>();
			buttonMeshRenderer->SetMesh(mesh);
			material->SetShader(shader);	
			material->SetTexture(0, texture);
			buttonMeshRenderer->SetMaterial(material);
		}
		ItemBG->AddComponent(buttonMeshRenderer);
		_shopObject.push_back(ItemBG);
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
		selectedItem->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 10.f));
		selectedItem->GetTransform()->SetLocalPosition(Vec3(0.f, 150.f, 500.f));
		shared_ptr<MeshRenderer> meshRenderer = _selected->GetMeshRenderer();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();

			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"ShopButtonBackGround", L"..\\Resources\\Texture\\ShopButtonBackground.png");;

			shared_ptr<Material> material = make_shared<Material>();
			meshRenderer->SetMesh(mesh);
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		selectedItem->AddComponent(meshRenderer);
		_shopObject.push_back(selectedItem);
	}
}