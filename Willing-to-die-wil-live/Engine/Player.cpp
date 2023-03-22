#include "pch.h"
#include "Engine.h"
#include "Player.h"
#include "Input.h"
#include "Animator.h"
#include "Transform.h"
#include "Timer.h"

Player::Player()
{
	_oldMousePos = { GEngine->GetWindow().width / 2, GEngine->GetWindow().height / 2 };
}

Player::~Player()
{

}

void Player::Update()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	if (INPUT->GetButton(KEY_TYPE::W))
		pos -= GetTransform()->GetLook() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::S))
		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::A))
		pos -= GetTransform()->GetRight() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::D))
		pos += GetTransform()->GetRight() * _speed * DELTA_TIME;

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_1))
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = (currentIndex + 1) % count;
		GetAnimator()->Play(index);
	}

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_2))
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = (currentIndex - 1 + count) % count;
		GetAnimator()->Play(index);
	}
	PlayerRotate();
	GetTransform()->SetLocalPosition(pos);
}

void Player::PlayerRotate()
{
	Vec3 rotation = GetTransform()->GetLocalRotation();
	_mousePos = INPUT->GetMousePos();
	cxdelta += (_mousePos.x - _oldMousePos.x) / 300.f;
	rotation.y = cxdelta;
	GetTransform()->SetLocalRotation(rotation);
}