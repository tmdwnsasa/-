#include "pch.h"
#include "CameraScript.h"
#include "Engine.h"
#include "Transform.h"
#include "Camera.h"
#include "GameObject.h"
#include "Input.h"
#include "Timer.h"
#include "SceneManager.h"
#include <iostream>
//cout 출력용 코드
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")


CameraScript::CameraScript() : Component(COMPONENT_TYPE::CAMERASCRIPT)
{
	//::ScreenToClient(GEngine->GetWindow().hwnd, &_oldMousePos);
	_oldMousePos = { GEngine->GetWindow().width / 2, GEngine->GetWindow().height / 2 };
}

CameraScript::~CameraScript()
{
}

void CameraScript::Update()
{
	if (_mouseLock == false )
		CameraRotation();
}

void CameraScript::LateUpdate()
{
	Vec3 pos = GetTransform()->GetLocalPosition();


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
		if (_mouseLock == true && _shopOpened == false)
		{
			POINT tempForSetCursor = _oldMousePos;
			ClientToScreen(GEngine->GetWindow().hwnd, &tempForSetCursor);
			SetCursorPos(tempForSetCursor.x, tempForSetCursor.y);
			_mouseLock = false;
		}
	}

	if (INPUT->GetButtonDown(KEY_TYPE::RBUTTON))
	{
		const POINT& pos = INPUT->GetMousePos();
		GET_SINGLE(SceneManager)->Pick(pos.x, pos.y);
	}

	if (INPUT->GetButtonDown(KEY_TYPE::ESC))
	{
		_mouseLock = true;
	}

	if (INPUT->GetButtonDown(KEY_TYPE::O))
	{
		POINT tempForSetCursor = _oldMousePos;
		ClientToScreen(GEngine->GetWindow().hwnd, &tempForSetCursor);
		SetCursorPos(tempForSetCursor.x, tempForSetCursor.y);
		if (_mouseLock == true)
			_mouseLock = false;
		else
			_mouseLock = true;
	}


	GetTransform()->SetLocalPosition(pos);
}

void CameraScript::CameraRotation()
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
	if (rotation.x > 1.55f)
	{
		rotation.x = 1.55f;
		cydelta = 1.55f;
	}
	if (rotation.x < -1.55f)
	{
		rotation.x = -1.55f;
		cydelta = -1.55f;
	}
	GetTransform()->SetLocalRotation(rotation);
}