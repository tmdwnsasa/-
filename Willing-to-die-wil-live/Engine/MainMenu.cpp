#include "pch.h"
#include "Engine.h"
#include "MainMenu.h"
#include "Input.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "Transform.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Resources.h"
#include "Shader.h"
#include "Font.h"


MainMenu::MainMenu() : Component(COMPONENT_TYPE::MAINMENU)
{
#pragma region MainMenu Background
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

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"MainMenuBackGround", L"..\\Resources\\Texture\\MainMenuBackground.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		obj->AddComponent(meshRenderer);
		_menuObject.push_back(obj);
	}
#pragma endregion

#pragma region Text_Stamina
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetName(L"StartText");
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(12.f, 12.f, 12.f));
		obj->GetTransform()->SetLocalPosition(Vec3(0.f - 300.f, 0.f - 150.f, 10.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

		shared_ptr<Font> font = make_shared<Font>();
		font->BuildFont();
		shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadFontMesh(font->GetTextVB("Press enter to start"));
		meshRenderer->SetMesh(mesh);

		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Font");

			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"deathspirit", L"..\\Resources\\Font\\DeathSpirit.png");;

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
			obj->AddComponent(meshRenderer);
			obj->AddComponent(font);
			_menuObject.push_back(obj);
		}
	}
#pragma endregion
}

MainMenu::~MainMenu()
{

}

void MainMenu::Update()
{
	if (INPUT->GetButtonDown(KEY_TYPE::ENTER))
	{
		if (_menuState == true)
			_menuState = false;
	}
}