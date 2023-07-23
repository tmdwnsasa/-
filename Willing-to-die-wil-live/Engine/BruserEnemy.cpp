#include "pch.h"
#include "BruserEnemy.h"
#include "Engine.h"
#include "Input.h"
#include "Animator.h"
#include "Timer.h"
#include "Transform.h"
#include "Astar.h"
#include "TileNode.h"
#include "TileMap.h"
#include "PathFinder.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Component.h"
#include <iostream>

//cout 출력용 코드
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")


BruserEnemy::BruserEnemy() : Component(COMPONENT_TYPE::BRUSERENEMY)
{

}

BruserEnemy::~BruserEnemy()
{

}

void BruserEnemy::Update()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	//pos += GetTransform()->GetLook() * _speed * 10 * DELTA_TIME;
	//pos += GetTransform()->GetRight() * _speed * DELTA_TIME;


	//GetTransform()->SetLocalPosition(pos);

	SetEnemyPosition(pos);

	SetPlayerPos();
	if (!ResponeCheck)
	{
		Respone();
	}
	Time += DELTA_TIME;
	if (Time > 0.1)
	{
		if (_hp > 0)
		{
			AstarCall();
		}
		Time = 0;
	}
	if (_hp > 0)
	{
		AstarMove(firstx, firsty, secondx, secondy);
	}

	//Animation();

	if (_hp <= 0)
	{
		AnimationCount();
	}

}


void BruserEnemy::AstarCall()
{
	TileMap tileMap;
	PathFinder pathFinder(&tileMap);

	Vector2 startPos;
	Vector2 endPos;
	tileMap.CreateTile(startPos, endPos, 3);

	//tileMap.Display(std::list<TileNode*>());
	//printf("\n\n");
	auto nodeList = pathFinder.DoFindPath(startPos, endPos);
	//tileMap.Display(nodeList);


	Vec3 EPos = GetEnemyPosition();

	//std::cout <<EPos.x <<"Z좌표 :    "<< EPos.z << std::endl;

	if (nodeList.size() != 0)
	{
		firstx = nodeList.front()->pos.x;
		firsty = nodeList.front()->pos.y;

		list<TileNode*>::iterator iter = nodeList.begin();

		advance(iter, 1);
		secondx = (*iter)->pos.x;
		secondy = (*iter)->pos.y;
	}
}

void BruserEnemy::AstarMove(int x, int y, int z, int w)
{
	if (DELTA_TIME > 1)
		return;
	Vec3 pos = GetTransform()->GetLocalPosition();

	x = x - z;
	y = y - w;
	if (x == -1 && y == 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, py * 0.5, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 오른쪽
		//float a = DELTA_TIME;
		//pos += GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 오른쪽
	}

	if (x == -1 && y == 1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, py * 0.25, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 오른쪽
		//float a = DELTA_TIME;
		//pos += GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 오른쪽
	}
	if (x == -1 && y == -1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, py * 0.75, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 오른쪽
		//float a = DELTA_TIME;
		//pos += GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 오른쪽
	}


	if (x == 1 && y == 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py * 0.5, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 왼쪽
		//pos -= GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 왼쪽
	}


	if (x == 1 && y == 1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py * 0.25, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 왼쪽
		//pos -= GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 왼쪽
	}


	if (x == 1 && y == -1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py * 0.75, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 왼쪽
		//pos -= GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 왼쪽
	}
	if (x == 0 && y == -1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;
	}

	if (x == 0 && y == 1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, 0, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;
	}
	double rotpos = GetTransform()->GetLocalRotation().y;
	GetTransform()->SetLocalPosition(pos);

}



void BruserEnemy::SetPlayerPos()
{
	for (int i = 0; i < 100; i++)
		for (int j = 0; j < 100; j++)
		{
			if (BrtileMap[i][j] == 2)
			{
				BrtileMap[i][j] = 0;
				/*EnemyCount++;
				if (EnemyCount == 10)
				{
					tileMap[i][j] = 0;
					EnemyCount = 0;
				}*/
			}
			else if (BrtileMap[i][j] == 3)
			{
				BrtileMap[i][j] = 0;
			}
		}
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	double k = PlayerPos.x;
	double l = -PlayerPos.z;

	k = (k / 300 * 4) - 1;
	l = (l / 300 * 4) + 1;

	int x = (int)k;
	int y = (int)l;
	BrtileMap[y][x] = 3;

	Vec3 EPos = GetEnemyPosition();

	if (EPos.z > 0)
		EPos.z = 0;

	double p = EPos.x;
	double q = -(EPos.z);


	p = (p / 300 * 4) - 1;
	q = (q / 300 * 4) + 1;


	x = (int)p;
	y = (int)q;
	BrtileMap[y][x] = 2;

	int w = x + y;
}

void BruserEnemy::Animation()
{
	if (AnimeCount == 0)
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = (currentIndex + 2) % count;
		GetAnimator()->Play(index);
		AnimeCount++;
	}
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

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_5))
	{
		GetAnimator()->Stop();
	}
}

void BruserEnemy::AnimationCount()
{
	if (AnimeCount == 1)
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = (currentIndex + 1) % count;
		GetAnimator()->Play(index);
		std::cout << index << std::endl;
		AnimeCount++;
	}

	else if (AnimeCount == 2)
	{
		DieTime += DELTA_TIME;
		if (DieTime > 2.2)
		{
			GetAnimator()->Stop();
		}
	}
}



int(*BruserEnemy::CreateMap())[BrHeight]
{
	return BrtileMap;
}

void BruserEnemy::LostHp()
{
	_hp -= 30;
}


void BruserEnemy::Respone()
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	if (PlayerPos.x > 3500)
		if (PlayerPos.z < -3500)
			CheckPoint = 1;
		else if (PlayerPos.z >= -3500)
			CheckPoint = 2;
	if (PlayerPos.x <= 3500)
		if (PlayerPos.z < -3500)
			CheckPoint = 3;
		else if (PlayerPos.z >= -3500)
			CheckPoint = 4;

	switch (CheckPoint)
	{
	case 1:
		GetTransform()->SetLocalPosition(ResponeArea1);
		break;
	case 2:
		GetTransform()->SetLocalPosition(ResponeArea2);
		break;
	case 3:
		GetTransform()->SetLocalPosition(ResponeArea3);
		break;
	case 4:
		GetTransform()->SetLocalPosition(ResponeArea4);
		break;
	default:
		break;
	}

	ResponeCheck = true;
}

