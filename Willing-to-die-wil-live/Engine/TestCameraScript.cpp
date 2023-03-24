#include "pch.h"
#include "TestCameraScript.h"
#include "Engine.h"
#include "Transform.h"
#include "Camera.h"
#include "GameObject.h"
#include "Input.h"
#include "Timer.h"
#include "SceneManager.h"

TestCameraScript::TestCameraScript()
{
	_oldMousePos = { GEngine->GetWindow().width/2, GEngine->GetWindow().height/2};
	//::ScreenToClient(GEngine->GetWindow().hwnd, &_oldMousePos);
}

TestCameraScript::~TestCameraScript()
{
}

void TestCameraScript::Update()
{
	if (_isOn == true)
		CameraRotation();
}

void TestCameraScript::LateUpdate()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	if (INPUT->GetButton(KEY_TYPE::W))
		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::S))
		pos -= GetTransform()->GetLook() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::A))
		pos -= GetTransform()->GetRight() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::D))
		pos += GetTransform()->GetRight() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::Q))
	{
		Vec3 rotation = GetTransform()->GetLocalRotation();
		rotation.x += DELTA_TIME * 0.5f;
		GetTransform()->SetLocalRotation(rotation);
	}

	if (INPUT->GetButton(KEY_TYPE::E))
	{
		Vec3 rotation = GetTransform()->GetLocalRotation();
		rotation.x -= DELTA_TIME * 0.5f;
		GetTransform()->SetLocalRotation(rotation);
	}

	if (INPUT->GetButton(KEY_TYPE::Z))
	{
		Vec3 rotation = GetTransform()->GetLocalRotation();
		rotation.y += DELTA_TIME * 0.5f;
		GetTransform()->SetLocalRotation(rotation);
	}

	if (INPUT->GetButton(KEY_TYPE::C))
	{
		Vec3 rotation = GetTransform()->GetLocalRotation();
		rotation.y -= DELTA_TIME * 0.5f;
		GetTransform()->SetLocalRotation(rotation);
	}

	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
	{
		if (_isOn == false)
		{
			POINT tempForSetCursor = _oldMousePos;
			ClientToScreen(GEngine->GetWindow().hwnd, &tempForSetCursor);
			SetCursorPos(tempForSetCursor.x, tempForSetCursor.y);
			_isOn = true;
		}
	}

	if (INPUT->GetButtonDown(KEY_TYPE::RBUTTON))
	{
		const POINT& pos = INPUT->GetMousePos();
		GET_SINGLE(SceneManager)->Pick(pos.x, pos.y);
	}

	if (INPUT->GetButtonDown(KEY_TYPE::TAB))
	{
		_isOn = false;
	}


	GetTransform()->SetLocalPosition(pos);
}

void TestCameraScript::CameraRotation()
{
	Vec3 rotation = GetTransform()->GetLocalRotation();
	POINT tempForSetCursor = _oldMousePos;
	_mousePos = INPUT->GetMousePos();
	cxdelta += (_mousePos.x - _oldMousePos.x) / 300.f;
	cydelta += (_mousePos.y - _oldMousePos.y) / 300.f;
	ClientToScreen(GEngine->GetWindow().hwnd, &tempForSetCursor);
	SetCursorPos(tempForSetCursor.x, tempForSetCursor.y);
	rotation.x = cydelta;
	rotation.y = cxdelta;
	GetTransform()->SetLocalRotation(rotation);
}