#include "pch.h"
#include "Transform.h"
#include "Gun.h"
#include <iostream>

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

Gun::Gun() : Component(COMPONENT_TYPE::GUN)
{

}

Gun::Gun(PLAYER_WEAPON gun) : Component(COMPONENT_TYPE::GUN)
{
	weapon = gun;
}

Gun::~Gun()
{

}

void Gun::Update()
{
	if (CameraTransform != NULL)
	{
		Vec3 Pos = CameraTransform->GetLocalPosition();
		Vec3 Right = CameraTransform->GetRight();
		Vec3 Look = CameraTransform->GetLook();
		Vec3 Up = CameraTransform->GetUp();

		Pos += Right * 30;
		Pos += Up * -130;
		Pos += Look * 100;
		GetTransform()->SetLocalPosition(Pos);
		GetTransform()->SetLocalScale(Vec3(10.f, 10.f, 10.f));
		GetTransform()->LookAt(Vec3(Look));
	}

	recoil();
}

void Gun::recoil()
{
	Vec3 basePos = GetTransform()->GetLocalPosition();
	Vec3 baseRot = GetTransform()->GetLocalRotation();
	if (_curRecoil > 0.f)
	{
		_curRecoil -= _downForce;
		_recoilAmount += _curRecoil;
		basePos += GetTransform()->GetLook() * -_curRecoil / 4;
		basePos += GetTransform()->GetUp() * _curRecoil / 8;
		GetTransform()->SetLocalRotation(Vec3(baseRot.x - (_recoilAmount / 70.f), baseRot.y, baseRot.z));

	}
	else if (_recoilAmount > 0.f)
	{
		_curRecoil2 -= _downForce;
		_recoilAmount -= _curRecoil2;
		basePos += GetTransform()->GetLook() * +_downForce / 10;
		basePos += GetTransform()->GetUp() * -_downForce / 25;
		GetTransform()->SetLocalRotation(Vec3(baseRot.x - (_recoilAmount / 70.f), baseRot.y, baseRot.z));
	}
	else if(_recoilAmount < 0.f)
	{
		_recoilAmount = 0;
		_curRecoil2 = 0;
	}

	GetTransform()->SetLocalPosition(basePos);
}