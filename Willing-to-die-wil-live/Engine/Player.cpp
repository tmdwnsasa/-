#include "pch.h"
#include "Player.h"
#include "Input.h"
#include "Animator.h"
#include "Transform.h"
#include "Timer.h"

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

	GetTransform()->SetLocalPosition(pos);
}