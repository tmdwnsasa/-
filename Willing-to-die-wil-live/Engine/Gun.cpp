#include "pch.h"
#include "Transform.h"
#include "Gun.h"

Gun::Gun() :Component(COMPONENT_TYPE::GUN)
{

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
		GetTransform()->SetLocalScale(Vec3(30.f, 30.f, 30.f));
		GetTransform()->LookAt(Vec3(Look));
	}
}